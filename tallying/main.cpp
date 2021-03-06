#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include <deque>
#include <thfhe.hpp>
#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <chrono>
#include <omp.h>

#define MAX_HW_WIDTH 5
#define MAX_ACC_WIDTH 16
#define __SZ 4

namespace chr = std::chrono;


template <int WIDTH>
struct Accumulator{
    std::vector<LweSample *> c;
    ThFHEPubKey *pk;
    TFheGateBootstrappingCloudKeySet *bk;
    LweParams *params;

    Accumulator(ThFHEPubKey *_pk, TFheGateBootstrappingCloudKeySet *_bk, const LweParams *_params)
    {
        pk = _pk;
        bk = _bk;
        params = (LweParams *)_params;
        // 0 -> MSB, WIDTH-1 -> LSB
        for (int i = 0; i < WIDTH; i++){
            LweSample *sample = new_LweSample(params);
            pk->Encrypt(sample, 0);
            c.push_back(sample);
        }
    }

    void fakeAddBit(LweSample *bit)
    {
        c[WIDTH-1] = bit;
    }

    void AddBit(LweSample *bit)
    {
        auto start = chr::high_resolution_clock::now();
        // Add single bit to WIDTH sized accumulator
        LweSample *carry = new_LweSample(params);
        pk->Encrypt(carry, 0);
        carry->current_variance = 0.0001;
        LweSample *dummy = new_LweSample(params);

        LweSample *res = new_LweSample(params);
        LweSample *res2 = new_LweSample(params);
        for (int i = WIDTH-1; i >= 0; i--){
            lweClear(res, params);
            lweClear(res2, params);

            if (i == WIDTH-1){

                /*
                    0000 1
                       1 1
                */
                // Don't worry about carry here
                // It will always be zero.
                bootsXOR(res, c[i], bit, bk);
                // lweCopy(res, dummy, params);
                bootsAND(res2, c[i], bit, bk);
            }else{
                bootsXOR(res, c[i], carry, bk);
                bootsAND(res2, c[i], carry, bk);
            }

            lweCopy(carry, res2, params);
            lweCopy(c[i], res, params);
        }
        delete_LweSample(res);
        delete_LweSample(res2);

        auto end = chr::high_resolution_clock::now();
        auto duration = chr::duration_cast<chr::milliseconds>(end - start);

        std::cerr << "AddBit Time: " << duration.count() << "ms" << std::endl;

    }

    LweSample* Compare(int val)
    {
        LweSample *acc = new_LweSample(params);
        LweSample *res = new_LweSample(params);
        LweSample *res2 = new_LweSample(params);
        LweSample *cbit = new_LweSample(params);
        pk->Encrypt(acc, 1);

        for (int i = WIDTH-1; i >= 0; i--){
            lweClear(res, params);
            lweClear(res2, params);
            lweClear(cbit, params);

            int bit = val & 1;
            pk->Encrypt(cbit, bit);
            val >>= 1;

            bootsXNOR(res, c[i], cbit, bk);
            bootsAND(res2, acc, res, bk);
            lweCopy(acc, res2, params);
        }
        free_LweSample(cbit);
        free_LweSample(res);
        free_LweSample(res2);

        return acc;
    }

    void Export(std::string path)
    {
        std::ofstream dump(path);
        dump << WIDTH << std::endl;
        dump << params->n << std::endl;
        // MSB -> LSB
        for (int i = 0; i < WIDTH; i++){
            for (int j = 0; j < params->n; j++){
                dump << c[i]->a[j] << " ";
            }
            dump << std::endl;

            dump << c[i]->b << std::endl;
            dump << c[i]->current_variance << std::endl;
        }
        dump.close();
    }

    Accumulator<WIDTH>& operator+=(const Accumulator<WIDTH>& other)
    {
        LweSample *carry = new_LweSample(params);
        pk->Encrypt(carry, 0);
        LweSample *halfadd = new_LweSample(params);
        LweSample *halfcarry1 = new_LweSample(params);
        LweSample *halfcarry2 = new_LweSample(params);

        for (int i = WIDTH - 1; i >= 0; i--){
            lweClear(halfadd, params);
            lweClear(halfcarry1, params);
            lweClear(halfcarry2, params);

            bootsXOR(halfadd, this->c[i], other.c[i], bk);
            bootsAND(halfcarry1, this->c[i], other.c[i], bk);

            bootsXOR(this->c[i], carry, halfadd, bk);
            bootsAND(halfcarry2, halfadd, carry, bk);

            bootsOR(carry, halfcarry1, halfcarry2, bk);
        }

        return *this;
    }
};




