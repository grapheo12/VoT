#include <tfhe/tfhe.h>
#include <thfhe.hpp>
#include <iostream>

int main()
{
    auto genKey = new ThFHE();
    genKey->KeyGen(2, 3);

    auto share0 = new ThFHEKeyShare();
    auto share1 = new ThFHEKeyShare();
    auto share2 = new ThFHEKeyShare();

    auto params = initialize_gate_bootstrapping_params();
    auto tparams = new_TLweParams(params->in_out_params->n, 1, params->in_out_params->alpha_min, params->in_out_params->alpha_max);
    
    int bit = 0;
    auto lweCipher = new_LweSample(params->in_out_params);
    genKey->pk->Encrypt(lweCipher, bit);
    auto cipher = new_TLweSample(tparams);
    TLweFromLwe(cipher, lweCipher, tparams);

    genKey->GetShareSet(1, share0);
    genKey->GetShareSet(2, share1);
    genKey->GetShareSet(3, share2);

    TorusPolynomial *part[2];
    part[0] = new_TorusPolynomial(params->in_out_params->n);
    part[1] = new_TorusPolynomial(params->in_out_params->n);

    share0->PartialDecrypt(cipher, tparams, part[0], {1, 2}, 2, 3, 0);
    share1->PartialDecrypt(cipher, tparams, part[1], {1, 2}, 2, 3, 0);

    int result = 0;
    result = finalDecrypt(cipher, part, tparams, {1, 2}, 2, 3);
    std::cout << result << std::endl;


    return 0;
}