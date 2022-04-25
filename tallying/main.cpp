#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include <thfhe.hpp>
#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>


std::vector<LweSample *>* parseVote(std::string path)
{
    std::ifstream dump(path);
    int len, pkN;
    dump >> len >> pkN;

    std::vector<LweSample *>* arr = new std::vector<LweSample *>(len);
    auto params = initialize_gate_bootstrapping_params();

    int tmp;
    for (int i = 0; i < len; i++){
        LweSample *result = new_LweSample(params->in_out_params);
        for (int j = 0; j < pkN; j++){
            dump >> tmp;
            result->a[j] = (Torus32)tmp;
        }
        dump >> tmp;
        result->b = (Torus32)tmp;

        (*arr)[i] = result;
    }

    dump.close();
    return arr;
}

void addVote(std::vector<LweSample *> *acc, std::vector<LweSample *> *vote, const LweParams *params, bool new_acc)
{
    // TODO: Actual Adder circuit
    if (new_acc){
        for (int i = 0; i < vote->size(); i++){
            LweSample *sample = (*vote)[i];
            LweSample *accSample = new_LweSample(params);
            for (int j = 0; j < params->n; j++){
                accSample->a[j] = sample->a[j];
            }
            accSample->b = sample->b;
            accSample->current_variance = 1e-28;
            acc->push_back(accSample);
        }
    }
}


int main(int argc, char *argv[])
{
    if (argc != 2){
        std::cerr << "Usage: ./bin/main <path/to/voteDumps>" << std::endl;
        exit(0);
    }

    auto params = initialize_gate_bootstrapping_params();
    std::vector<LweSample *> *accumulator = NULL;
    

    std::string path(argv[1]);
    for (const auto& entry : std::filesystem::directory_iterator(path)){
        auto arr = parseVote(std::string(entry.path()));
        if (!accumulator){
            accumulator = new std::vector<LweSample *>(0);
            addVote(accumulator, arr, params->in_out_params, true);
        }else{
            addVote(accumulator, arr, params->in_out_params, false);
        }
    }

    std::cout << accumulator->size() << std::endl;
    std::cout << params->in_out_params->n << std::endl;

    for (int i = 0; i < accumulator->size(); i++){
        for (int j = 0; j < params->in_out_params->n; j++){
            std::cout << (int)((*accumulator)[i]->a[j]) << " ";
        }
        std::cout << std::endl;
        std::cout << (int)((*accumulator)[i]->b) << std::endl;
    }

    
    return 0;
}