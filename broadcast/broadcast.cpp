#define PORT 9000
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <thread>
#include <future>
#include <arpa/inet.h>
#include <errno.h>
#include <cpr/cpr.h>
#include <cpr/api.h>
//#include <libtorrent/entry.hpp> // Network is basically a Torrent.
//#include <libtorrent/alert.hpp>
//#include <libtorrent/fwd.hpp>
//#include <libtorrent/bencode.hpp>
//#include <libtorrent/peer_info.hpp>
//#include <libtorrent/torrent_handle.hpp>
//#include <libtorrent/session.hpp>
//#include <libtorrent/torrent_info.hpp>
#include <nlohmann/json.hpp>
#include "../blockchain/blockchain.h"
#include "broadcast.h"


void error_handler(std::string message) {
	std::cout << "Failed at : " << message << std::endl;
	std::cout << strerror(errno) << std::endl;
	sleep(3);
}


std::string retrieve_peer(int n) {
	std::ifstream ifs(blockchain::peer_path);
	nlohmann::json j = j.parse(ifs);
	return j[n]; // returning n element at the moment
}

void clear_peers() {
	std::ofstream ofs(blockchain::peer_path);
	ofs << "{}";
	ofs.close();
}


/*
int check_node(std::string ip) { // optimal solution for now, will implement senders/recievers list later
	cpr::Response rip = cpr::Get(cpr::Url{"https://api.ipify.org/"});
	if(ip == rip.text) {
		return 1;
	}
	return 0;
}
*/

int recieve_chain() {
	struct sockaddr_in sockaddr;
	int isocket, client_fd;
 	isocket = socket(AF_INET, SOCK_STREAM, 0);
	int optional, inet;
	char buff[1024] = {0}; // recieving blockchain
	std::ofstream ofsBlock;
	if(isocket < 0) {
		error_handler("SOCKET CREATION");
		recieve_chain();
	}
	/*if(check_node(retrieve_peer(0)) == 1) { Thanks to ISP only 1 IP for all devices!
		next_peer = retrieve_peer(1);
	}*/
	sockaddr.sin_port = htons(PORT);
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	inet = inet_pton(AF_INET, retrieve_peer(0).c_str(), &sockaddr.sin_addr);
	if(inet <= 0) {
		error_handler("INET_PTON : return code " + std::to_string(inet));
		recieve_chain();
	}
	client_fd = connect(isocket, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
	if(client_fd < 0) {
		error_handler("CONNECT");
		recieve_chain();
	}
	std::cout << "Waiting for blockchain..." << std::endl;
	read(isocket, buff, 1024);
	buff[strlen(buff)] = '\0';
	ofsBlock.open(blockchain::path);
	std::cout << buff << std::endl;
	ofsBlock << buff;
	return 0;
}

int send_chain() {
	struct sockaddr_in sockaddr;
	int opt = 1, new_socket, server_fd = socket(AF_INET, SOCK_STREAM, 0);
	std::ifstream ifs(blockchain::path);
	nlohmann::json j = j.parse(ifs);
	std::string json_str = j.dump();
	std::string next_peer = retrieve_peer(0);
	char json_char[1024];
	int i;
	for(i = 0; i < json_str.size(); i++) {
		json_char[i] = json_str[i];
	}
	json_char[i+1] = '\0';
	/*if(check_node(retrieve_peer(0)) == 1) { Thanks to ISP only 1 IP for all devices!
		next_peer = retrieve_peer(1);
	}
	*/
	if(server_fd == 0) {
		error_handler("SERVER_FD");
		send_chain();
	}
	if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) != 0) {
		error_handler("SETSOCKOPT");
		send_chain();
	}
	sockaddr.sin_port = htons(PORT);
	sockaddr.sin_family = AF_INET;
	if(bind(server_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr))< 0) {
		error_handler("BIND");
		send_chain();
	}
	if(listen(server_fd, 5) < 0) {
		error_handler("LISTEN");
		send_chain();
	}
	std::cout << "\nConnecting to peer : " << next_peer << std::endl;
	socklen_t size = sizeof(sockaddr);
	if((new_socket = accept(server_fd, (struct sockaddr*)&sockaddr, &size)) < 0) {
		error_handler("ACCEPT");
	} 
	if (send(new_socket, json_char, strlen(json_char), 0) == -1) {
		error_handler("SEND");
	}
	close(new_socket);
	shutdown(server_fd, SHUT_RDWR);
	return 0;
}

int signup_peer() {
	cpr::Response rpeer_server = cpr::Post(cpr::Url{blockchain::peer_tracker});
	return 0;
}

int get_peers() {
	std::ofstream ofsPeer(blockchain::peer_path);
	cpr::Response rpeer = cpr::Get(cpr::Url{blockchain::peer_tracker + "/peers/"});
	ofsPeer << rpeer.text;
	ofsPeer.close();
	return 0;
}
/*
int get_peers() {
	lt::session s;
	lt::add_torrent_params p;
	lt::torrent_handle t;
	std::string path_temp = blockchain::path;
	path_temp.erase(path_temp.size() - 15, path_temp.size());
	std::ofstream o(blockchain::peer_path);
	lt::error_code ec;
	nlohmann::json j;
	std::vector<libtorrent::peer_info> vpeer_info;
	std::vector<std::string> peer_ips;
	std::vector<lt::alert*> alerts;
	p.save_path = path_temp;
	p.ti = std::make_shared<lt::torrent_info>(blockchain::torrent_file);
	t = s.add_torrent(p, ec);
	int i = 0;
	while(i != 3){ 
		t.get_peer_info(vpeer_info);
		for(libtorrent::peer_info s : vpeer_info) {
			if(i == 3) {
				goto done;
			}
			j[i] = s.ip.address().to_string(); 
			if(!j.empty()) {
				i++;
			}
		}
		if(i != 3 && !j.empty()) {
			break;
		}
	}
	done:		
		o << j;
		o.close();

	return 0;
}


*/
