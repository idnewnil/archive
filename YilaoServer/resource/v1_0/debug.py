from config.abstractconfig import DBGConfig
from wrap import _use
from flask import request, Response
from flask_restful import Resource
import json
import traceback
from ..util import get_rcon, exc, mycursor

_use(DBGConfig)


class CMDResource(Resource):
    class Error(Exception):
        pass

    class UnknownCmdTypeError(Exception):
        pass

    def post(self):
        data = json.loads(request.data)
        type_ = data['type']
        cmd = data['cmd']
        try:
            if type_ == 'python':
                return eval(cmd), 201
            elif type_ == 'mysql':
                with mycursor(dictionary=True) as c:
                    c.execute(cmd)
                    res = c.fetchall()
                    return res, 201
            elif type_ == 'redis':
                get_rcon().execute_command(cmd)
                return Response(status=201)
            else:
                raise self.UnknownCmdTypeError
        except self.UnknownCmdTypeError:
            return exc('unknown cmd type', True), 400
        except Exception as e:
            _use(e)
            print(traceback.format_exc())
            return exc('发生内部错误'), 500
