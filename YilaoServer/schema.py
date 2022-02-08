from functools import partial
from marshmallow import Schema, fields

__all__ = ['PointSchema', 'UserSchema', 'ResourceSchema', 'DialogSchema', 'CommoditySchema', 'OrderSchema',
           'user_schema', 'resource_schema', 'dialog_schema', 'commodity_schema', 'order_schema']


def varchar(s, n):
    return len(s) <= n


varchar32 = partial(varchar, n=32)
varchar64 = partial(varchar, n=64)
text = partial(varchar, n=65535)

fields.DateTime = partial(fields.DateTime, format='%Y-%m-%d %H:%M:%S')


class PointSchema(Schema):
    longitude = fields.Float(validate=lambda it: -180 <= it <= 180)
    latitude = fields.Float(validate=lambda it: -90 <= it <= 90)
    name = fields.Str(allow_none=True, validate=varchar64)


class UserSchema(Schema):
    mobile = fields.Int()
    passwd = fields.Str(load_only=True, validate=lambda it: 8 <= len(it) <= 16)
    nickname = fields.Str(validate=varchar32)
    sex = fields.Str(allow_none=True, validate=lambda it: it in ['male', 'female'])
    portrait = fields.Int()
    default_location = fields.Nested(PointSchema, allow_none=True)
    mark = fields.Str(dump_only=True, allow_none=True, validate=lambda it: it in ['test'])
    create_at = fields.DateTime(dump_only=True)
    id_name = fields.Str(validate=varchar32)
    id_school = fields.Str(validate=varchar32)
    id_photo = fields.Str(validate=lambda it: len(it) <= 36)


class ResourceSchema(Schema):
    uuid = fields.Str(dump_only=True)
    name = fields.Str(validate=varchar32)
    from_user = fields.Int(dump_only=True)
    create_at = fields.DateTime(dump_only=True)


class DialogSchema(Schema):
    class Meta:
        ordered = True

    id = fields.Int(dump_only=True)
    content = fields.Str()
    from_user = fields.Int(dump_only=True)
    to_user = fields.Int()
    send_at = fields.DateTime(dump_only=True)
    type = fields.Str()


class CommoditySchema(Schema):
    id = fields.Int(dump_only=True)
    name = fields.Str(validate=varchar32)
    from_user = fields.Int(dump_only=True)
    location = fields.Nested(PointSchema, allow_none=True)
    on_offer = fields.Bool()
    price = fields.Float()
    sales_volume = fields.Float(dump_only=True)
    photo = fields.Int(allow_none=True)


class OrderSchema(Schema):
    class Meta:
        ordered = True

    id = fields.Int(dump_only=True)
    from_user = fields.Int(dump_only=True)
    phone = fields.Int()
    destination = fields.Nested(PointSchema, allow_none=True)
    emergency_level = fields.Str(validate=lambda it: it in ['normal', 'urgent'])
    create_at = fields.DateTime(dump_only=True)
    receive_at = fields.DateTime(dump_only=True)
    executor = fields.Int(dump_only=True)
    close_at = fields.DateTime(dump_only=True)
    close_state = fields.Str(validate=lambda it: it in ['finish', 'cancel'])
    type = fields.Str(validate=varchar32)
    category = fields.Str()
    detail = fields.Str(allow_none=True, validate=text)
    protected_info = fields.Str(allow_none=True, validate=text)
    count = fields.Int()
    reward = fields.Float()
    in_at = fields.DateTime(allow_none=True)
    out_at = fields.DateTime(allow_none=True)
    id_photo = fields.Str(dump_only=True)
    photos = fields.Str()
    name = fields.Str()
    id_name = fields.Str()
    id_photo1 = fields.Str()
    id_name1 = fields.Str()


user_schema = UserSchema()
resource_schema = ResourceSchema()
dialog_schema = DialogSchema()
commodity_schema = CommoditySchema()
order_schema = OrderSchema()
