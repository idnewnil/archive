from aliyunsdkcore.client import AcsClient
from aliyunsdkcore.request import CommonRequest
from config.yilaoconfig import DBGConfig, SMSConfig
from random import randint


def rand_code():
    if DBGConfig.on and DBGConfig.SMS.fixed is not None:
        return DBGConfig.SMS.fixed
    else:
        return f'{randint(0, 9999):04}'


def send_code(mobile: int, code: str):
    """发送验证码"""
    if DBGConfig.on and DBGConfig.SMS.close:
        return

    # 阿里云验证码code的相关规定：code必须为数字或字母，且长度不大于6
    if len(code) > 6 or not code.isalnum():
        pass

    client = AcsClient(SMSConfig.accessKeyId, SMSConfig.accessSecret, SMSConfig.RegionId)

    request = CommonRequest()
    request.set_accept_format('json')
    request.set_domain('dysmsapi.aliyuncs.com')
    request.set_method('POST')
    request.set_protocol_type('https')  # https | http
    request.set_version('2017-05-25')
    request.set_action_name('SendSms')

    request.add_query_param('RegionId', SMSConfig.RegionId)
    request.add_query_param('PhoneNumbers', str(mobile))
    request.add_query_param('SignName', SMSConfig.SignName)
    request.add_query_param('TemplateCode', SMSConfig.TemplateCode)
    request.add_query_param('TemplateParam', "{\"code\":\"" + code + "\"}")

    return client.do_action_with_exception(request)
