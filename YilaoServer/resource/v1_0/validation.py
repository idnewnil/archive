from config.abstractconfig import DBGConfig, ResourceConfig, yl
from flask import Response
from flask_restful import Resource
from functools import wraps
import json
from sms import rand_code, send_code
from uuid import uuid4
from ..util import get_logger, get_rcon, hash_passwd, exc, mycursor
from flask import request


class SMSResource(Resource):
    @staticmethod
    def sms_name(appid, mobile, method, path):
        appid = str(appid).lower()
        mobile = str(mobile)
        method = str(method).upper()
        path = str(path).lower()
        return yl(f'sms?{appid=}&{mobile=}&{method=}&{path=}')

    def post(self):
        data = json.loads(request.data)
        appid = data.get('appid')
        if appid not in ResourceConfig.SMS.appid_list:
            return exc('Invalid appid', True), 400

        # 阿里云本身已有信息限制功能，可以考虑改成IP限制
        count_name = yl(f"sms/count?mobile={data['mobile']}")
        rcon = get_rcon()

        # 无法确保原子性
        count = rcon.incr(count_name)
        if rcon.ttl(count_name) == -1:
            rcon.expire(count_name, ResourceConfig.SMS.expire)

        if (not DBGConfig.on or not DBGConfig.SMS.close_times_limit) and count > ResourceConfig.SMS.times_limit:
            return exc('frequent request'), 400
        else:
            name = self.sms_name(**data)
            code = rand_code()
            rcon.set(name, code, ex=ResourceConfig.SMS.expire)
            send_code(data['mobile'], code)
            get_logger().debug(f'Send code:\n'
                               f'{name=}\n'
                               f'{code=}')
            return Response(status=201)

    @classmethod
    def validate(cls, func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            mobile = kwargs.get('mobile', request.args.get('mobile', ''))
            appid = request.args.get('appid', '')
            name = cls.sms_name(appid, mobile, request.method, request.path)
            code = request.args.get('code')
            rcon = get_rcon()
            real_code = rcon.get(name)
            if real_code and real_code == code:
                get_logger().debug('Succeed')
                return func(*args, **kwargs)
            else:
                get_logger().debug(f'Failed to pass SMS validate:\n'
                                   f'{mobile=}\n'
                                   f'{name=}\n'
                                   f'{code=}\n'
                                   f'{real_code=}')
                return Response(status=401)

        return wrapper


class PasswdResource(Resource):
    @staticmethod
    def validate(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            mobile = kwargs.get('mobile', request.args.get('mobile', ''))
            passwd = request.args.get('passwd')
            if passwd:
                with mycursor(autocommit=False) as c:
                    c.execute('select passwd from user where mobile=%s limit 1', (mobile,))
                    res = c.fetchone()
                    if res is None:
                        get_logger().debug(f'Could not pass password validate because user {mobile} is nonexistent\n')
                        return exc('nonexistent user', True), 404
                    elif res[0] == hash_passwd(passwd):
                        get_logger().debug('Pass password validate')
                        return func(*args, **kwargs)
                    else:
                        get_logger().debug('Could not pass password validate\n'
                                           f'{mobile=}\n'
                                           f'{passwd=}\n'
                                           f'real_passwd={res[0]}')
            return Response(status=401)

        return wrapper


def or_(*validate_func):
    if len(validate_func) == 0:
        raise ValueError('empty func tuple')

    def _(func):
        artifact = []
        for it in validate_func:
            artifact.append(it(func))

        def wrapper(*args, **kwargs):
            get_logger().debug('begin Or_ validate')
            for it_ in artifact:
                res = it_(*args, **kwargs)
                # 如果http状态码不是401，那么返回结果
                if not (isinstance(res, Response) and res.status_code == 401) and not (
                        isinstance(res, tuple) and res[1] == 401):
                    get_logger().debug('pass Or_ validate')
                    return res
            get_logger().debug('Could not pass Or_ validate')
            return Response(status=401)

        return wrapper

    return _


class TokenResource(Resource):
    @staticmethod
    def token_name(mobile, appid):
        appid = str(appid).lower()
        return yl(f'mobile/{mobile}/token?{appid=}')

    @or_(SMSResource.validate, PasswdResource.validate)
    def post(self, mobile):
        appid = request.args.get('appid')
        if appid is None:
            return exc('appid is null'), 400
        uuid = uuid4().hex
        with mycursor() as c:
            get_rcon().delete(self.token_name(mobile, appid))
            c.execute(
                'replace into token(user, appid, hex, deadline) '
                'values (%s, %s, %s, current_timestamp + interval %s second)',
                (mobile, appid, uuid, ResourceConfig.Token.expire))
        return {'token': uuid}, 201

    @classmethod
    def validate(cls, func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            mobile = kwargs.get('mobile', request.args.get('mobile', ''))
            appid = request.args.get('appid')
            name = cls.token_name(mobile, appid)
            token = request.args.get('token')
            rcon = get_rcon()
            res = rcon.get(name)
            if res is None:
                get_logger().debug('token not in redis, select from database')
                with mycursor(autocommit=False) as c:
                    c.execute(
                        'select hex from token where user=%s and appid=%s and current_timestamp < deadline limit 1',
                        (mobile, appid))
                    res = c.fetchone()
                    if res:
                        res = res[0]
                        rcon.set(name, res, ex=ResourceConfig.Token.expire, nx=True)
                    else:
                        get_logger().debug('token not in database')
                        get_logger().debug('could not pass token validation')
                        return Response(status=401)
            if res == token:
                return func(*args, **kwargs)
            else:
                return Response(status=401)

        return wrapper


sms_validate = SMSResource.validate
passwd_validate = PasswdResource.validate
token_validate = TokenResource.validate
any_validate = or_(sms_validate, passwd_validate, token_validate)


def test(validate_func, **kwargs):
    res = validate_func(lambda *args, **kwargs_: True)(**kwargs)
    return res is True
