
[Return to Main Page](../../)
## Table of Contents
- [Project Structure](#Project-Structure)
- [broadcast.cpp](#broadcast-.h)
- [blockchain.cpp](#blockchain.cpp)
- [wallet.cpp](#wallet.cpp)
- [rsa.cpp](#rsa.cpp)
- [block.cpp](#block.cpp)

## Project Structure
The projects consists of 3 folders.\

The [src](src/) folder contains all source files, while [include](include/) contains the header files. The [src](src/) folder contains the main function of the 2 programs.

## broadcast.cpp
This source file contains all functions responsible for the Blockchain Network. It includes a simple TCP Server/Client, which recieves/sends blockchains or transactions to other nodes. 90% of the time went into this. [broadcast.cpp](src/broadcast.cpp) also contains all functions related to adding transactions/blocks to the current blockchain JSON file.

## blockchain.cpp
[blockchain.cpp](src/blockchain.cpp) contains all functions related to checking the blockchain JSON file and retrieving data from it. The function blockchain::init_blockchain() is also the entry point of the miner programs which is inside this source file.

## wallet.cpp
[wallet.cpp](src/wallet.cpp) is responsible for the wallet application and is connected to [broadcast.cpp](src/broadcast.cpp) because it accesses the broadcast::send_chain() function to broadcast the transactions into the network.

## rsa.cpp
This source file is simply a wrapper for the [Crypto++](https://www.cryptopp.com/) library. All cryptocraphic operations are made by that library.

## block.cpp
[block.cpp](src/block.cpp) is an OOP implementation of a block in the blockchain. In this source file the blocks are getting mined and contains extensions of [broadcast.cpp](src/broadcast.cpp).
