#include <tfhe/tfhe.h>
#include <thfhe.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <openssl/bn.h>


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

void createPuzzle()
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
    BN_CTX *ctx1 = BN_CTX_new();
    BN_CTX *ctx2 = BN_CTX_new();
    BN_CTX *ctx3 = BN_CTX_new();
    BN_CTX *ctx4 = BN_CTX_new();

    BN_rand(p, 1024, BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ODD);
    BN_rand(q, 768, BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ODD);
    BN_rand(x, 1024, BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ODD);
    BN_rand(T, 96, BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ODD);

    BN_mul(N, p, q, ctx1);
    BN_sub(pm1, p, BN_value_one());
    BN_sub(qm1, q, BN_value_one());
    BN_mul(phiN, pm1, qm1, ctx2);
    
    BN_dec2bn(&two, "2");
    BN_mod_exp(twoPowerT, two, T, phiN, ctx3);
    BN_mod_exp(x2t, x, twoPowerT, N, ctx4);

    std::cout << "N = " << BN_bn2dec(N) << std::endl;
    std::cout << "x = " << BN_bn2dec(x) << std::endl;
    std::cout << "x^2^T = " << BN_bn2dec(x2t) << std::endl;

    std::cout << "Slow path" << std::endl;
    BN_exp(twoPowerT, two, T, ctx3);
    BN_mod_exp(x2t, x, twoPowerT, N, ctx4);
    std::cout << "x^2^T = " << BN_bn2dec(x2t) << std::endl;


}

int main(int argc, char *argv[])
{
    // if (argc != 3){
    //     std::cerr << "Usage: ./main t T" << std::endl;
    //     exit(0);
    // }

    // int t = atoi(argv[1]);
    // int T = atoi(argv[2]);
    // auto genKey = new ThFHE();
    // genKey->KeyGen(t, T);

    // ThFHEKeyShare *shares = new ThFHEKeyShare[T];
    // for (int i = 0; i < T; i++){
    //     genKey->GetShareSet(i, &shares[i]);
    // }

    // exportPK("pk.key", genKey->pk);
//////////////////////////////////////////////////////////////////////////

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

    // share0->PartialDecrypt(cipher, tparams, part[0], {1, 2}, 2, 3, 0);
    // share1->PartialDecrypt(cipher, tparams, part[1], {1, 2}, 2, 3, 0);

    // int result = 0;
    // result = finalDecrypt(cipher, part, tparams, {1, 2}, 2, 3);
    // std::cout << result << std::endl;

    createPuzzle();

    return 0;
}