from datetime import datetime, timedelta
from functools import lru_cache, partial
import inspect
import json
import logging
import re
import requests
import sys
from typing import Optional

_prefix: str
default_appid = 'df3b72a07a0a4fa1854a48b543690eab'
args_pattern = re.compile(rf'^[^()]+\((.*)\)$')
loglevel = 11
header_width = 50


def _use(_): pass


class HaveAll:
    def __getattr__(self, item):
        return self


# mysql-connector-python可能会出现ImportError，所以仅尝试插入
try:
    from resource.v1_0.commodity import CommodityListResource, CommodityResource
    from resource.v1_0.dialog import DialogResource
    from resource.v1_0.order import PublicOrderListResource, OrderListResource, OrderResource
    from resource.v1_0.user import UserResource
    from resource.v1_0.validation import PasswdResource, SMSResource, TokenResource
except ImportError:
    has_all = HaveAll()
    CommodityListResource = has_all
    CommodityResource = has_all
    DialogResource = has_all
    PublicOrderListResource = has_all
    OrderListResource = has_all
    OrderResource = has_all
    UserResource = has_all
    PasswdResource = has_all
    SMSResource = has_all
    TokenResource = has_all


# 链接到resource
def link(*args):
    _use(args)

    def wrapper(func):
        return func

    return wrapper


class Error(Exception):
    pass


class CheckError(Error):
    def __init__(self, url, code, content):
        super(CheckError, self).__init__(url, code, content)
        self.url = url
        self.code = code
        self.content = content


def check(resp):
    logmsg = (f"{' REQUEST '.center(header_width, '=')}\n"
              f'{resp.request.method} {resp.request.url}\n')
    if resp.request.body:
        try:
            logmsg += f'\n{json.dumps(json.loads(resp.request.body.decode()), indent=4)}\n'
        except Exception as e:
            logmsg += f'\nlen={len(resp.request.body)}\n'
    logmsg += f"{' RESPONSE '.center(header_width, '=')}\n"
    logmsg += f'{resp.status_code} {resp.reason}\n'
    if resp.content:
        try:
            logmsg += f'\n{json.dumps(json.loads(resp.content.decode()), indent=4)}\n'
        except Exception as e:
            logmsg += f'\nlen={len(resp.content)}\n'
    logmsg += f"{' END '.center(header_width, '=')}"
    logging.log(loglevel, logmsg)
    if resp.status_code // 100 != 2:
        raise CheckError(resp.url, resp.status_code, resp.content.decode())


class Null:
    pass


def get_args_name():
    s = inspect.stack()[2].code_context[0]
    res = args_pattern.match(s)
    return [it.strip() for it in res[1].split(',')]


def set_field(di, value, name=None):
    if value is not None:
        if name is None:
            name = get_args_name()[1]
        di[name] = None if value == Null else value


def point(longitude, latitude, name):
    return {
        'longitude': longitude,
        'latitude': latitude,
        'name': name
    }


class SMSTest:
    @staticmethod
    def url():
        return f'{_prefix}/v1.0/sms'

    @classmethod
    @link(SMSResource.post)
    def post(cls, mobile, method, base_url, appid=default_appid):
        path = base_url.replace(_prefix, '')
        url = cls.url()
        json_ = {}
        set_field(json_, appid)
        set_field(json_, mobile)
        set_field(json_, method)
        set_field(json_, path)
        resp = requests.post(url, json=json_)
        check(resp)


class TokenTest:
    @staticmethod
    def url(mobile):
        return f'{_prefix}/v1.0/users/{mobile}/tokens'

    @classmethod
    @link(TokenResource.post)
    def post(cls, mobile, passwd=None, code=None, appid=default_appid):
        url = cls.url(mobile)
        params = {}
        set_field(params, passwd)
        set_field(params, code)
        set_field(params, appid)
        resp = requests.post(url, params=params)
        check(resp)
        return resp.json()['token']


