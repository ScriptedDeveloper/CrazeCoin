from flask import Flask, request, jsonify
from main import API
app = Flask(__name__)

class WebWrapper():
    def __init__(self, app):
        self.app = app

    def add_endpoint(self, url, name, function, methods=["GET"]):
        self.app.add_url_rule(url, name, function, methods=methods)

    def run(self):
        self.app.run()


class server(WebWrapper):
    def __init__(self, API, wrapper):
        self.api = API()
        self.wrapper = WebWrapper(app)

    def start(self):
        self.wrapper.add_endpoint("/peers", "peers", self.api.retrieve_peer_amount, methods=["GET"])
        self.wrapper.add_endpoint("/add_peer", "add_peer", self.api.add_peer, methods=["POST"])
        self.wrapper.run()



