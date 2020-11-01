#ifndef SIMULATOR_CONSTANTS_C
#define SIMULATOR_CONSTANTS_C
#include <vector>
#include <stdio.h>
using namespace std;

#define SIGNOF(x) ((x == 0) ? 0 : ((x < 0) ? -1 : 1))
#define FLOAT_ABS(x) (float)((x < 0.0f) ? -x : x)

const float eA_THRESHOLD = 150.0f; //old: 10.0f
const float eMOTOR_THRESHOLD = 300.0f; //old: 20.0f

const float SUM_DECAY_RATE = 0.950f;
//the rate at which the sum decays per cycle

const float STC_DECAY_RATE = 1.0f;
//the factor to decay the short-term change in strength every cycle


const float juice_decrement = 0.21f;
//how much to drain the juice value when current passes through a connection

const float juice_increment = 0.001f;
//how much to naturally regenerate the juice (every time step)

const float juice_threshold = 50.0f; //remember thy semicolons

const float firing_juice_threshold = 40.0f;
//Used for operant conditioning: If a neuron's firing_juice dips below this, then it has fired significantly.

const float food_reward_OC = 0.5f;
const float poison_penalty_OC = 0.5f;

const float DEBATE_DIFFERENCE_WIN_THRESHOLD = 1.0f;
const float DEBATE_WIN_THRESHOLD = 10.0f;
//When the ratio of one score to the other of a synapse exceeds this, the higher score's related change is applied to the synapse
//see inhibitory_support_score and excitatory_support_score

const float weight_max = 100.0f;
const float weight_min = -100.0f;

const float default_baseline_str_lts = 1.0f;

#define TIME_STEP 0.1f
#define RF01() (rand()/(double)RAND_MAX)

#endif
