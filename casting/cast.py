from hashlib import sha256
from sys import argv
import subprocess
from uuid import uuid4

def uploadToIPFS(content):
    # TODO: Change to IPFS calls
    digest = sha256(content)
    hsh = digest.hexdigest()
    with open(f"db/{hsh}.vote", "wb") as f:
        f.write(content)


if __name__ == "__main__":
    p = subprocess.Popen(["./bin/makevote", argv[1], argv[2]],
                        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, _ = p.communicate()
    uploadToIPFS(out)