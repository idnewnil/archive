from mysql.connector.cursor import MySQLCursor
from mysql.connector.errorcode import *
from mysql.connector.errors import ProgrammingError
import os


class IterDB:
    def __init__(self, require_version, script, version):
        self.require_version = require_version
        self.version = version
        with open(f'{__path__[0]}{os.sep}{script}.sql', encoding='utf8') as f:
            self.script = f.read()


class UserVersion:
    @classmethod
    def get(cls, c: MySQLCursor):
        try:
            c.execute('select user_version()')
            return c.fetchone()[0]
        except ProgrammingError as e:
            if e.errno != ER_SP_DOES_NOT_EXIST:
                raise

    @classmethod
    def update_to(cls, version, c: MySQLCursor):
        c.execute('drop function if exists user_version')
        c.execute('create function user_version() returns text deterministic no sql return %s', (version,))


iter_table = [
    IterDB(None, 'ms1', 'v1.0')
]
