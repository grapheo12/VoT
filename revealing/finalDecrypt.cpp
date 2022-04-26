#include "common.hpp"

void import_PartialCipher(TorusPolynomial ***arr, std::string path)
{
    std::ifstream src(path);
    int w, N;
    src >> w >> N;
    (*arr) = new TorusPolynomial*[w];

    for (int i = 0; i < w; i++){
        (*arr)[i] = new_TorusPolynomial(N);

        for (int j = 0; j < N; j++){
            int tmp;
            src >> tmp;
            (*arr)[i]->coefsT[j] = (Torus32)tmp;
        }
    }
    src.close();
}


int main(int argc, char *argv[])
{
    if (argc <= 3){
        std::cerr << "Usage: ./bin/finalDecrypt <path/to/ciphertext.vote> <path/to/config.json> <path/to/partials>... " << std::endl;
        exit(0);
    }

    auto config = DecryptConfig(argv[2]);

    int w;
    auto ciphers = getCipherText(std::string(argv[1]), w);

    TorusPolynomial ***arr = new TorusPolynomial**[config.t];
    for (int i = 0; i < config.t; i++){
        import_PartialCipher(&(arr[i]), std::string(argv[3 + i]));
    }

    TLweParams *params = new_TLweParams(1024, 1, pow(2., -25), pow(2, -15));

    int dmsg = 0;
    for (int i = w-1; i >= 0; i--){
        TLweSample *c = new_TLweSample(params);
        TLweFromLwe(c, ciphers[i], params);
        TorusPolynomial **bit_arr = new TorusPolynomial*[config.t];
        for (int j = 0; j < config.t; j++){
            bit_arr[j] = new_TorusPolynomial(arr[j][i]->N);
            torusPolynomialCopy(bit_arr[j], arr[j][i]);
        }

        int bit = finalDecrypt(c, bit_arr, params, config.parties, config.t, config.T);
        dmsg += (bit << (w - 1 - i));
    }

    std::cout << dmsg << std::endl;
}
