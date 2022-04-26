from sys import argv
import requests

n = int(argv[1])
for i in range(n):
    rq = requests.post(f"http://127.0.0.1:8080/{i}")
    print(rq.text)
    print("\n\n")

