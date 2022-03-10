/**
 * \author Andrey Popov
 *
 * Interface to use ROOT TRandom3 in a pure C application.
 */

#ifndef CTRANDOM_H
#define CTRANDOM_H

#ifdef __cplusplus
extern "C"
{
#endif

    void CTRandom_SetSeed(unsigned int seed);
    double CTRandom_Rndm();
    unsigned int CTRandom_Integer(unsigned int max);

#ifdef __cplusplus
}
#endif

#endif
