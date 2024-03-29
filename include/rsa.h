#pragma once
#include <cryptopp/queue.h>
#include <iostream>
#include <cryptopp/hex.h>
#include <cryptopp/secblockfwd.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/rsa.h>

namespace rsa_wrapper {
	CryptoPP::RSA::PrivateKey generate_private_key();
	void save_private_key(std::string filename, const CryptoPP::RSA::PrivateKey &privkey);
	void save_public_key(std::string filename, const CryptoPP::RSA::PublicKey &publkey);
	void load_private_key(std::string filename, CryptoPP::RSA::PrivateKey &privkey);
	void load(std::string filename, CryptoPP::ByteQueue &bt);
	void load_public_key(std::string filename, CryptoPP::RSA::PublicKey &publkey);
	void save_signature(std::string filename, CryptoPP::ByteQueue bt);
	std::string raw_hex_decode(std::string hex_str);
	CryptoPP::SecByteBlock sign_data(std::string data, CryptoPP::RSA::PrivateKey);
	std::string hex_encode(CryptoPP::SecByteBlock signature);
}
