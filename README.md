
# CrazeCoin

A simple CryptoCurrency on **its own** Blockchain written in C++. Its goal is to be a functioning CryptoCurrency with the ability to be easily self-hosted.


## Features

- Ability to send and recieve transactions within the network
- Block mining possible
- Wallet can send and recieve transactions
  - Generating wallet private key also possible 
- Cross platform

## Deployment

To start CrazeCoin, you need a peer-tracker and (at least) one CrazeCoin-Instance. 

Start peer-tracker:
```bash
  python peer_tracker/main.py
```

Start CrazeCoin-Instance:
```bash
  make run
```
You need to input the IP/Domain of the Peer-Tracker to get started.

Generate Wallet Privat key:
```bash 
  make && ./wallet.out <private key file name> generate
```

## FAQ

#### Is this just another ShitCoin attempt? 

**No**. CrazeCoin itself is not a Crypto Currency, but provides the functionality to create hobbiest CryptoCurrencys at ease without fiddling with the Bitcoin Source. Unlike other CryptoCurrencys, CrazeCoin runs on its **own Blockchain**, which makes it so unique.

#### Cool! So I can now create my own very secure CryptoCurrency which will also go to the Moon, right?

Again, no. CrazeCoin is designed with security in mind, BUT isn't near any production-stable application. This means CrazeCoin should NOT be used for serious attempts of creating a Crypto Currency since this is an educational project after all.

#### When will CrazeCoin be finished?

I can't say for sure. I'm the only person working on this project and have to go to school as well, so CrazeCoin should be considered a long-term project.

#### Is this safe to be port forwared to the internet?

No, I wouldn't recommend doing that. I did not test CrazeCoin against potential attacks. A VPN would be safer.

# Contributions
Any form of contribution is welcome since I'm only a solo developer working on this project.

# Documentation
[You can visit the Documentation here.](docs.md)

## Questions?
Feel free to open a issue or contact me personally through contact@crazeserver.de