class UserTest:
    @staticmethod
    def url(mobile):
        return f'{_prefix}/v1.0/users/{mobile}'

    @classmethod
    @link(UserResource.get)
    def get(cls, mobile, token=None, appid: Optional[str] = default_appid):
        url = cls.url(mobile)
        params = {}
        # set_field(params, token)
        # set_field(params, appid)
        resp = requests.get(url, params)
        check(resp)
        return resp.json()

    @classmethod
    @link(UserResource.put)
    def put(cls, mobile, code, passwd, appid=default_appid, nickname=None, sex=None, portrait=None,
            default_location=None, id_name=None, id_school=None, id_photo=None):
        url = cls.url(mobile)
        params = {}
        set_field(params, code)
        set_field(params, appid)
        json_ = {}
        set_field(json_, passwd)
        set_field(json_, nickname)
        set_field(json_, sex)
        set_field(json_, portrait)
        set_field(json_, default_location)
        set_field(json_, id_name)
        set_field(json_, id_school)
        set_field(json_, id_photo)
        resp = requests.put(url, params=params, json=json_)
        check(resp)

    @classmethod
    @link(UserResource.patch)
    def patch(cls, mobile, token=None, passwd=None, code=None, appid=None, new_passwd=None, nickname=None, sex=None,
              portrait=None, default_location=None, id_name=None, id_school=None, id_photo=None):
        url = cls.url(mobile)
        params = {}
        set_field(params, token)
        set_field(params, passwd)
        set_field(params, code)
        set_field(params, appid)
        json_ = {}
        set_field(json_, new_passwd, 'passwd')
        set_field(json_, nickname)
        set_field(json_, sex)
        set_field(json_, portrait)
        set_field(json_, default_location)
        set_field(json_, id_name)
        set_field(json_, id_school)
        set_field(json_, id_photo)
        resp = requests.patch(url, params=params, json=json_)
        check(resp)


class PublicOrderListTest:
    @staticmethod
    def url():
        return f'{_prefix}/v1.0/public_orders'

    @classmethod
    @link(PublicOrderListResource.get)
    def get(cls, begin=None, end=None, type_=None):
        url = cls.url()
        params = {}
        set_field(params, begin)
        set_field(params, end)
        set_field(params, type_)
        resp = requests.get(url, params=params)
        check(resp)
        return resp.json()


class OrderListTest:
    @staticmethod
    def url(mobile):
        return f'{_prefix}/v1.0/users/{mobile}/orders'

    @classmethod
    @link(OrderListResource.get)
    def get(cls, mobile, token, begin=None, end=None, type_=None, distance=None, appid=default_appid):
        url = cls.url(mobile)
        params = {}
        set_field(params, begin)
        set_field(params, end)
        set_field(params, type_, 'type')
        set_field(params, distance)
        set_field(params, token)
        set_field(params, appid)
        resp = requests.get(url, params=params)
        check(resp)
        return resp.json()

    @classmethod
    @link(OrderListResource.post)
    def post(cls, mobile, token, phone, type_, count, reward, destination=None, emergency_level=None, category=None,
             detail=None, protected_info=None, photos=None, appid=default_appid):
        url = cls.url(mobile)
        params = {}
        set_field(params, token)
        set_field(params, appid)
        json_ = {}
        set_field(json_, phone)
        set_field(json_, destination)
        set_field(json_, emergency_level)
        set_field(json_, type_, 'type')
        set_field(json_, count)
        set_field(json_, reward)
        set_field(json_, category)
        set_field(json_, detail)
        set_field(json_, protected_info)
        set_field(json_, photos)
        resp = requests.post(url, params=params, json=json_)
        check(resp)
        return resp.json()['id']


class OrderTest:
    @staticmethod
    def url(mobile, order_id):
        return f'{_prefix}/v1.0/users/{mobile}/orders/{order_id}'

    @classmethod
    @link(OrderResource.patch)
    def patch(cls, mobile, token, order_id, receive=None, close=None, appid=default_appid):
        url = cls.url(mobile, order_id)
        params = {}
        set_field(params, token)
        set_field(params, appid)
        set_field(params, receive)
        set_field(params, close)
        resp = requests.patch(url, params=params)
        check(resp)
        try:
            return resp.json()
        except json.decoder.JSONDecodeError:
            pass


class DialogListTest:
    @staticmethod
    def url(mobile, other):
        return f'{_prefix}/v1.0/users/{mobile}/dialogs_with/{other}'

    @classmethod
    @link(DialogResource.get)
    def get(cls, mobile, token, other, appid=default_appid):
        url = cls.url(mobile, other)
        params = {}
        set_field(params, token)
        set_field(params, appid)
        resp = requests.get(url, params=params)
        check(resp)
        return resp.json()


