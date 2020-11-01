#ifndef CONSTANT_C
#define CONSTANT_C
#include <math.h>
#include <stdlib.h>

#define E 2.71828
#define THRESHOLD 5.0f
#define ACTIVATION_CHANCE(x) (x>THRESHOLD) ? 1.0f : pow(x/(float)THRESHOLD,E)
#define RF01() (rand()/(double)RAND_MAX)

//I'm considering changing this to an INT
#define TIME_STEP 0.1f


#define INIT_WEIGHT_NUMERATOR 0.1f

//for AFR
#define RECENT_CACHE_LENGTH 80
#define BASELINE_CACHE_LENGTH 1000
//monitoring protocols activate if the recent activation rate exceeds the baseline activation
//rate times this factor
#define BASELINE_PLASTICITY_TRIGGER_FACTOR 1.2f
#define PLASTICITY_STRENGTHENING_FACTOR 100.0f

//if the absolute value of the residual sum is greater than RESIDUE_LIMIT, then synaptic scaling occurs.
#define RESIDUE_LIMIT 5.0f 
#define RESIDUE_DECAY 0.93f //factor to decay the RESIDUE by each time step

#define SIGM(x) (double)exp(x)/(1.0f + (double)exp(x))

#define DOWNSCALE_C(t) (-1.0f/(5.0f*t + 1.0f) + 1.0f)

//by the Alternating thoughts theorem, the strengthening of inhibition should, on average,
//be weaker than the strengthening of excitation.
//#define INHIBITORY_STRENGTHENING_MULTIPLIER 0.2f

//To prevent barbell-level weights
#define PLASTICITY_MULTIPLIER(CURRENT_WEIGHT) (0.05f/(100*CURRENT_WEIGHT + 1))

#define STS_MAX 1.0f
#define STS_MIN 1.0f

#define SE_LRNING_PLASTICITY_FACTOR 50.0f
#define SE_LRNING_FACTOR_DECAY 0.9f
#define SE_LRNING_FACTOR_MIN 10.0f

#endif
