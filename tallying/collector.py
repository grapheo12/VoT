import json
from uuid import uuid4
from web3 import Web3
from web3.exceptions import ContractLogicError
from web3.middleware import geth_poa_middleware
import ipfshttpclient
import os


config = json.load(open("config.json"))
w3 = Web3(Web3.IPCProvider(config["ipcPath"]))
w3.middleware_onion.inject(geth_poa_middleware, layer=0)
contract = w3.eth.contract(address=config["casterContract"],
                            abi=config["casterAbi"])

ipfsClient = ipfshttpclient.connect()

if not os.path.exists("dump"):
    os.mkdir("dump")


def getAllCids():
    global contract

    cids = []
    i = 0
    while True:
        try:
            ret = contract.functions.votes(i).call()
            i += 1
            cids.append(ret)
        except ContractLogicError:
            break

    return cids


def getFromIPFS(cid):
    global ipfsClient
    content = ipfsClient.cat(cid)
    return content
    

def filterDump(cid, content):
    n_candidates = int(content.decode().split("\n")[0].strip())
    if n_candidates != config["n_candidates"]:
        return

    with open(f"dump/{cid}_{str(uuid4())}", "wb") as f:
        f.write(content)



if __name__ == "__main__":
    cids = getAllCids()
    blacklist = set(config["blacklist"])
    for cid in cids:
        if cid in blacklist:
            continue
        content = getFromIPFS(cid)
        n_candidates = int(content.decode().split("\n")[0].strip())
        print(cid, n_candidates)
        filterDump(cid, content)