#pragma once

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

struct DecryptConfig {
    int t, T;
    std::vector<int> parties;

    DecryptConfig(char *path);
};

DecryptConfig::DecryptConfig(char *path)
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

LweSample** getCipherText(std::string path, int& w)
{
    std::ifstream src(path);
    int width, n;
    src >> width >> n;
    auto params = initialize_gate_bootstrapping_params();
    LweSample **arr = new LweSample*[width];
    for (int i = 0; i < width; i++){
        arr[i] = new_LweSample(params->in_out_params);
        for (int j = 0; j < n; j++){
            src >> arr[i]->a[j];
        }
        src >> arr[i]->b >> arr[i]->current_variance;
    }

    src.close();
    w = width;
    return arr;
}
