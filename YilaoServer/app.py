from config.yilaoconfig import *
from flask import request, Response
import logging
import json
from mysqlscript import iter_table, UserVersion
from resource import *
import resource.util
import sys
from wrap import connect_mysql, connect_redis, SmartCursor, Sync
import faulthandler

faulthandler.enable()

logging.basicConfig(level=LogConfig.level, format=LogConfig.format_patterns[LogConfig.pattern_n], stream=sys.stdout)
app = Flask(__name__)
logger = app.logger


def init_database():
    logger.info('Begin to build database')
    with SmartCursor(connect_mysql(db=None, logger=logger)) as c:
        if Environment.rank() == 0:
            if DBGConfig.on and DBGConfig.MySQL.drop_before_run:
                logger.info('drop database')
                c.execute(f'drop database if exists {MySQLConfig.db}')
            logger.info('create database')
            c.execute(f'create database if not exists {MySQLConfig.db}')
            c.execute(f'use {MySQLConfig.db}')
            for it in iter_table:
                if UserVersion.get(c) == it.require_version:
                    for _ in c.execute(it.script, multi=True):
                        pass
                    UserVersion.update_to(it.version, c)
    logger.info('Build database successfully')


def sync():
    return Sync(rcon)


rcon = connect_redis(logger=logger)
impl(rcon, __name__ != '__main__')
if MySQLConfig.init_database:
    with sync():
        init_database()
mcon = connect_mysql(logger=logger)
resource.util.get_mcon = lambda: mcon
resource.util.real_get_rcon = lambda: rcon
resource.util.logger = logger
register_api(app)


@app.after_request
def output_body(response: Response):
    width = 50
    logger.debug(' Request Body '.center(width, '='))
    if len(request.data) != 0:
        try:
            logger.debug(json.dumps(json.loads(request.data.decode()), indent=4))
        except json.JSONDecodeError:
            logger.debug('bytes object')
    logger.debug(' Response Body '.center(width, '='))
    try:
        if len(response.data) != 0:
            try:
                logger.debug(json.dumps(json.loads(response.data.decode()), indent=4))
            except json.JSONDecodeError:
                logger.debug('bytes object')
    except RuntimeError:
        # send_from_directory
        logger.debug('file')
    logger.debug(' END '.center(width, '='))
    return response


if __name__ == '__main__':
    app.run(ServerConfig.host, ServerConfig.port, True, use_reloader=False, threaded=False)
