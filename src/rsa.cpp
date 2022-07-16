// written with Crypto++ wiki
#include <cryptopp/queue.h>
#include <iostream>
#include <cryptopp/cryptlib.h>
#include <cryptopp/files.h>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include "../include/rsa.h"

CryptoPP::RSA::PrivateKey rsa_wrapper::generate_private_key() {
	CryptoPP::AutoSeededRandomPool numgen;
	CryptoPP::InvertibleRSAFunction params;
	CryptoPP::RSA::PrivateKey privkey(params);
	privkey.GenerateRandomWithKeySize(numgen, 4096); // generating private key, see Crypto++ docs for more details
	return privkey;
}

void rsa_wrapper::write(std::string filename, CryptoPP::BufferedTransformation &bt) {
	CryptoPP::FileSink file(filename.c_str());
	bt.CopyTo(file);
	file.MessageEnd();
}

void rsa_wrapper::load(std::string filename, CryptoPP::BufferedTransformation &bt) {
	CryptoPP::FileSource file(filename.c_str(), true);
	file.TransferTo(bt);
	bt.MessageEnd();
}	

void rsa_wrapper::save_private_key(std::string filename, CryptoPP::RSA::PrivateKey &privkey) {
	CryptoPP::ByteQueue queue;
	privkey.DEREncodePrivateKey(queue);
	write(filename, queue);
}

void rsa_wrapper::save_public_key(std::string filename, CryptoPP::RSA::PublicKey &publkey) {
	CryptoPP::ByteQueue queue;
	publkey.DEREncodePublicKey(queue);
	write(filename, queue);
}

void rsa_wrapper::load_private_key(std::string filename, CryptoPP::RSA::PrivateKey &privkey) {
	CryptoPP::ByteQueue queue;
	load(filename, queue);
	privkey.BERDecodePrivateKey(queue, false, queue.MaxRetrievable());
}

void rsa_wrapper::load_public_key(std::string filename, CryptoPP::RSA::PublicKey &publkey) {
	CryptoPP::ByteQueue queue;
	load(filename, queue);
	publkey.BERDecodePublicKey(queue, false, queue.MaxRetrievable());
}
