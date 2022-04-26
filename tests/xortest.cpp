#include <tfhe/tfhe.h>
#include <thfhe.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <cstdio>


int main()
{
    int t = 3;
    int T = 5;
    auto genKey = new ThFHE();
    genKey->KeyGen(t, T);
    auto params = initialize_gate_bootstrapping_params();

    LweSample *one = new_LweSample(params->in_out_params);
    LweSample *zero = new_LweSample(params->in_out_params);
    genKey->pk->Encrypt(one, 1);
    genKey->pk->Encrypt(zero, 0);
    // bootsSymEncrypt(one, 1, genKey->sk);
    // bootsSymEncrypt(zero, 0, genKey->sk);

    LweSample *res = new_LweSample(params->in_out_params);
    bootsXOR(res, one, zero, genKey->bk);
    
    std::cout << bootsSymDecrypt(res, genKey->sk) << std::endl;
    std::cout << bootsSymDecrypt(one, genKey->sk) << std::endl;
    std::cout << bootsSymDecrypt(zero, genKey->sk) << std::endl;

    ThFHEKeyShare *share1 = new ThFHEKeyShare();
    ThFHEKeyShare *share2 = new ThFHEKeyShare();
    ThFHEKeyShare *share3 = new ThFHEKeyShare();

    genKey->GetShareSet(1, share1);
    genKey->GetShareSet(2, share2);
    genKey->GetShareSet(3, share3);
    TLweParams *tparams = new_TLweParams(1024, 1, pow(2., -25), pow(2, -15));

    TLweSample *tres = new_TLweSample(tparams);
    TLweFromLwe(tres, res, tparams);
    
    TorusPolynomial **poly = new TorusPolynomial*[3];
    poly[0] = new_TorusPolynomial(1024);
    share1->PartialDecrypt(tres, tparams, poly[0], {1, 2, 3}, 3, 5, 0.0001);
    poly[1] = new_TorusPolynomial(1024);
    share2->PartialDecrypt(tres, tparams, poly[1], {1, 2, 3}, 3, 5, 0.0001);
    poly[2] = new_TorusPolynomial(1024);
    share3->PartialDecrypt(tres, tparams, poly[2], {1, 2, 3}, 3, 5, 0.0001);

    int msg = finalDecrypt(tres, poly, tparams, {1, 2, 3}, 3, 5);
    std::cout << msg << std::endl;


    return 0;
}