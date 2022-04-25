#include <iostream>
#include <fstream>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filereadstream.h>
#include <openssl/bn.h>
#include <openssl/evp.h>


namespace rj = rapidjson;


void getAESKey(const char *_N, const char *_T, const char *_x, const char *_x2Tk, unsigned char *aes_key_bytes)
{
    BIGNUM *two = BN_new();
    BIGNUM *T = BN_new();
    BIGNUM *N = BN_new();
    BIGNUM *x = BN_new();
    BIGNUM *x2Tk = BN_new();
    BIGNUM *twoPowerT = BN_new();
    BIGNUM *x2T = BN_new();
    BIGNUM *x2Tinv = BN_new();
    BIGNUM *k = BN_new();
    
    BN_CTX *ctx1 = BN_CTX_new();
    BN_CTX *ctx2 = BN_CTX_new();
    BN_CTX *ctx3 = BN_CTX_new();
    BN_CTX *ctx4 = BN_CTX_new();

    BN_dec2bn(&two, "2");
    BN_dec2bn(&T, _T);
    BN_dec2bn(&N, _N);
    BN_dec2bn(&x, _x);
    BN_dec2bn(&x2Tk, _x2Tk);

    BN_exp(twoPowerT, two, T, ctx1);
    BN_mod_exp(x2T, x, twoPowerT, N, ctx2);
    BN_mod_inverse(x2Tinv, x2T, N, ctx3); // Inverse will always exist by construction
    BN_mod_mul(k, x2Tk, x2Tinv, N, ctx4);

    memset(aes_key_bytes, 0, 32 * sizeof(char));
    BN_bn2bin(k, aes_key_bytes);

    BN_free(two);
    BN_free(T);
    BN_free(N);
    BN_free(x);
    BN_free(x2Tk);
    BN_free(twoPowerT);
    BN_free(x2T);
    BN_free(x2Tinv);
    BN_free(k);
    
    BN_CTX_free(ctx1);
    BN_CTX_free(ctx2);
    BN_CTX_free(ctx3);
    BN_CTX_free(ctx4);
}


int main(int argc, char *argv[])
{
    if (argc != 2){
        std::cerr << "Usage: ./bin/revealShare <path/to/share.json>" << std::endl;
        exit(0);
    }

    FILE *fp = fopen(argv[1], "r");
    char readBuffer[65536];
    rj::FileReadStream inp(fp, readBuffer, sizeof(readBuffer));

    rj::Document keyDoc;
    keyDoc.ParseStream(inp);

    const char *N = keyDoc["N"].GetString();
    const char *T = keyDoc["T"].GetString();
    const char *x = keyDoc["x"].GetString();
    const char *x2Tk = keyDoc["x2Tk"].GetString();

    unsigned char aes_key_bytes[32];
    getAESKey(N, T, x, x2Tk, aes_key_bytes);

    fclose(fp);
    return 0;
}