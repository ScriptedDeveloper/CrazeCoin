#include <fstream>
#include <ios>
#include <iostream>
#include <libtorrent/alert.hpp>
#include <libtorrent/fwd.hpp>
#include <string>
#include <sstream>
#include <unistd.h>
#include <vector>
#include <thread>
#include <future>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/peer_info.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/session.hpp>
#include <nlohmann/json.hpp> // simply put torrent handle in json file.. problem solved
#include <libtorrent/torrent_info.hpp>
#include "../blockchain/blockchain.h"
#include "broadcast.h"
// Network is basically a Torrent.


void print_peers(lt::torrent_handle t) {
	std::vector<libtorrent::peer_info> vpeer_info;
	t.get_peer_info(vpeer_info);
	for(libtorrent::peer_info s : vpeer_info) {
		std::cout << s.ip << std::endl;
	}
}


lt::torrent_handle connect_network() {
	lt::session s;
	lt::add_torrent_params p;
	lt::torrent_handle t;
	std::string path_temp = blockchain::path;
	path_temp.erase(path_temp.size() - 15, path_temp.size());
	std::ofstream o(path_temp + "peers.json", std::ios_base::in);
	lt::error_code ec;
	nlohmann::json j;
	std::vector<libtorrent::peer_info> vpeer_info;
	std::vector<std::string> peer_ips;
	std::vector<lt::alert*> alerts;
	p.save_path = path_temp;
	p.ti = std::make_shared<lt::torrent_info>(blockchain::torrent_file);
	t = s.add_torrent(p, ec);
	int i = 0;
	while(i != 5){ 
		t.get_peer_info(vpeer_info);
		for(libtorrent::peer_info s : vpeer_info) {
			if(i == 5) {
				o << j;
				o.close();
				goto done;
			}
			j[i] = s.ip.address().to_string(); 
			o << j; 
			i++;
			j = {};
		}
	}
done:
	return t;
}

