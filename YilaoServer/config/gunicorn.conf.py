from config.abstractconfig import Environment, get_secret, GunicornConfig
import os
from wrap import connect_redis

Name = Environment.Name
prefix = f'{Environment.root_dir}/{os.getpid()}'
with connect_redis() as conn:
    secret = get_secret()
    # 设置信息
    for name in [Name.sms_aid, Name.sms_as, Name.sms_rid, Name.sms_sn, Name.sms_tc]:
        conn.set(f'{prefix}/{name}', secret[name])

bind = GunicornConfig.bind
workers = GunicornConfig.workers
loglevel = GunicornConfig.loglevel
access_log_format = GunicornConfig.access_log_format
