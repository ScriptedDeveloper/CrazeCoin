// written with Crypto++ wiki
#include <cryptopp/filters.h>
#include <cstdint>
#include <iostream>
#include <cryptopp/config_int.h>
#include <cryptopp/queue.h>
#include <cryptopp/secblockfwd.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/files.h>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <sstream>
#include "../include/rsa.h"
#include "../include/wallet.h"

CryptoPP::RSA::PrivateKey rsa_wrapper::generate_private_key() {
	CryptoPP::AutoSeededRandomPool numgen;
	CryptoPP::InvertibleRSAFunction params;
	CryptoPP::RSA::PrivateKey privkey(params);
	privkey.GenerateRandomWithKeySize(numgen, 4096); // generating private key, see Crypto++ docs for more details
	return privkey;
}

void rsa_wrapper::write(std::string filename, CryptoPP::BufferedTransformation &bt) { // saves in hex format
	CryptoPP::HexEncoder encoder;
	CryptoPP::FileSink file(filename.c_str());
	bt.CopyTo(encoder);
	bt.MessageEnd();
	encoder.CopyTo(file);
	encoder.MessageEnd();
}

void rsa_wrapper::load(std::string filename, CryptoPP::BufferedTransformation &bt) { // saves in BER format
	CryptoPP::HexDecoder decoder;
	std::ifstream ifs(filename);
	std::stringstream key;
	key << ifs.rdbuf();
	decoder.Put((CryptoPP::byte*)key.str().data(), key.str().size());
	decoder.MessageEnd();
	decoder.CopyTo(bt);
	bt.MessageEnd();
}

std::string rsa_wrapper::raw_hex_decode(std::string hex_str) { // decodes hex signature from stringsource to raw
	CryptoPP::HexDecoder decoder;
	std::string decoded_str;
	decoder.Put((CryptoPP::byte*)hex_str.data(), hex_str.size());
	decoder.MessageEnd();
	CryptoPP::word64 size = decoder.MaxRetrievable();
	if(size <= SIZE_MAX && size) {
		decoded_str.resize(size);
		decoder.Get((CryptoPP::byte *)&decoded_str[0], decoded_str.size());
	}
	return decoded_str;
}

void rsa_wrapper::save_private_key(std::string filename, CryptoPP::RSA::PrivateKey &privkey) {
	CryptoPP::ByteQueue queue;
	privkey.Save(queue);
	CryptoPP::HexEncoder encoder;
	queue.CopyTo(encoder);
	queue.MessageEnd();
	write(filename, queue); 
}

void rsa_wrapper::save_public_key(std::string filename, CryptoPP::RSA::PublicKey &publkey) {
	CryptoPP::ByteQueue queue;
	publkey.Save(queue);
	write(filename, queue); 
}

void rsa_wrapper::load_private_key(std::string filename, CryptoPP::RSA::PrivateKey &privkey) {
	CryptoPP::ByteQueue queue;
	CryptoPP::AutoSeededRandomPool rng;
	load(filename, queue);
	privkey.Load(queue);
	if(!privkey.Validate(rng, 3)) { // checks whether wrong private key
		std::cout << "ALERT! Private key is invalid." << std::endl;
		exit(1);
	}
}

void rsa_wrapper::load_public_key(std::string filename, CryptoPP::RSA::PublicKey &publkey) {
	CryptoPP::ByteQueue queue;
	CryptoPP::AutoSeededRandomPool rng;
	publkey.Load(queue);
	if(!publkey.Validate(rng, 3)) { // simply generating public key from private key
		CryptoPP::RSA::PrivateKey privkey;
		CryptoPP::RSA::PublicKey publkey;
		std::cout << "[RECOVERY] Public key is invalid. Generating new..." << std::endl;
		load_private_key(wallet::path, privkey);
		publkey = CryptoPP::RSA::PublicKey(privkey); // generated key
		save_public_key(filename, publkey);
		std::cout << "Successfully recovered public key. Your public key is stored in " << filename <<". Please don't lose it next time." << std::endl; 
		exit(0);
	}
	load(filename, queue);
}

CryptoPP::SecByteBlock rsa_wrapper::sign_data(std::string data, CryptoPP::RSA::PrivateKey privkey) {
	CryptoPP::RSASSA_PKCS1v15_SHA_Signer signer(privkey);
	CryptoPP::AutoSeededRandomPool rng;
	CryptoPP::SecByteBlock signature(signer.SignatureLength());
	signer.SignMessage(rng, (CryptoPP::byte const*)data.data(), data.size(), signature);
	CryptoPP::FileSink signed_data("signed.dat");
	CryptoPP::ByteQueue sign_queue;
	signed_data.Put((CryptoPP::byte *const)data.data(), data.size());
	sign_queue.Put(signature, signature.size());
	size_t size = signer.MaxSignatureLength();
	write("signature.dat", sign_queue);
	return signature;
}
