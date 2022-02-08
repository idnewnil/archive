from config.abstractconfig import Environment, ResourceConfig
from flask import request, send_from_directory, Response
from flask_restful import Resource
import os
from schema import resource_schema
from ..util import mycursor
from uuid import uuid4
from .validation import test, token_validate
from werkzeug.utils import secure_filename


def get_path(mobile, visibility='public'):
    return f'{Environment.root_dir}/data/{mobile}/{visibility}'


class ResourceListResource(Resource):
    def post(self, mobile):
        if mobile != 0 and not test(token_validate, mobile=mobile):
            return Response(status=401)
        fs = request.files.getlist('file')
        uuids = []
        for f in fs:
            uuid = str(uuid4())
            name = secure_filename(f.filename)
            path = get_path(mobile)
            if not os.path.exists(path):
                os.makedirs(path)
            f.save(os.path.join(path, uuid))
            if mobile != 0:
                with mycursor() as c:
                    c.execute(
                        'insert into resource(uuid, name, from_user, create_at) values (%s, %s, %s, current_timestamp)',
                        (uuid, name, mobile))
            uuids.append(uuid)
        return resource_schema.dump({'uuid': ','.join(uuids)}), 201


def get_public_resource(mobile, uuid):
    return send_from_directory(get_path(mobile), uuid)
