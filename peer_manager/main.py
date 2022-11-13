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

from flask import request, jsonify
from json import dumps
import webserver, json, os

def recover_format():
    format = {"peers" : [], "pending_peers" : [], "peer_amount" : 0}
    if(os.path.getsize("peers.json") == 0):
        with open("peers.json", "w") as peers_write:
            json.dump(format, peers_write)

class API:
    def __init__(self):
        self.peers_json = json.load(open("peers.json", "r"))

    def save_peerlist(self):
        open("peers.json", "w").write(dumps(self.peers_json)) # updating peer_json for other funcs

    def check_peer(self, peer, remove = False): 
        it = 0
        for peer in self.peers_json["peers"]:
            if peer == request.remote_addr:
                if remove:
                    self.peers_json["peers"].pop(it)
                return False
            it += 1
        return True # Returning true if not in JSON file
 
    def check_pend_peer(self, peer, remove = False):
        it = 0
        for peer in self.peers_json["pending_peers"]:
            if peer == request.remote_addr:
                if remove:
                    self.peers_json["pending_peers"].pop(it)
                return False
            it += 1
        return True # Returning true if not in JSON file
 
    def check_pending(self, pend_miner):
        for pend in self.peers_json["pending_peers"]:
            if pend == request.remote_addr:
                return False

        return True # Returning true if not in JSON file

    def add_peer(self):
        if self.check_peer(request.remote_addr) == False:
            return jsonify({"status" : 203, "message" : "SIGNED"}) # IP addr is in json file, denied
        self.peers_json["peer_amount"] += 1
        self.peers_json["peers"].append(str(request.remote_addr))
        open("peers.json", "w").write(json.dumps(self.peers_json)) # dumping all changes to file
        self.save_peerlist()
        return jsonify({"status" : 200, "message" : "SUCCESS"}) 
 
    def add_pending_peer(self): # same code as add_peer() with minor changes
        if self.check_peer(request.remote_addr) == False:
            return jsonify({"status" : 203, "message" : "SIGNED"})
        self.peers_json["pending_peers"].append(str(request.remote_addr))
        open("peers.json", "w").write(json.dumps(self.peers_json))
        self.save_peerlist()
        return jsonify({"status" : 200, "message" : "SUCCESS"})

    def remove_pending_peer(self):
        if self.check_pend_peer(request.remote_addr, True) == True:
            return jsonify({"status" : "203", "message" : "UNKNOWN"})
        self.peers_json["peer_amount"] -= 1
        self.save_peerlist()
        return jsonify({"status" : 200, "message" : "SUCCESS"})

    def remove_peer(self):
        if self.check_pend_peer(request.remote_addr, True) == True:
            return jsonify({"status" : "203", "message" : "UNKNOWN"})
        self.peers_json["peer_amount"] -= 1
        self.save_peerlist()
        return jsonify({"status" : 200, "message" : "SUCCESS"})

    def get_peers(self):
        return str(json.dumps(self.peers_json["peers"]))

    def retrieve_peer_amount(self):
        return str(self.peers_json["peer_amount"])
    
    def retrieve_pending_peers(self):
        return str(json.dumps(self.peers_json["pending_peers"]))

if(__name__ == '__main__'):
    recover_format()
    webserver.server().start() 
        
 