class DialogTest:
    @staticmethod
    def url(mobile):
        return f'{_prefix}/v1.0/users/{mobile}/dialogs'

    @classmethod
    @link(DialogResource.post)
    def post(cls, mobile, token, to_user, content, appid=default_appid):
        url = cls.url(mobile)
        params = {}
        set_field(params, token)
        set_field(params, appid)
        json_ = {}
        set_field(json_, content)
        set_field(json_, to_user)
        resp = requests.post(url, params=params, json=json_)
        check(resp)


class ResourceListTest:
    @staticmethod
    def url(mobile):
        return f'{_prefix}/v1.0/users/{mobile}/resources'

    @classmethod
    def post(cls, mobile, token, filename, appid=default_appid):
        url = cls.url(mobile)
        params = {}
        set_field(params, token)
        set_field(params, appid)
        files = {
            'file': (filename, open(filename, 'rb'))
        }
        resp = requests.post(url, params=params, files=files)
        check(resp)
        return resp.json()['uuid']


class ResourceTest:
    @staticmethod
    def url(mobile, uuid):
        return f'{_prefix}/v1.0/users/{mobile}/resources/{uuid}'

    @classmethod
    def get(cls, mobile, token, uuid, appid=default_appid):
        url = cls.url(mobile, uuid)
        params = {}
        set_field(params, token)
        set_field(params, appid)
        resp = requests.get(url, params=params)
        check(resp)
        return resp.content


class CommodityListTest:
    @staticmethod
    def url(mobile):
        return f'{_prefix}/v1.0/users/{mobile}/commodities'

    @classmethod
    @link(CommodityListResource.post)
    def post(cls, mobile, token, name, price, detail=None, location=None, on_offer=None, appid=default_appid):
        url = cls.url(mobile)
        params = {}
        set_field(params, token)
        set_field(params, appid)
        _json = {}
        set_field(_json, name)
        set_field(_json, price)
        set_field(_json, detail)
        set_field(_json, location)
        set_field(_json, on_offer)
        resp = requests.post(url, params=params, json=_json)
        check(resp)
        return resp.json()['id']


class CommodityTest:
    @staticmethod
    def url(commodity_id):
        return f'{_prefix}/v1.0/commodities/{commodity_id}'

    @classmethod
    @link(CommodityResource.get)
    def get(cls, commodity_id):
        url = cls.url(commodity_id)
        resp = requests.get(url)
        check(resp)
        return resp.json()


def signup(mobile, get_code, get_passwd, nickname=None, sex=None, portrait=None, default_location=None, id_name=None,
           id_school=None, id_photo=None):
    try:
        UserTest.get(mobile, appid=None)
        raise Error(f'{mobile} has been signup')
    except CheckError as e:
        if e.code == 404:
            SMSTest.post(mobile, 'PUT', UserTest.url(mobile))
            UserTest.put(mobile, get_code(), get_passwd(), default_appid, nickname, sex, portrait, default_location,
                         id_name, id_school, id_photo)
        else:
            raise


def login_by_passwd(mobile, passwd):
    return TokenTest.post(mobile, passwd)


def login_by_code(mobile, get_code):
    SMSTest.post(mobile, 'POST', TokenTest.url(mobile))
    return TokenTest.post(mobile, code=get_code(), appid=default_appid)


def reset_passwd_by_old(mobile, get_old_passwd, get_new_passwd):
    UserTest.patch(mobile, passwd=get_old_passwd(), new_passwd=get_new_passwd())


def reset_passwd_by_code(mobile, get_code, get_new_passwd):
    SMSTest.post(mobile, 'PATCH', UserTest.url(mobile))
    UserTest.patch(mobile, code=get_code(), appid=default_appid, new_passwd=get_new_passwd())


def get_code_template():
    return input(r'input your code(^\d{4}$): ')


@lru_cache()
def get_passwd_template(i: int):
    s = {1: '1st', 2: '2nd', 3: '3rd'}.get(i, f'{i}th')
    return input(rf"input your {s} passwd(^\w{{8, 64}}$): ")


class Color:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


color = Color