std::vector<LweSample *>* parseVote(std::string path, ThFHEPubKey *pk, TFheGateBootstrappingCloudKeySet *bk)
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

    auto start = chr::high_resolution_clock::now();
    // Hamming Weight
    auto hw = new Accumulator<MAX_HW_WIDTH>(pk, bk, params->in_out_params);
    for (int i = 0; i < arr->size(); i++){
        auto x = (*arr)[i];
        hw->AddBit(x);
    }
    
    auto cmp = hw->Compare(1);

    for (int i = 0; i < arr->size(); i++){
        auto x = (*arr)[i];

        LweSample *res = new_LweSample(params->in_out_params);
        LweSample *zero = new_LweSample(params->in_out_params);
        pk->Encrypt(zero, 0);
        bootsMUX(res, cmp, x, zero, bk);
        lweCopy((*arr)[i], res, params->in_out_params);
        free_LweSample(res);
        free_LweSample(zero);

    }

    auto end = chr::high_resolution_clock::now();
    auto duration = chr::duration_cast<chr::milliseconds>(end - start);
    std::cerr << "Vote parse time: " << duration.count() << "ms" << std::endl;

    return arr;
}


// Defunct now.
template<int WIDTH>
struct VoteArray{
    std::vector<LweSample *> arr;
    ThFHEPubKey *pk;
    TFheGateBootstrappingCloudKeySet *bk;
    LweParams *params;

    VoteArray(ThFHEPubKey *_pk, TFheGateBootstrappingCloudKeySet *_bk, const LweParams *_params)
    {
        pk = _pk;
        bk = _bk;
        params = (LweParams*)_params;
    }

    void AddBit(LweSample *b)
    {
        arr.push_back(b);
    }

    Accumulator<WIDTH>* Add()
    {
        std::deque<Accumulator<WIDTH>*> q;

        int n = arr.size() / __SZ;
        if (arr.size() % __SZ > 0) n++;

        #pragma omp parallel for num_threads(4)
        for (int i = 0; i < n; i++){
            int start = i * __SZ;
            int end = start + __SZ - 1;
            if (end > arr.size() - 1) end = arr.size() - 1;

            Accumulator<WIDTH> *acc = new Accumulator<WIDTH>(pk, bk, params);
            for (int j = start; j <= end; j++){
                acc->AddBit(arr[i]);
            }

            #pragma omp critical
            {
                q.push_back(acc);
            }
        }

        std::cout << "Vec Size: " << q.size() << std::endl;


        while (q.size() > 1){
            int vec_sz = q.size();
            std::cout << "Vec Size: " << vec_sz << std::endl;

            int n = vec_sz / __SZ;
            if (vec_sz % __SZ > 0) n++;

            #pragma omp parallel for num_threads(4)
            for (int i = 0; i < n; i++){
                int start = i * __SZ;
                int end = start + __SZ - 1;

                Accumulator<WIDTH> *ans = new Accumulator<WIDTH>(pk, bk, params);
                for (int j = start; j <= end; j++){
                    Accumulator<WIDTH> *tmp = q[j];
                    (*ans) += (*tmp);
                }

                #pragma omp critical
                {
                    q.push_back(ans);
                }
            }

            while (vec_sz > 0){
                q.pop_front();
                vec_sz--;
            }
        }

        return q.front();
    }
};


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
    if (argc != 5){
        std::cerr << "Usage: ./bin/main <path/to/voteDumps> <path/to/pubkey> <path/to/bootstrappingKey> <path/to/output>" << std::endl;
        exit(0);
    }

    auto params = initialize_gate_bootstrapping_params();

    FILE *bkFp = fopen(argv[3], "rb");
    auto bk = new_tfheGateBootstrappingCloudKeySet_fromFile(bkFp);
    fclose(bkFp);
    
    ThFHEPubKey *pk = new ThFHEPubKey(NULL, 0);
    importPK(std::string(argv[2]), pk);

    Accumulator<MAX_ACC_WIDTH> **count = NULL;
    int cntLen = -1;
    std::vector<LweSample*> *__acc = NULL;
    
    std::string path(argv[1]);
    for (const auto& entry : std::filesystem::directory_iterator(path)){
        auto arr = parseVote(std::string(entry.path()), pk, bk);
        if (count == NULL){
            count = new Accumulator<MAX_ACC_WIDTH>*[arr->size()];
            cntLen = arr->size();
            for (int i = 0; i < arr->size(); i++){
                count[i] = new Accumulator<MAX_ACC_WIDTH>(pk, bk, params->in_out_params);
            }

            __acc = new std::vector<LweSample *>[arr->size()];
        }

        for (int i = 0; i < arr->size(); i++){
            __acc[i].push_back((*arr)[i]);
        }
    }

    #pragma omp parallel for num_threads(8)
    for (int i = 0; i < cntLen; i++){
        for (auto x: __acc[i]){
            count[i]->AddBit(x);
        }
    }

    char name[1000];
    for (int i = 0; i < cntLen; i++){
        sprintf(name, "%s/result%d.vote", argv[4], i + 1);
        count[i]->Export(std::string(name));
    }

    return 0;
}