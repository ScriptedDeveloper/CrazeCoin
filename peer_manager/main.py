from flask import request, jsonify
import webserver, json, os

def recover_format():
    format = {"peers" : [], "peer_amount" : 0}
    if(os.path.getsize("peers.json") == 0):
        with open("peers.json", "w") as peers_write:
            json.dump(format, peers_write)



class API:
    def __init__(self):
        self.peers_json = json.load(open("peers.json", "r"))

    def check_peer(self, peer):
        for peer in self.peers_json["peers"]:
            if peer == request.remote_addr:
                return False
        return True
 
    def add_peer(self):
        if self.check_peer(request.remote_addr) == False:
            return jsonify({"status" : 203, "message" : "SIGNED"})
        self.peers_json["peer_amount"] += 1
        self.peers_json["peers"].append(str(request.remote_addr))
        open("peers.json", "w").write(json.dumps(self.peers_json))
        self.peers_json = json.load(open("peers.json", "r"))
        return jsonify({"status" : 200, "message" : "SUCCESS"})
 
    def get_peers(self):
        return str(json.dumps(self.peers_json["peers"]))

    def retrieve_peer_amount(self):
        return str(self.peers_json["peer_amount"])


if(__name__ == '__main__'):
    recover_format()
    webserver.server().start() 
        
    
