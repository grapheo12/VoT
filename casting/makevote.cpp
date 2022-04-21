#include <tfhe/tfhe.h>
#include <thfhe.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <cstring>


void importPK(std::string fpath, ThFHEPubKey *key)
{
    std::ifstream src(fpath);
    src >> key->n_samples >> key->n >> key->alpha;
    key->samples = new LweSample*[key->n_samples];
    auto mainParams = initialize_gate_bootstrapping_params();

    for (int i = 0; i < key->n_samples; i++){
        int tmp;
        key->samples[i] = new_LweSample(mainParams->in_out_params);
        for (int j = 0; j < key->n; j++){
            src >> tmp;
            key->samples[i]->a[j] = (Torus32)tmp;
        }
        src >> tmp;
        key->samples[i]->b = (Torus32)tmp;
        src >> key->samples[i]->current_variance;
    }
    src.close();
}


int main(int argc, char *argv[])
{
    if (argc != 3){
        std::cout << "Usage: ./makevote <path/to/pk.key> <voteencoding>" << std::endl;
        exit(0);
    }

    auto pk = new ThFHEPubKey(NULL, 0);

    importPK(std::string(argv[1]), pk);

    int len = strlen(argv[2]);
    auto params = initialize_gate_bootstrapping_params();
    
    std::cout << len << std::endl;
    std::cout << pk->n << std::endl;

    for (int i = 0; i < len; i++){
        int bit = (argv[2][i] - '0') & 1;
        LweSample *result = new_LweSample(params->in_out_params);
        pk->Encrypt(result, bit);
        for (int j = 0; j < pk->n; j++){
            std::cout << (int)result->a[j] << " ";
        }
        std::cout << std::endl;
        std::cout << (int)result->b << std::endl;
        free_LweSample(result);
    }


    return 0;
}