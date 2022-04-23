#pragma once
#include <iostream>
#include <string>
#include <libtorrent/torrent_handle.hpp>

lt::torrent_handle connect_network();
void print_peers(lt::torrent_handle t);
