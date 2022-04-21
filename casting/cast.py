from sys import argv
import subprocess
import ipfshttpclient

def uploadToIPFS(content):
    client = ipfshttpclient.connect()
    hsh = client.add_bytes(content)
    return hsh


if __name__ == "__main__":
    p = subprocess.Popen(["./bin/makevote", argv[1], argv[2]],
                        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, _ = p.communicate()
    hsh = uploadToIPFS(out)

    client = ipfshttpclient.connect()
    print(client.cat(hsh).decode())