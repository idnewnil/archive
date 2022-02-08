from .abstractconfig import *
from wrap import Sync


def impl(rcon, gunicorn):
    name = Environment.Name

    sms_aid = yl(name.sms_aid, True)
    sms_as = yl(name.sms_as, True)
    sms_rid = yl(name.sms_rid, True)
    sms_sn = yl(name.sms_sn, True)
    sms_tc = yl(name.sms_tc, True)

    if gunicorn:
        with Sync(rcon) as s:
            # 获取重要信息
            SMSConfig.accessKeyId = rcon.get(sms_aid)
            SMSConfig.accessSecret = rcon.get(sms_as)
            SMSConfig.RegionId = rcon.get(sms_rid)
            SMSConfig.SignName = rcon.get(sms_sn)
            SMSConfig.TemplateCode = rcon.get(sms_tc)
        Environment.family = s.li
        if Environment.rank() == 0:
            rcon.delete(sms_aid)
            rcon.delete(sms_as)
            rcon.delete(sms_rid)
            rcon.delete(sms_sn)
            rcon.delete(sms_tc)
    else:
        secret = get_secret()
        SMSConfig.accessKeyId = secret[name.sms_aid]
        SMSConfig.accessSecret = secret[name.sms_as]
        SMSConfig.RegionId = secret[name.sms_rid]
        SMSConfig.SignName = secret[name.sms_sn]
        SMSConfig.TemplateCode = secret[name.sms_tc]
        GunicornConfig.workers = 1
        Environment.family = [os.getpid()]
