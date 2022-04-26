#include <tfhe/tfhe.h>
#include <thfhe.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <cstdio>


void exportPK(std::string fpath, ThFHEPubKey *key)
{
    std::ofstream dump(fpath);
    dump << key->n_samples << std::endl;
    dump << key->n << std::endl;
    dump << key->alpha << std::endl;

    for (int i = 0; i < key->n_samples; i++){
        for (int j = 0; j < key->n; j++){
            dump << (int)key->samples[i]->a[j] << " ";
        }
        dump << "\n" << (int)key->samples[i]->b << " " << key->samples[i]->current_variance << std::endl;
    }

    dump.close();

}

void exportTimeLockPuzzle(ThFHEKeyShare *share, int tbit, const std::string fpath)
{
    BIGNUM *p = BN_new();
    BIGNUM *q = BN_new();
    BIGNUM *N = BN_new();
    BIGNUM *T = BN_new();
    BIGNUM *x = BN_new();
    BIGNUM *pm1 = BN_new();
    BIGNUM *qm1 = BN_new();
    BIGNUM *phiN = BN_new();
    BIGNUM *two = BN_new();
    BIGNUM *twoPowerT = BN_new();
    BIGNUM *x2t = BN_new();
    BIGNUM *x2tk = BN_new();
    BIGNUM *aes_key = BN_new();

    BN_CTX *ctx1 = BN_CTX_new();
    BN_CTX *ctx2 = BN_CTX_new();
    BN_CTX *ctx3 = BN_CTX_new();
    BN_CTX *ctx4 = BN_CTX_new();
    BN_CTX *ctx5 = BN_CTX_new();


    // BN_rand(p, 1024, BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ODD);
    // BN_rand(q, 768, BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ODD);
    BN_generate_prime_ex(p, 1024, true, NULL, NULL, NULL);
    BN_generate_prime_ex(q, 768, true, NULL, NULL, NULL);

    BN_rand(T, tbit, BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ODD);
    BN_rand(aes_key, 256, BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ANY);

    BN_mul(N, p, q, ctx1);
    while (true){
        BN_rand(x, 1024, BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ODD);
        BN_CTX *__ctx = BN_CTX_new();
        auto res = BN_mod_inverse(NULL, x, N, __ctx);
        BN_CTX_free(__ctx);
        if (res){
            break;
        }
    }

    BN_sub(pm1, p, BN_value_one());
    BN_sub(qm1, q, BN_value_one());
    BN_mul(phiN, pm1, qm1, ctx2);
    
    BN_dec2bn(&two, "2");
    BN_mod_exp(twoPowerT, two, T, phiN, ctx3);
    BN_mod_exp(x2t, x, twoPowerT, N, ctx4);
    BN_mul(x2tk, x2t, aes_key, ctx5);

    std::ofstream dump(fpath);


    dump << "{" << std::endl;
    dump << "\t\"N\": \"" << BN_bn2dec(N) << "\"," <<std::endl;
    dump << "\t\"T\": \"" << BN_bn2dec(T) << "\"," <<std::endl;
    dump << "\t\"x\": \"" << BN_bn2dec(x) << "\"," <<std::endl;
    dump << "\t\"x2Tk\" : \"" << BN_bn2dec(x2tk) << "\"," << std::endl;

    unsigned char aes_key_byte[32], __key[32], iv[16];
    BN_bn2bin(aes_key, aes_key_byte);

    dump << "\t\"cipher\": [" << std::endl;
    int sz = share->shared_key_repo.size();
    int isz = 0;

    for (auto& kv : share->shared_key_repo){
        int k = kv.second->params->k;

        dump << "\t\t{" << std::endl;
        dump << "\t\t\t\"group\": " << kv.first << "," << std::endl;
        dump << "\t\t\t\"key\": [" << std::endl;

        for (int j = 0; j < k; j++){
            EVP_CIPHER_CTX *cipher_ctx;
            EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha512(),
                            NULL, aes_key_byte, 32, 1,
                            __key, iv);
            cipher_ctx = EVP_CIPHER_CTX_new();
            EVP_EncryptInit_ex(cipher_ctx, EVP_aes_256_cbc(), NULL, __key, iv);

            unsigned char ciphertext[10000];
            int len = 0;
            int cp_len;
            EVP_EncryptUpdate(cipher_ctx, ciphertext, &len,
                                (unsigned char *)(kv.second->key[j].coefs), sizeof(int32_t) * kv.second->key->N);
            cp_len = len;
            EVP_EncryptFinal_ex(cipher_ctx, ciphertext + len, &len);
            cp_len += len;
            EVP_CIPHER_CTX_free(cipher_ctx);

            int encLen = 4 * ((cp_len + 2) / 3) + 100;
            unsigned char *encCipher = new unsigned char[encLen];
            EVP_EncodeBlock(encCipher, ciphertext, cp_len);

            if (j == k - 1)
                dump << "\t\t\t\t\"" << encCipher << "\"" << std::endl;
            else
                dump << "\t\t\t\t\"" << encCipher << "\"," << std::endl;

        }
        dump << "\t\t\t]" << std::endl;
        if (isz == sz - 1)
            dump << "\t\t}" << std::endl;
        else
            dump << "\t\t}," << std::endl;
        isz++;
    }
    
    dump << "\t]" << std::endl;
    dump << "}" << std::endl;
    dump.close();


    BN_free(p);
    BN_free(q);
    BN_free(N);
    BN_free(T);
    BN_free(x);
    BN_free(pm1);
    BN_free(qm1);
    BN_free(phiN);
    BN_free(two);
    BN_free(twoPowerT);
    BN_free(x2t);
    BN_free(x2tk);
    BN_free(aes_key);

}

int main(int argc, char *argv[])
{
    if (argc != 3){
        std::cerr << "Usage: ./main t T" << std::endl;
        exit(0);
    }

    int t = atoi(argv[1]);
    int T = atoi(argv[2]);
    auto genKey = new ThFHE();
    genKey->KeyGen(t, T);

    ThFHEKeyShare *shares = new ThFHEKeyShare[T];
    for (int i = 0; i < T; i++){
        genKey->GetShareSet(i+1, &shares[i]);
    }

    exportPK("keys/pk.key", genKey->pk);

    char name[1000];
    for (int i = 1; i <= T; i++){
        sprintf(name, "keys/share%d.json", i);
        exportTimeLockPuzzle(&shares[i-1], 10, name);
    }

    FILE *fp = fopen("keys/bk.key", "wb");
    export_tfheGateBootstrappingCloudKeySet_toFile(fp, genKey->bk);
    fclose(fp);

    return 0;
}