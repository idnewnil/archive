from flask import Flask, redirect
from .v1_0 import register_api_1_0 as _register_api_1_0


def register_api(app: Flask):
    @app.route('/')
    def goto_docs():
        return redirect('/v1.0/docs/')

    _register_api_1_0(app)
