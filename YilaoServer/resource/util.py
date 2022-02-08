from config.abstractconfig import DBGConfig
from flask import request, Response
from flask_restful import Api
from functools import lru_cache
from hashlib import sha256
import logging
from marshmallow import Schema, fields
from marshmallow.exceptions import ValidationError
import mysql.connector
from mysql.connector.errorcode import *
import re
from shapely import wkb
from schema import PointSchema
from datetime import datetime, timedelta, timezone
import sys
import traceback
from werkzeug.exceptions import HTTPException
from wrap import connect_mysql, connect_redis, SmartCursor

logger = logging
get_mcon = connect_mysql
real_get_rcon = connect_redis


def mycursor(conn=None, autocommit=True, close_conn=False, buffered=None, raw=None, prepared=None,
             cursor_class=None, dictionary=None, named_tuple=None):
    if conn is None:
        conn = get_mcon()
    return SmartCursor(conn, autocommit, close_conn, buffered=buffered, raw=raw, prepared=prepared,
                       cursor_class=cursor_class, dictionary=dictionary, named_tuple=named_tuple)


def get_rcon():
    return real_get_rcon()


def get_logger():
    return logger


null_point = {
    'longitude': 0,
    'latitude': 90,
    'name': None
}

sql_null_point = "st_pointfromtext('point(0 90)', 4326)"


@lru_cache()
def get_point_keys(schema: Schema):
    spatial_keys = set()
    for k, v in schema.fields.items():
        if isinstance(v, fields.Nested) and v.nested == PointSchema:
            spatial_keys.add(k)
    return spatial_keys


def get_sql_params(di: dict, schema: Schema):
    keys = []
    placeholders = []
    values = []
    point_keys = get_point_keys(schema)
    for k, v in di.items():
        if k in point_keys:
            keys.extend([k, f'{k}_name'])
            placeholders.extend(["st_pointfromtext('point(%s %s)', 4326)", '%s'])
            values.extend([v['longitude'], v['latitude'], v['name']])
        else:
            keys.append(k)
            placeholders.append('%s')
            values.append(v)
    # 用反引号包裹变量名，防止特殊变量名造成错误
    keys = map(lambda k_: f'`{k_}`', keys)
    return keys, placeholders, values


def curd_params(di: dict, schema: Schema):
    keys, placeholders, values = get_sql_params(di, schema)
    return ', '.join(map(lambda z: f'{z[0]}={z[1]}', zip(keys, placeholders))), values


def to_readable_location(di: dict, name):
    # mysql空间类型的内部存储结构SRID(4)+WKB(21)，只需要后面21位
    if name in di and di[name] is not None:
        b = bytes(di[name][4:])
        point = wkb.loads(b)
        longitude = point.xy[0][0]
        latitude = point.xy[1][0]
        location_name = di.pop(f'{name}_name')
        di[name] = {
            'longitude': longitude,
            'latitude': latitude,
            'name': location_name
        }
    else:
        di[name] = None


def get_now():
    return datetime.utcnow() + timedelta(hours=8)


def dump_locations(di: dict, schema: Schema):
    point_keys = get_point_keys(schema)

    for k in point_keys:
        to_readable_location(di, k)


class BaseApi(Api):
    dup_entry = re.compile(r"Check constraint '(\w+)' is violated.")
    field_specified_twice = re.compile(r"Column '(\w+)' specified twice")
    no_referenced_row = re.compile(r'Cannot add or update a child row: '
                                   r'a foreign key constraint fails \(`[^`]+`\.`[^`]+`, '
                                   r'CONSTRAINT `[^`]+` FOREIGN KEY \((`[^`]+`)\) REFERENCES `[^`]+` \(`[^`]+`\)\)')

    def handle_error(self, e):
        if isinstance(e, HTTPException):
            return Response(status=e.code)
        elif isinstance(e, mysql.connector.Error):
            if e.errno == ER_DUP_ENTRY:
                return exc(e.msg), 409
            elif e.errno == ER_FIELD_SPECIFIED_TWICE:
                key = self.field_specified_twice.match(e.msg)[1]
                return exc(f'"{key}" specified twice'), 400
            elif e.errno == ER_NO_REFERENCED_ROW_2:
                key = self.no_referenced_row.match(e.msg)[1]
                return exc(f'{key}不存在', True), 400
            else:
                logger.debug(str(e))
                return Response(status=500)
        elif isinstance(e, ValidationError):
            return exc(e.args[0], True), 400
        else:
            logger.debug(traceback.format_exc())
            return Response(status=500)


def exc(s, echo=False, **kwargs):
    kwargs['exc'] = s
    kwargs['echo'] = f'{request.method} {request.url}\n{request.headers}{request.data.decode()}'
    return kwargs


def hash_passwd(s: str):
    if DBGConfig.on and not DBGConfig.hash_passwd:
        return s
    else:
        return sha256(s.encode()).hexdigest()
