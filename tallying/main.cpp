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




int main(int argc, char *argv[])
{
    if (argc != 2){
        std::cerr << "Usage: ./bin/main <path/to/voteDumps>" << std::endl;
        exit(0);
    }

    std::string path(argv[1]);
    for (const auto& entry : std::filesystem::directory_iterator(path)){
        std::cout << std::string(entry.path()) << " ";
        auto arr = parseVote(std::string(entry.path()));
        std::cout << arr->size() << std::endl;

    }
    
    return 0;
}