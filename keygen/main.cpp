#include <tfhe/tfhe.h>
#include <thfhe.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <openssl/bn.h>
#include <openssl/evp.h>


void exportPK(std::string fpath, ThFHEPubKey *key)
{
    std::ofstream dump(fpath);
    dump << key->n_samples << std::endl;

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


    BN_rand(p, 1024, BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ODD);
    BN_rand(q, 768, BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ODD);
    BN_rand(x, 1024, BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ODD);
    BN_rand(T, tbit, BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ODD);
    BN_rand(aes_key, 256, BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ANY);

    BN_mul(N, p, q, ctx1);
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
                                (unsigned char *)(kv.second->key[0].coefs), sizeof(int32_t) * kv.second->key->N);
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


    // DECRYPT PART
    // EVP_CIPHER_CTX *dctx;
    // unsigned char dkey[32];
    // unsigned char div[16];
    // unsigned char dmsg[1000];

    // EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha512(), NULL,
    //                 aes_key_byte, 32, 1, dkey, div);
    
    // dctx = EVP_CIPHER_CTX_new();
    // EVP_DecryptInit_ex(dctx, EVP_aes_256_cbc(), NULL, dkey, div);
    // int plen = 0;
    // EVP_DecryptUpdate(dctx, dmsg, &len, ciphertext, cp_len);
    // plen += len;

    // EVP_DecryptFinal_ex(dctx, dmsg + len, &len);
    // plen += len;
    // dmsg[plen] = '\0';

    // dump << dmsg << std::endl;
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

    exportPK("pk.key", genKey->pk);

    // auto params = initialize_gate_bootstrapping_params();
    // auto tparams = new_TLweParams(params->in_out_params->n, 1, params->in_out_params->alpha_min, params->in_out_params->alpha_max);
    
    // int bit = 0;
    // auto lweCipher = new_LweSample(params->in_out_params);
    // genKey->pk->Encrypt(lweCipher, bit);
    // auto cipher = new_TLweSample(tparams);
    // TLweFromLwe(cipher, lweCipher, tparams);

    // TorusPolynomial *part[2];
    // part[0] = new_TorusPolynomial(params->in_out_params->n);
    // part[1] = new_TorusPolynomial(params->in_out_params->n);

    // shares[0].PartialDecrypt(cipher, tparams, part[0], {1, 2}, 2, 3, 0);
    // shares[1].PartialDecrypt(cipher, tparams, part[1], {1, 2}, 2, 3, 0);

    // int result = 0;
    // result = finalDecrypt(cipher, part, tparams, {1, 2}, 2, 3);
    // std::cout << result << std::endl;

    exportTimeLockPuzzle(&shares[0], 96, "share0.json");
    exportTimeLockPuzzle(&shares[1], 96, "share1.json");
    exportTimeLockPuzzle(&shares[2], 96, "share2.json");

    return 0;
}