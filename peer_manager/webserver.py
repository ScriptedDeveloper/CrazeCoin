"""
CrazeCoin, a semi-decentralised work-in-progress CryptoCurrency
Copyright (C) 2022  ScriptedDev
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""


from flask import Flask, request, jsonify
import main
app = Flask(__name__)

class WebWrapper():
    def __init__(self, app):
        self.app = app

    def add_endpoint(self, url, name, function, methods=["GET"]):
        self.app.add_url_rule(url, name, function, methods=methods)

    def run(self):
        self.app.run(port=6882, host="0.0.0.0")


class server(WebWrapper):
    def __init__(self):
        self.api = main.API()
        self.wrapper = WebWrapper(app)

    def start(self):
        self.wrapper.add_endpoint("/peers", "peers", self.api.retrieve_peer_amount, methods=["GET"])
        self.wrapper.add_endpoint("/add_peer", "add_peer", self.api.add_peer, methods=["GET"])
        self.wrapper.add_endpoint("/add_pending_peers", "add_pending_peer", self.api.add_pending_peer, methods=["GET"])
        self.wrapper.add_endpoint("/get_peers", "get_peers", self.api.get_peers, methods=["GET"])
        self.wrapper.add_endpoint("/pending_peers", "pending_peers", self.api.retrieve_pending_peers, methods=["GET"])
        self.wrapper.run()



