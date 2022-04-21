import asyncio
import json
import random
from aiohttp import web
from web3 import Web3
from web3.middleware import geth_poa_middleware
import secrets
from eth_account import Account

config = json.load(open("config.json", "r"))
w3 = Web3(Web3.IPCProvider(config["ipcPath"]))
w3.middleware_onion.inject(geth_poa_middleware, layer=0)
contract = w3.eth.contract(address=config["voterContract"],
                            abi=config["voterAbi"])
txFrom = w3.eth.accounts[0]


VOTER_IDS = set()
PUB_KEYS = []
LCK = asyncio.Lock()

MAX_SIZE = 10

def gen_key():
    print("Generating Keys")
    # priv_key = Ed25519PrivateKey.generate()
    # pub_key = priv_key.public_key()

    priv_key = "0x" + secrets.token_hex(32)
    ac = Account.from_key(priv_key)
    pub_key = ac.address

    return priv_key, pub_key

def random_insert(k):
    global PUB_KEYS
    if len(PUB_KEYS) == 0:
        PUB_KEYS.append(k)
        return
    
    idx = random.randint(0, len(PUB_KEYS) - 1)
    PUB_KEYS = PUB_KEYS[:idx] + [k] + PUB_KEYS[idx:]

async def clientHandler(request):
    global PUB_KEYS, VOTER_IDS, LCK
    resp = None
    async with LCK:
        voter_id = request.match_info.get('voter_id', 'NULL')
        if voter_id == "NULL" or voter_id in VOTER_IDS:
            resp = web.Response(status=302, text="ID already registered / Unacceptable format")
        else:
            priv_key, pub_key = gen_key()

            VOTER_IDS.add(voter_id)
            random_insert(pub_key)

            keys = f"""#----------- PRIVATE KEY -------#
{priv_key}
#----------- PUBLIC KEY  -------#
{pub_key}"""
            resp = web.Response(status=200, text=keys)


    return resp


async def randomInsertor():
    global LCK

    while True:
        stime = float(random.randint(1, 5))
        await asyncio.sleep(stime)

        async with LCK:
            _, pub_key = gen_key()
            random_insert(pub_key)


async def writeToLedger(keys):
    global txFrom, contract

    print(f"Dispatching {len(keys)} keys:")

    for k in keys:
        w3.eth.send_transaction({
            "to": k,
            "from": txFrom,
            "value": int(100e+9)
        })
        ret = contract.functions.registerVoter(k).transact({
            "from": txFrom
        })
        print("Key:", k, "Return:", ret)
    
async def keyDispatch():
    global LCK, MAX_SIZE, PUB_KEYS

    while True:
        await asyncio.sleep(0.5)
        pub_keys = None
        try:
            async with LCK:
                if len(PUB_KEYS) >= MAX_SIZE:
                    pub_keys = PUB_KEYS[:]
                    PUB_KEYS = []
                else:
                    continue

            await writeToLedger(pub_keys)
        except Exception as e:
            print(e)


if __name__ == "__main__":
    app = web.Application()
    app.add_routes([
        web.post("/{voter_id}", clientHandler)
    ])

    loop = asyncio.new_event_loop()
    rIns = loop.create_task(randomInsertor())
    kD = loop.create_task(keyDispatch())
    web.run_app(app, loop=loop)







