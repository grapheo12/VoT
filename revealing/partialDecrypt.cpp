#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <vector>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filereadstream.h>
// #include <openssl/bn.h>
// #include <openssl/evp.h>
#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <thfhe.hpp>

namespace rj = rapidjson;


ThFHEKeyShare *getKeyShare(char *path)
{
    ThFHEKeyShare *share = new ThFHEKeyShare();
    TLweParams *params = new_TLweParams(1024, 1, pow(2., -25), pow(2, -15));

    rj::Document shareDoc;
    FILE *fp = fopen(path, "r");
    char readBuffer[65536];
    rj::FileReadStream inp(fp, readBuffer, sizeof(readBuffer));

    shareDoc.ParseStream(inp);

    auto arr = shareDoc.GetArray();
    for (auto& x: arr){
        int gid = x["group"].GetInt();
        auto keydata = x["unlocked_key"].GetArray();
        TLweKey *key = new_TLweKey(params);
        for (int i = 0; i < params->k; i++){
            for (int j = 0; j < params->N; j++){
                key->key[i].coefs[j] = keydata[i][j].GetInt();
            }
        }

        share->shared_key_repo[gid] = key;
    }

    fclose(fp);

    return share;
}

struct PartialDecryptConfig {
    int t, T;
    std::vector<int> parties;

    PartialDecryptConfig(char *path);
};

PartialDecryptConfig::PartialDecryptConfig(char *path)
{
    rj::Document configDoc;
    FILE *fp = fopen(path, "r");
    char readBuffer[65536];
    rj::FileReadStream inp(fp, readBuffer, sizeof(readBuffer));
    configDoc.ParseStream(inp);

    this->t = configDoc["t"].GetInt();
    this->T = configDoc["T"].GetInt();
    auto arr = configDoc["parties"].GetArray();
    for (int i = 0; i < this->t; i++){
        this->parties.push_back(arr[i].GetInt());
    }
    fclose(fp);
}


int main(int argc, char *argv[])
{
    if (argc != 4){
        std::cerr << "Usage: ./bin/partialDecrypt <path/to/keyShare.json> <path/to/cipherText.json> <path/to/config.json>" << std::endl;
        exit(0);
    }

    auto share = getKeyShare(argv[1]);
    TLweParams *params = new_TLweParams(1024, 1, pow(2., -25), pow(2, -15));
    auto config = PartialDecryptConfig(argv[3]);

    

    


    
    return 0;
}