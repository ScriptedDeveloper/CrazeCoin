#pragma once
#include <cryptopp/secblockfwd.h>
#include <iostream>
#include <cryptopp/cryptlib.h>
#include <cryptopp/rsa.h>

namespace rsa_wrapper {
	CryptoPP::RSA::PrivateKey generate_private_key();
	void save_private_key(std::string filename, CryptoPP::RSA::PrivateKey &privkey);
	void write(std::string filename, CryptoPP::BufferedTransformation &bt);
	void save_public_key(std::string filename, CryptoPP::RSA::PublicKey &publkey);
	void load_private_key(std::string filename, CryptoPP::RSA::PrivateKey &privkey);
	void load(std::string filename, CryptoPP::BufferedTransformation &bt);
	void load_public_key(std::string filename, CryptoPP::RSA::PublicKey &publkey);
	CryptoPP::SecByteBlock sign_data(std::string data, CryptoPP::RSA::PrivateKey);
}
