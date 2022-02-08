from getpass import getpass
import json
import logging
import os
from typing import List, Optional

logger = logging


class DBGConfig:
    hash_passwd: bool = False

    class SMS:
        close: bool = True
        fixed: Optional[str] = '1234'
        close_times_limit = True

    class MySQL:
        drop_before_run: bool = False
        close_foreign_key: bool = False

    on = True


class Environment:
    class Name:
        sms_aid: str = 'SMSConfig/accessKeyId'
        sms_as: str = 'SMSConfig/accessSecret'
        sms_rid = 'SMSConfig/RegionId'
        sms_sn = 'SMSConfig/SignName'
        sms_tc = 'SMSConfig/TemplateCode'
        worker_list: str = 'worker_list'

    root_dir: str = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    family: List[int]

    @classmethod
    def rank(cls):
        return cls.family.index(os.getpid())


class GunicornConfig:
    bind: str = '0.0.0.0:5000'
    backlog: int = 512
    timeout: int = 30
    worker_class: str = 'gevent'
    workers: int = 4  # multiprocessing.cpu_count() * 2 + 1
    threads: int = 2
    loglevel: str = 'info'
    access_log_format: str = '%(t)s %(p)s %(h)s "%(r)s" %(s)s %(L)s %(b)s %(f)s" "%(a)s"'


class LogConfig:
    format_patterns: List[str] = [
        '%(message)s',
        '%(asctime)s ptid=%(process)d-%(thread)s %(levelname)s at(%(pathname)s, %(lineno)d): %(message)s']
    level: int = 0
    pattern_n: int = 0


class MySQLConfig:
    host: str = 'mysql'
    port: int = 3306
    user: str = 'root'
    db: str = 'yilao'
    passwd: Optional[str] = 'root'
    scripts: List[str] = ['', '1.0']
    init_database: bool = True
    retry_times: int = 10
    retry_interval: float = 1


class ServerConfig:
    host: str = '0.0.0.0'
    port: int = 8001


class SMSConfig:
    accessKeyId: Optional[str]
    accessSecret: Optional[str]
    RegionId: Optional[str]
    SignName: Optional[str]
    TemplateCode: Optional[str]


class RedisConfig:
    host = 'redis'
    port = 6379
    db = 0
    passwd = None
    retry_times: int = 10
    retry_interval: float = 1


class ResourceConfig:
    class Resource:
        chuck_size: int = 4096

    class User:
        active_duration: float = 300

    class Token:
        expire = 3000000
        cache = True

    class SMS:
        expire = 300
        times_limit = 5
        appid_list = [
            'df3b72a07a0a4fa1854a48b543690eab'
        ]


ppid = os.getppid()


def yl(s, diff_launch=False):
    return f'{Environment.root_dir}/{ppid}/{s}' if diff_launch else f'{Environment.root_dir}/{s}'


def get_secret():
    try:
        with open(f'{os.path.dirname(os.path.abspath(__file__))}{os.sep}secret.json') as f:
            secret = json.load(f)
        # 重要密码
        if Environment.Name.sms_as not in secret:
            secret[Environment.Name.sms_as] = getpass('请输入阿里云RAM用户的accessSecret')
        return secret
    except FileNotFoundError:
        logger.info('required file "secret.json" template:')
        print(json.dumps({
            Environment.Name.sms_aid: "Optional[str]",
            Environment.Name.sms_as: "Optional[str]",
            Environment.Name.sms_rid: "Optional[str]",
            Environment.Name.sms_sn: "Optional[str]",
            Environment.Name.sms_tc: "Optional[str]"
        }, indent=4))
        logger.info("what's more, see the link: https://dysms.console.aliyun.com/dysms.htm#/quickStart")
        exit(0)
    except KeyboardInterrupt:
        print()
        exit(0)
