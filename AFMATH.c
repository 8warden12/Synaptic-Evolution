/*
AFMATH.C: defines mathematical functions required for the program

*/
#ifndef AFMATH_C
#define AFMATH_C

#include <math.h>
#include <stdlib.h>

#define TESTBIT(TEST,BIT) (TEST & 1 << BIT)
//Tests whether TEST has bit BIT set

#define SETBIT(SET,BIT) (SET |= 1 << BIT)

#define PI 3.14159265359

#define MIN(x,y) ((x<=y) ? x : y)

struct double_pair{
	double first = 0.0f;
	double second = 0.0f;
};

double_pair normal_rand(float SD, float MEAN)
{
	double U1 = rand()/(float)RAND_MAX;
	double U2 = rand()/(float)RAND_MAX;
	
	double Z1 = sqrt(log(U1) * -2.0)*cos(2*PI*U2);
	double Z2 = sqrt(log(U1) * -2.0)*sin(2*PI*U2);
	
	//Z1 and Z2 are standard normal variables with mean = 0 and SD = 1

	//We now transform them to comply with the SD and the mean that the USER wants
	Z1 = Z1 * SD + MEAN;
	Z2 = Z2 * SD + MEAN;	
	
	double_pair p;
	p.first = Z1;
	p.second = Z2;
	
	//printf("%0.1f %0.1f\n",Z1,Z2);
	return p;
}


#endif

