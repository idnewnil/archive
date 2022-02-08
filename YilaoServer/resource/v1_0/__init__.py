from config.abstractconfig import DBGConfig, Environment, yl
from flask import Flask, send_from_directory
from mkdocs import config
from mkdocs.__main__ import build
# import tempfile
from wrap import send, recv
from .commodity import CommodityListResource, CommodityResource
from .dialog import DialogListResource, DialogResource, DialogUsersResource
from .order import PublicOrderListResource, OrderListResource, OrderResource
from .resource import get_public_resource, ResourceListResource
from .user import UserResource
from .validation import PasswdResource, SMSResource, TokenResource
from ..util import BaseApi, get_rcon


class Api1o0(BaseApi):
    pass


def register_api_1_0(app: Flask):
    api = Api1o0(app)
    name = yl('v1.0/docs_path', False)
    if Environment.rank() == 0:
        # td = tempfile.TemporaryDirectory()
        site_dir = yl('run/site', False)  # td.name
        build.build(config.load_config(f'{__path__[0]}/docs/mkdocs.yml', site_dir=site_dir), dirty=True)
        send(get_rcon(), name, site_dir)
    else:
        site_dir = recv(get_rcon(), name)

    @app.route('/v1.0/docs/', methods=['GET'])
    @app.route('/v1.0/docs/<path:filename>', methods=['GET'])
    def get_doc(filename: str = None):
        if filename is None:
            filename = 'index.html'
        if filename.endswith('/'):
            filename += 'index.html'
        return send_from_directory(site_dir, filename)

    api.add_resource(CommodityListResource, '/v1.0/users/<int:mobile>/commodities')
    api.add_resource(CommodityResource, '/v1.0/commodities/<int:commodity_id>')
    api.add_resource(DialogUsersResource, '/v1.0/users/<int:mobile>/dialogs_users')
    api.add_resource(DialogListResource, '/v1.0/users/<int:mobile>/dialogs_with/<int:other>')
    api.add_resource(DialogResource, '/v1.0/users/<int:mobile>/dialogs')
    api.add_resource(OrderListResource, '/v1.0/users/<int:mobile>/orders')
    api.add_resource(OrderResource, '/v1.0/users/<int:mobile>/orders/<int:order_id>')
    api.add_resource(PublicOrderListResource, '/v1.0/public_orders')
    api.add_resource(ResourceListResource, '/v1.0/users/<int:mobile>/resources')
    api.add_resource(SMSResource, '/v1.0/sms')
    api.add_resource(TokenResource, '/v1.0/users/<int:mobile>/tokens')
    api.add_resource(UserResource, '/v1.0/users/<int:mobile>')
    app.route('/v1.0/users/<int:mobile>/resources/<string:uuid>', methods=['Get'])(get_public_resource)
    if DBGConfig.on:
        pass

    return api
