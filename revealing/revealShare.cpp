#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
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

void decryptKeyShare(rj::Document& key, unsigned char *aes_key_bytes)
{
    unsigned char dkey[32];
    unsigned char iv[16];

    EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha512(), NULL,
                    aes_key_bytes, 32, 1, dkey, iv);

    auto& alloc = key.GetAllocator();
    auto cipher = key["cipher"].GetArray();
    for (auto& x : cipher){
        auto keys = x["key"].GetArray();
        rj::Value objValue;
        objValue.SetArray();
        for (auto& y : keys){
            auto enc = y.GetString();
            unsigned char *c = new unsigned char[strlen(enc)];
            int cp_len = EVP_DecodeBlock(c, (const unsigned char *)enc, strlen(enc));
            
            unsigned char dmsg[5000];
            int len = 0, plen = 0;

            EVP_CIPHER_CTX *dctx = EVP_CIPHER_CTX_new();
            EVP_DecryptInit_ex(dctx, EVP_aes_256_cbc(), NULL, dkey, iv);
            EVP_DecryptUpdate(dctx, dmsg, &len, c, cp_len);
            plen += len;
            EVP_DecryptFinal_ex(dctx, dmsg + len, &len);
            plen += len;
            dmsg[plen] = '\0';
            int32_t *coefs = (int32_t *)dmsg;      
            
            rj::Value uArr;
            uArr.SetArray();

            for (int k = 0; k < 1024; k++){
                uArr.PushBack(coefs[k], alloc);
            }

            EVP_CIPHER_CTX_free(dctx);
            delete c;

            objValue.PushBack(uArr, alloc);

        }
        x.AddMember("unlocked_key", objValue, alloc);
        x.RemoveMember("key");
    }
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

    decryptKeyShare(keyDoc, aes_key_bytes);
    keyDoc.RemoveMember("N");
    keyDoc.RemoveMember("T");
    keyDoc.RemoveMember("x");
    keyDoc.RemoveMember("x2Tk");


    rj::StringBuffer buffer;
    rj::Writer<rj::StringBuffer> writer(buffer);
    keyDoc["cipher"].Accept(writer);

    std::cout << std::setw(4) << buffer.GetString() << std::endl;

    fclose(fp);
    return 0;
}