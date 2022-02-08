from datetime import datetime, timedelta
from typing import List

from flask import request, Response
from flask_restful import Resource
from schema import dialog_schema
from ..util import curd_params, mycursor, get_now
from .validation import token_validate
from marshmallow import Schema, fields


class DialogUserSchema(Schema):
    from_user = fields.Str()
    to_user = fields.Str()
    id_name = fields.Str()
    id_photo = fields.Str()
    last_send_at = fields.DateTime()
    last_content = fields.Str()
    type = fields.Str()


dialog_user_schema = DialogUserSchema()


class DialogUsersResource(Resource):
    @token_validate
    def get(self, mobile):
        with mycursor(dictionary=True) as c:
            sql = """select from_user, to_user, id_name, id_photo, send_at last_send_at, content last_content, type
from dialog
         left join user u on u.mobile = dialog.to_user
where from_user = %s
  and id = (select d.id
            from dialog d
            where (d.from_user = %s and d.to_user = dialog.to_user)
               or (d.from_user = dialog.to_user and d.to_user = %s)
            order by send_at desc
            limit 1)
union
select from_user, to_user, id_name, id_photo, send_at last_send_at, content last_content, type
from dialog
         left join user u on u.mobile = dialog.from_user
where to_user = %s
  and id = (select d.id
            from dialog d
            where (d.from_user = dialog.from_user and d.to_user = %s)
               or (d.from_user = %s and d.to_user = dialog.from_user)
            order by send_at desc
            limit 1);"""
            c.execute(sql, (mobile, mobile, mobile, mobile, mobile, mobile))
            return dialog_user_schema.dump(c.fetchall(), many=True)


class DialogListResource(Resource):
    @staticmethod
    def filter_(dialogs: List[dict]):
        now = get_now()
        min_id = request.args.get('min_id', -1, int)
        try:
            begin = now + timedelta(seconds=int(request.args.get('begin')))
        except TypeError:
            begin = None
        try:
            end = now + timedelta(seconds=int(request.args.get('end')))
        except TypeError:
            end = None
        res = []
        for dialog in dialogs:
            create_at = dialog['send_at']
            if begin is not None and create_at < begin:
                continue
            if end is not None and create_at > end:
                continue
            if min_id != -1 and dialog['id'] < min_id:
                continue
            res.append(dialog)
        return res

    @token_validate
    def get(self, mobile, other):
        with mycursor(dictionary=True) as c:
            min_id = request.args.get('min_id', -1, int)
            c.execute('select * from dialog '
                      'where ((from_user=%s and (%s=0 or to_user=%s)) '
                      'or ((%s=0 or from_user=%s) and to_user=%s)) and (%s!=0 or receive=0)',
                      (mobile, other, other, other, other, mobile, min_id))  # min_id为0时only_receive
            result = self.filter_(c.fetchall())
            res = dialog_schema.dump(result, many=True)
            c.execute('update dialog set receive=1 '
                      'where id in (select id from yilao.dialog where ((%s=0 or from_user=%s) and to_user=%s))',
                      (other, other, mobile))
            return res


class DialogResource(Resource):
    @token_validate
    def post(self, mobile):
        with mycursor() as c:
            data = dialog_schema.loads(request.data.decode())
            data['from_user'] = mobile
            data['send_at'] = get_now().replace(microsecond=0)
            to_insert = curd_params(data, dialog_schema)
            c.execute(f'insert into dialog set {to_insert[0]}', to_insert[1])
            return dialog_schema.dump({'id': c.lastrowid, 'send_at': data['send_at']}), 201
