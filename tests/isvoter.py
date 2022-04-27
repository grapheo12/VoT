import json
from sys import argv, exit
from web3 import Web3
from web3.middleware import geth_poa_middleware


config = json.load(open(argv[1]))
w3 = Web3(Web3.IPCProvider("privnet/node0/geth.ipc"))
w3.middleware_onion.inject(geth_poa_middleware, layer=0)

contract = w3.eth.contract(address=config["voterContract"],
                            abi=config["voterAbi"])

ret = contract.functions.isVoter(argv[2]).call()
print(ret)
