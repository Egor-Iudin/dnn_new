#include "CTRandom.h"
#include "TRandom3.h"

TRandom3 generator;

void CTRandom_SetSeed(unsigned int seed)
{
    generator.SetSeed(seed);
}

double CTRandom_Rndm()
{
    return generator.Rndm();
}

unsigned int CTRandom_Integer(unsigned int max)
{
    return generator.Integer(max);
}