def header(msg, i=0):
    header_color = [color.HEADER, color.OKGREEN, color.OKBLUE]
    header_str = f' {header_color[i]}{msg}{color.ENDC} '
    logging.log(loglevel, header_str.center(header_width + 9, '*'))


def test_template(mobile=13927553153, prefix='http://api.yilao.tk:15000', get_code=get_code_template,
                  get_passwd=get_passwd_template, proxies=None):
    global _prefix
    _prefix = prefix
    if proxies is not None:
        requests.api.request = partial(requests.api.request, proxies=proxies)
    logging.basicConfig(format='%(message)s', level=loglevel, stream=sys.stdout)

    # while True:
    #     ResourceTest.get(13060887368, None, '14ec3d68-8f52-48c7-ab81-e8013e4aa6f3')

    # 新建资源
    photo_uuid = ResourceListTest.post(0, None, __file__)

    try:
        header('Usertest')
        header('Test Signup', 1)
        try:
            signup(mobile, get_code, partial(get_passwd, 1), 'hello', 'male', None, None, 'me', 'school', photo_uuid)
        except Error:
            pass

        # user login
        header('Test Login', 1)
        header('By passwd', 2)
        login_by_passwd(mobile, get_passwd(1))
        header('By code', 2)
        login_by_code(mobile, get_code)

        # user secret patch
        header('Test Patch passwd', 1)
        header('By old', 2)
        reset_passwd_by_old(mobile, partial(get_passwd, 1), partial(get_passwd, 2))
        login_by_passwd(mobile, get_passwd(2))
        header('By code', 2)
        reset_passwd_by_code(mobile, get_code, partial(get_passwd, 1))
        login_by_passwd(mobile, get_passwd(1))

        # user common patch
        header('Test patch common', 1)
        header('Patch name', 2)
        token = login_by_passwd(mobile, get_passwd(1))
        # UserTest.patch(mobile, token, appid=default_appid, default_location=point(12, 34, 'abc'))

        # get user info
        header('Get Info', 1)
        UserTest.get(mobile)

        header('OrderTest')
        header('Post order', 1)
        order_id = OrderListTest.post(mobile, token, 15466666, 'abc', 1, 2, point(1, 2, 'aaa'))
        header('Get relative order', 1)
        OrderListTest.get(mobile, token, type_='abc')
        header('Get public order', 1)

        PublicOrderListTest.get(begin=-100, end=100)
        header('Relative order of another one')
        another_one_mobile = 16698066603
        try:
            signup(another_one_mobile, get_code, partial(get_passwd, 1))
        except Error:
            pass
        another_token = login_by_passwd(another_one_mobile, get_passwd(1))
        OrderListTest.get(another_one_mobile, another_token)
        header('Receive order', 1)
        OrderTest.patch(another_one_mobile, another_token, order_id, receive=True)
        header('Cancel receive order', 1)
        OrderTest.patch(another_one_mobile, another_token, order_id, receive=False)
        header('Receive order', 1)
        OrderTest.patch(another_one_mobile, another_token, order_id, receive=True)
        header('Closing order', 1)
        OrderTest.patch(mobile, token, order_id, close='cancel')
        header('Accept close order', 1)
        OrderTest.patch(another_one_mobile, another_token, order_id, close='close')

        header('DialogTest')
        header('Send', 1)
        DialogTest.post(mobile, token, another_one_mobile, 'hello!')
        header('Get all', 1)
        DialogListTest.get(mobile, token, another_one_mobile)
        header('Get empty', 1)
        DialogListTest.get(mobile, token, 0)
        header('Another get all', 1)
        DialogListTest.get(another_one_mobile, another_token, mobile)
        header('Another send', 1)
        DialogTest.post(another_one_mobile, another_token, mobile, 'hi!')
        header('Get all', 1)
        DialogListTest.get(mobile, token, another_one_mobile)

        header('ResourceTest')
        header('Post', 1)
        uuid = ResourceListTest.post(mobile, token, __file__)
        header('Get', 1)
        ResourceTest.get(mobile, token, uuid)

        header('CommodityTest')
        header('Post', 1)
        commodity_id = CommodityListTest.post(mobile, token, 'abc1234', 123)
        header('Get', 1)
        CommodityTest.get(commodity_id)
    except CheckError:
        logging.log(loglevel, 'Failed to pass the test')
