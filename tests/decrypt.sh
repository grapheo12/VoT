#!/bin/sh

./revealing/bin/partialDecrypt revealing/keys/share1.json tallying/results/result1.vote revealing/config.json revealing/ciphers/result1_1.partial
./revealing/bin/partialDecrypt revealing/keys/share2.json tallying/results/result1.vote revealing/config.json revealing/ciphers/result1_2.partial
./revealing/bin/partialDecrypt revealing/keys/share3.json tallying/results/result1.vote revealing/config.json revealing/ciphers/result1_3.partial

./revealing/bin/partialDecrypt revealing/keys/share1.json tallying/results/result2.vote revealing/config.json revealing/ciphers/result2_1.partial
./revealing/bin/partialDecrypt revealing/keys/share2.json tallying/results/result2.vote revealing/config.json revealing/ciphers/result2_2.partial
./revealing/bin/partialDecrypt revealing/keys/share3.json tallying/results/result2.vote revealing/config.json revealing/ciphers/result2_3.partial

./revealing/bin/partialDecrypt revealing/keys/share1.json tallying/results/result3.vote revealing/config.json revealing/ciphers/result3_1.partial
./revealing/bin/partialDecrypt revealing/keys/share2.json tallying/results/result3.vote revealing/config.json revealing/ciphers/result3_2.partial
./revealing/bin/partialDecrypt revealing/keys/share3.json tallying/results/result3.vote revealing/config.json revealing/ciphers/result3_3.partial

./revealing/bin/partialDecrypt revealing/keys/share1.json tallying/results/result4.vote revealing/config.json revealing/ciphers/result4_1.partial
./revealing/bin/partialDecrypt revealing/keys/share2.json tallying/results/result4.vote revealing/config.json revealing/ciphers/result4_2.partial
./revealing/bin/partialDecrypt revealing/keys/share3.json tallying/results/result4.vote revealing/config.json revealing/ciphers/result4_3.partial

./revealing/bin/partialDecrypt revealing/keys/share1.json tallying/results/result5.vote revealing/config.json revealing/ciphers/result5_1.partial
./revealing/bin/partialDecrypt revealing/keys/share2.json tallying/results/result5.vote revealing/config.json revealing/ciphers/result5_2.partial
./revealing/bin/partialDecrypt revealing/keys/share3.json tallying/results/result5.vote revealing/config.json revealing/ciphers/result5_3.partial

./revealing/bin/finalDecrypt tallying/results/result1.vote revealing/config.json revealing/ciphers/result1_1.partial revealing/ciphers/result1_2.partial revealing/ciphers/result1_3.partial
./revealing/bin/finalDecrypt tallying/results/result2.vote revealing/config.json revealing/ciphers/result2_1.partial revealing/ciphers/result2_2.partial revealing/ciphers/result2_3.partial
./revealing/bin/finalDecrypt tallying/results/result3.vote revealing/config.json revealing/ciphers/result3_1.partial revealing/ciphers/result3_2.partial revealing/ciphers/result3_3.partial
./revealing/bin/finalDecrypt tallying/results/result4.vote revealing/config.json revealing/ciphers/result4_1.partial revealing/ciphers/result4_2.partial revealing/ciphers/result4_3.partial
./revealing/bin/finalDecrypt tallying/results/result5.vote revealing/config.json revealing/ciphers/result5_1.partial revealing/ciphers/result5_2.partial revealing/ciphers/result5_3.partial


