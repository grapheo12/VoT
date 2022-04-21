import json
from sys import argv, exit
import subprocess
import ipfshttpclient
from web3 import Web3
from eth_account import Account
from web3.middleware import geth_poa_middleware


config = json.load(open("config.json"))
w3 = Web3(Web3.IPCProvider(config["ipcPath"]))
w3.middleware_onion.inject(geth_poa_middleware, layer=0)

contract = w3.eth.contract(address=config["casterContract"],
                            abi=config["casterAbi"])


def uploadToIPFS(content):
    client = ipfshttpclient.connect()
    hsh = client.add_bytes(content)
    return hsh


if __name__ == "__main__":
    if len(argv) != 4:
        print("Usage: python3 cast.py <path/to/pub.key> <vote> <WalletPrivKey>")
        exit(0)

    p = subprocess.Popen(["./bin/makevote", argv[1], argv[2]],
                        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, _ = p.communicate()
    hsh = uploadToIPFS(out)

    print("Uploaded to IPFS")

    ac = w3.eth.account.from_key(argv[3])
    nonce = w3.eth.get_transaction_count(ac.address)
    print("Available balance:", w3.eth.get_balance(ac.address),
            "GasPrice:", w3.eth.gas_price)
    tx = {
        "nonce": nonce,
        "from": ac.address,
        "gas": int(1e+6),
        "gasPrice": 20,
        "value": 0,
        "chainId": 1515
    }
    txn = contract.functions.castVote(hsh).buildTransaction(tx)
    signed = w3.eth.account.sign_transaction(txn, argv[3])
    ret = w3.eth.send_raw_transaction(signed.rawTransaction)
    print("Ledger updated. Return:", ret)

