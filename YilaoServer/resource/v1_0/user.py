from flask_restful import Resource
from schema import UserSchema, user_schema
from .resource import get_path
from wrap import _use
from ..util import *
from .validation import or_, passwd_validate, sms_validate, test, token_validate
import shutil
import os


def get_user(mobile, *args):
    with mycursor(dictionary=True, autocommit=False) as c:
        c.execute(f"select {', '.join(args)} from user where mobile=%s", (mobile,))
        return c.fetchall()


class UserResource(Resource):
    def get(self, mobile):
        _use(self)
        with mycursor(dictionary=True) as c:
            c.execute('select * from user where mobile=%s limit 1', (mobile,))
            res = c.fetchone()
            if res is None:
                return Response(status=404)
            # if not test(token_validate, mobile=mobile):
            #     return Response(status=401)
            dump_locations(res, user_schema)
            return user_schema.dump(res)

    @sms_validate
    def put(self, mobile):
        data = user_schema.loads(request.data.decode())
        data['mobile'] = int(mobile)
        if 'passwd' not in data:
            return exc('need passwd field.')
        data['passwd'] = hash_passwd(data['passwd'])
        if 'id_photo' in data:
            src = f"{get_path(0)}/{data['id_photo']}"
            dst_path = get_path(mobile)
            if not os.path.exists(dst_path):
                os.makedirs(dst_path)
            dst = f"{dst_path}/{data['id_photo']}"
            shutil.move(src, dst)

        with mycursor() as c:
            to_insert = curd_params(data, user_schema)
            c.execute(f'insert into user set {to_insert[0]}', to_insert[1])
            return Response(status=201)

    def patch(self, mobile, partial_user_schema=UserSchema(exclude=('mobile',), partial=True)):
        _use(self)
        new_data = partial_user_schema.loads(request.data.decode())
        secret_field = {'passwd'}
        field = set(new_data.keys())
        intersection = secret_field & field
        if len(intersection) > 0:
            # 含有敏感字段，需要使用密码验证或者验证码验证才能通过
            if not test(or_(passwd_validate, sms_validate), mobile=mobile):
                return Response(status=401)
        else:
            if not test(token_validate, mobile=mobile):
                return Response(status=401)

        if 'passwd' in new_data:
            new_data['passwd'] = hash_passwd(new_data.pop('passwd'))

        with mycursor() as c:
            to_update = curd_params(new_data, partial_user_schema)
            c.execute(f"update user set {to_update[0]} where mobile=%s", (*to_update[1], mobile))
            return Response(status=204)
