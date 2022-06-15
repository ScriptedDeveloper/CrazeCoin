from flask import request, jsonify
import webserver
import json


class API:
    def __init__(self):
        self.peers_json = json.load(open("peers.json", "r"))

    def check_peer(self, peer):
        for peer in self.peers_json["peers"]:
            if peer == request.remote_addr:
                return False
        return True
        

    def add_peer(self):
        peer = request.get_json()[request.remote_addr]
        if self.check_peer(request.remote_addr) == False:
            return jsonify({"status" : 203, "message" : "SIGNED"})
        self.peers_json["peers_amount"] += 1
        self.peers_json["peers"][self.peers_json["peer_amount"]] = peer
        open("peers.json", "w").write(self.peers_json)
        self.peers_json = json.load(open("peers.json", "r"))
        return jsonify({"status" : 200, "message" : "SUCCESS"})
    
    def retrieve_peer_amount(self):
        return str(self.peers_json["peer_amount"])


if(__name__ == '__main__'):
    webserver.server().start() 
        
    
