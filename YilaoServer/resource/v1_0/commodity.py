from flask import request
from flask_restful import Resource
from schema import commodity_schema
from wrap import _use
from ..util import curd_params, mycursor, null_point
from .validation import token_validate


class CommodityListResource(Resource):
    @token_validate
    def post(self, mobile):
        _use(self)
        commodity = commodity_schema.loads(request.data.decode())
        commodity['from_user'] = mobile
        if 'location' not in commodity:
            commodity['location'] = null_point

        with mycursor() as c:
            to_insert = curd_params(commodity, commodity_schema)
            c.execute(f'insert into `commodity` set {to_insert[0]}', to_insert[1])
            commodity_id = c.lastrowid
            return commodity_schema.dump({'id': commodity_id}), 201


class CommodityResource(Resource):
    def get(self, commodity_id):
        _use(self)
        with mycursor(dictionary=True) as c:
            c.execute('select * from commodity where id=%s limit 1', (commodity_id,))
            return commodity_schema.dump(c.fetchone())
