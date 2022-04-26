#include "common.hpp"

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


int main(int argc, char *argv[])
{
    if (argc != 5){
        std::cerr << "Usage: ./bin/partialDecrypt <path/to/keyShare.json> <path/to/cipherText.vote> <path/to/config.json> <path/to/output>" << std::endl;
        exit(0);
    }

    auto share = getKeyShare(argv[1]);
    TLweParams *params = new_TLweParams(1024, 1, pow(2., -25), pow(2, -15));
    auto config = DecryptConfig(argv[3]);

    std::ofstream dump(argv[4]);

    int w;
    auto ciphers = getCipherText(std::string(argv[2]), w);
    TLweSample **arr = new TLweSample*[w];
    
    dump << w << std::endl;
    dump << params->N << std::endl;

    for (int i = 0; i < w; i++){
        arr[i] = new_TLweSample(params);
        TLweFromLwe(arr[i], ciphers[i], params);
        auto partial = new_TorusPolynomial(params->N);
        share->PartialDecrypt(arr[i], params, partial, config.parties, config.t, config.T, 0.0001);
        for (int j = 0; j < partial->N; j++){
            dump << (int)partial->coefsT[j] << " ";
        }
        dump << std::endl;
    }

    dump.close();

    return 0;
}