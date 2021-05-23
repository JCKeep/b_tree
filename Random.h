#ifdef _cplusplus
#if _cplusplus
extern "C" {
#endif /* _cplusplus */
#endif /* _cplusplus */



#ifndef RANDOM_NUMBER_H
#define RANDOM_NUMBER_H

#include <limits.h>


static unsigned long Seed = 1;

#define A 48271L
#define M 2147483647L
#define Q (M / A)
#define R (M % A)

double Random(void)
{
	long TmpSeed;
	
	TmpSeed = A * (Seed % Q) - R * (Seed / Q);
	if (TmpSeed >= 0)
		Seed = TmpSeed;
	else
		Seed = TmpSeed + M;
	
	return (double) Seed / M;
}

void RandomInitialize(unsigned long InitVal)
{
	Seed = InitVal;
}

int RandomInt(void)
{	
	int random = INT_MAX;
	return (int)(random * Random());
}


#endif /* RANDOM_NUMBER_H */



#ifdef _cplusplus 
#if _cplusplus 
}
#endif /* _cplusplus */
#endif /* _cplusplus */
