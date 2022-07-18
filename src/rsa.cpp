// written with Crypto++ wiki
#include <cryptopp/config_int.h>
#include <cryptopp/queue.h>
#include <cryptopp/secblockfwd.h>
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

CryptoPP::SecByteBlock rsa_wrapper::sign_data(std::string data, CryptoPP::RSA::PrivateKey privkey) {
	CryptoPP::RSASSA_PKCS1v15_SHA_Signer signer(privkey);
	CryptoPP::AutoSeededRandomPool rng;
	CryptoPP::SecByteBlock signature(signer.SignatureLength());
	signer.SignMessage(rng, (CryptoPP::byte const*)data.data(), data.size(), signature);
	CryptoPP::FileSink sink("signed.dat");
	sink.Put((CryptoPP::byte *const)data.data(), data.size());
	CryptoPP::FileSink filesig("sig.dat");
	filesig.Put(signature, signature.size());
	return signature;
}
