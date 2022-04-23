#include <iostream>
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

int thread_wait_cancel() {
	sleep(5);
	return 0;
}

lt::torrent_handle connect_network() {
	lt::session s;
	lt::add_torrent_params p;
	lt::torrent_handle t;
	lt::error_code ec;
	p.save_path = blockchain::path.erase(blockchain::path.size()- 15, blockchain::path.size());
	p.ti = std::make_shared<lt::torrent_info>(blockchain::torrent_file);
	t = s.add_torrent(p, ec);
	while(s.is_dht_running()){
		break;
	}
	return t;
}

