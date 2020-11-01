#ifndef THE_MATRIX_C
#define THE_MATRIX_C

#include <set>
#include <algorithm>
#include <vector>
#include <stdlib.h>
#include "NC2.C"
#include "SIMULATOR_CONSTANTS.C"
#include "curses.h"
#undef getch
#include "NW_T.c"

#include <conio.h>

#define FOOD_DENSITY 0.1f
#define POISON_DENSITY 0.08f

bool LOCK_PLASTICITY = false;


int foodEaten = 0;
int poisonEaten = 0;

int trialFoodEaten = 0;
int trialPoisonEaten = 0;

using namespace std;

#define WORLD_Y 20
#define WORLD_X 40

#define VALIDPOS(y,x) (x > 0 && y > 0 && x < WORLD_X-1 && y < WORLD_Y-1) ? true : false
#define FOOD_REWARD 2
#define POISON_PENALTY 10
#define MOVE_INTO_WALL_PENALTY 10
#define REWARD_EXERCISE 0

char world[WORLD_Y][WORLD_X];

vector<bool> pattern_space = {1,0,0,0};
vector<bool> pattern_poison = {0,1,0,0};
vector<bool> pattern_food = {0,0,1,0};
vector<bool> pattern_barrier = {0,0,0,1};;

void init_world()
{
	for(int i = 0; i<WORLD_Y; ++i)
	{
		for(int x = 0; x<WORLD_X; ++x)
		{
			world[i][x] = ' ';
			if(i == 0 || i == WORLD_Y-1 || x == 0 || x == WORLD_X-1)
			world[i][x] = '#';
		}
	}
}




void spawnItems()
{
	for(int i = 0; i<WORLD_Y; ++i)
	{
		for(int x = 0; x<WORLD_X; ++x)
		{
			if(world[i][x] == ' ')
			{
				switch(rand()%1000)
				{
					case 1:
						world[i][x] = '*';
					break;
					
					case 2:
						world[i][x] = '.';
					break;
				}
			}
		}
	}
}



//a brain WITH LEGS
struct creature{

	int points = 0;

	network *brain;
	int posx = 0;
	int posy = 0;

	void init_creature(bool init_brain);
	
	float ACTIVATION_CHANCE(float sum, float threshold);

	void temporal_cycle();
	void activation_cycle();
	void plasticity_cycle();
	void summation_cycle();

	void getBrainInputs();
	void cycle();
	
	float SUM_MIN = -10.0f; //the minimum sum possibly attainable
	float SUM_MAX = 10.0f;//the maximum sum possibly attainable
	
	float nextOperantCond = 0.0f;
	//This is the operant conditioning modifier. When it takes a nonzero value when
	//the organisms goes through the plasticity cycle, operant conditioning via neuromodulation
	//occurs. A negative value signifies a penalty, while a positive value signifies a reward.
	//The magnitude is the relative strength of the feedback.
	
	void reset_firing_juice();
	
};

void creature::reset_firing_juice()
{
	for(int i = 0; i<brain->all_neurons.size(); ++i)
	{
		brain->all_neurons[i]->firing_juice = 100.0f;
	}	
} 

float mean_firing_juice(const creature* b)
{
	int count = 0;
	int total = 0.0f;
	for(int i = 0; i<b->brain->all_neurons.size(); ++i)
	{
		if(b->brain->all_neurons[i]->type == NORMAL)
		{
			++count;
			total += b->brain->all_neurons[i]->firing_juice;
		}
	}
	return total/count;
}

bool precedenceBWL(const creature* a, const creature* b)
{
	if(a->points > b->points)
	return true;
	
	return false;
}


void creature::getBrainInputs()
{
	vector<bool> inputs;
	vector<char> sights;
	
	sights.push_back(world[this->posy-1][this->posx]);
	sights.push_back(world[this->posy][this->posx+1]);
	sights.push_back(world[this->posy+1][this->posx]);
	sights.push_back(world[this->posy][this->posx-1]);
	
	for(int i = 0; i<4; ++i)
	{
		switch(sights[i])
		{
			case ' ':
				inputs.insert(inputs.end(),pattern_space.begin(),pattern_space.end());
			break;
			
			case '.':
				inputs.insert(inputs.end(),pattern_food.begin(),pattern_food.end());
			break;
			
			case '*':
				inputs.insert(inputs.end(),pattern_poison.begin(),pattern_poison.end());
			break;
			
			case '#':
				inputs.insert(inputs.end(),pattern_barrier.begin(),pattern_barrier.end());
			break;
			
		}
	}
	
	insertInputVals(this->brain,inputs);
}

void creature::cycle()
{
	this->getBrainInputs();

	this->summation_cycle();
	
	this->activation_cycle();
	world[this->posy][this->posx] = ' ';
	if(this->brain->output_neurons[0]->activated)
	{
		if(VALIDPOS(this->posy-1,this->posx))
		{
			--this->posy;
			points += REWARD_EXERCISE;
			
		}else{
			points -= MOVE_INTO_WALL_PENALTY;
		}
	}
	if(this->brain->output_neurons[1]->activated)
	{
		if(VALIDPOS(this->posy,this->posx+1))
		{
			++this->posx;
		}else{
			points -= MOVE_INTO_WALL_PENALTY;
		}
			
	}
	if(this->brain->output_neurons[2]->activated)
	{
		if(VALIDPOS(this->posy+1,this->posx))
		{
			points += REWARD_EXERCISE;
			++this->posy;
		}else{
			points -= MOVE_INTO_WALL_PENALTY;
		}
			
	}
	if(this->brain->output_neurons[3]->activated)
	{
		if(VALIDPOS(this->posy,this->posx-1))
		{
			points += REWARD_EXERCISE;
			--this->posx;	
		}else{
			points -= MOVE_INTO_WALL_PENALTY;
		}
	}
	
	if(world[this->posy][this->posx] == '.')
	{
		++foodEaten;
		++trialFoodEaten;
		this->points += FOOD_REWARD;
		nextOperantCond += food_reward_OC;
	}else if(world[this->posy][this->posx] == '*')
	{
		++poisonEaten;
		++trialPoisonEaten;
		this->points -= POISON_PENALTY;
		nextOperantCond -= poison_penalty_OC;
	}
	
	world[this->posy][this->posx] = '@';
	
	this->temporal_cycle();
	this->plasticity_cycle();

}

//returns a float value between 0 and 1 that gives the probability that the neuron wil activate.
float creature::ACTIVATION_CHANCE(float sum, float threshold)
{
	//I forgot to account for the cases when the sum is negative...
	if(sum < 0.0f)
	return 0.0f;
	
	assert((float)threshold != 0.0f);
//	return (sum>threshold) ? 1.0f : pow(sum/(float)threshold,this->eP_FIRE_EXP);

	return (float)(1.0f - pow(fabs(sum-threshold)/(float)threshold,3.0));
	
}

/*
initializes the creature/organism

*/
void creature::init_creature(bool init_brain)
{
	this->posx = rand()%(WORLD_X-2)+1;
	this->posy = rand()%(WORLD_Y-2)+1;
	
	if(init_brain)
	{
		this->brain = generate_network_circular(80,16,4,5,0.8f);
	}
}
	

//update the "temporal" attributes of neurons, such as resting_timer
void creature::temporal_cycle()
{
	if(!this->brain)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return;
	}
	neuron* N = NULL;
	for(int i = 0; i<this->brain->all_neurons.size(); ++i)
	{
		
		N = this->brain->all_neurons[i];
		N->resting_timer += TIME_STEP;//plasticity-related purposes
		
		N->sum *= SUM_DECAY_RATE;
		
		
		
		//refill some of the firing juice
		N->firing_juice += juice_increment;
		if(N->firing_juice > 100.0f)
		N->firing_juice = 100.0f;

		//decay currentPassed and STC
		for(int x = 0; x<brain->all_neurons[i]->outbound.size(); ++x)
		{
			brain->all_neurons[i]->outbound[x]->currentPassed *= SUM_DECAY_RATE;
			brain->all_neurons[i]->outbound[x]->Dstr_stc_factor(STC_DECAY_RATE);
		}
			
		//increment the juice of each connection by some amount
		for(int x = 0; x<this->brain->all_neurons[i]->outbound.size(); ++x)
		{
			
			if(this->brain->all_neurons[i]->outbound[x]->juice_level <= 100.0f - juice_increment)
			this->brain->all_neurons[i]->outbound[x]->juice_level += juice_increment;
		}
		
		//calculate the baseline rate
		assert(N->baseline_cache.size() > 0);
		N->baselineRate = (N->baseline_cache[0] - N->baseline_cache[N->baseline_cache.size()-1])/(float)N->baseline_cache.size();
		//calculuate the recent rate
		N->recentRate = (N->recent_cache[0] - N->recent_cache[N->recent_cache.size()-1])/(float)N->recent_cache.size();
	}
}

void creature::activation_cycle()
{
	if(!this->brain)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return;
	}
//	logNeurons(this->brain);
//	exit(0);
	float threshold;
	for(int i = 0; i<this->brain->all_neurons.size(); ++i)
	{
		threshold = eA_THRESHOLD;
		if(brain->all_neurons[i]->type == 2)
		{
			threshold = eMOTOR_THRESHOLD;
		}
		
		if(this->ACTIVATION_CHANCE(this->brain->all_neurons[i]->sum,threshold) > RF01())
		{
			this->brain->all_neurons[i]->activated = true;
			brain->all_neurons[i]->hasActivated = true;
			
			this->brain->all_neurons[i]->sum = 0.0f;
			
			//decrease firing juice
			brain->all_neurons[i]->firing_juice -= juice_decrement;
		}
	}
}


#define FREEZE_TL 70000000

void creature::plasticity_cycle()
{
	if(timesteps > FREEZE_TL || LOCK_PLASTICITY)
	{
		return;
	}
	
	
	//Check scores
	for(int i = 0; i<brain->all_neurons.size(); ++i)
	{
		for(int x = 0; x<brain->all_neurons[i]->inbound.size(); ++x)
		{
			connection *c = brain->all_neurons[i]->inbound[x]; //FOR CLEANER CODE
			
			if(c->inhibitory_support_score > 0.0f && c->excitatory_support_score > 0.0f)
			{
				if(c->excitatory_support_score - c->inhibitory_support_score > DEBATE_DIFFERENCE_WIN_THRESHOLD)
				{//the excitatory side wins the debate 
				
				//	FILE *f = fopen("perms.txt","a+");
				//	fprintf(f,"Exceeded for excitatory, %f %d %f - %f\n",c->excitatory_support_score - c->inhibitory_support_score,timesteps,c->excitatory_support_score,c->inhibitory_support_score);
				//	fclose(f);
					c->str_lts += c->str_stc * 0.2;
					c->str_aggregate =- c->str_stc * 0.8;
					c->str_stc = 0.0f;
					c->excitatory_support_score = 0.0f;
					c->inhibitory_support_score = 0.0f;
					
				}else if(c->inhibitory_support_score - c->excitatory_support_score > DEBATE_DIFFERENCE_WIN_THRESHOLD)
				{//the inhibitory side wins the debate
				exit(12120);
				//	FILE *f = fopen("perms.txt","a+");
				//	fprintf(f,"Exceeded for inhibitory,%f,  %d, %f/%f\n",c->excitatory_support_score-c->inhibitory_support_score,timesteps,c->excitatory_support_score,c->inhibitory_support_score);
				//	fclose(f);
					c->str_lts += c->str_stc * 0.2;
					c->str_aggregate =- c->str_stc * 0.8;
					c->str_stc = 0.0f;
					c->excitatory_support_score = 0.0f;
					c->inhibitory_support_score = 0.0f;
					
				}else{
					
				}
			}
			
			
		}
	}
		
	//Feedback: PENALTY
	if(nextOperantCond < 0.0f)
	{
		//go through each neuron
		for(int i = 0; i<brain->all_neurons.size(); ++i)
		{
			
			if(brain->all_neurons[i]->hasActivated)
			{
				//if the neuron has activated, there is a significant chance that it is part of an errant
				//chain of neurons.
				
				brain->all_neurons[i]->hasActivated = false;				
			}
			
			reset_firing_juice();
			nextOperantCond = 0.0f;	
			
			for(int i = 0; i<brain->all_neurons.size(); ++i)
			{
				brain->all_neurons[i]->resetInboundCP();
			}
			
		}
	}else if(nextOperantCond > 0.0f)
	{
	//go through each neuron
		for(int i = 0; i<brain->all_neurons.size(); ++i)
		{
			//calculate the fault percentage of each excitaory outbound neuron
			float totalExcitatoryCP = 0.0f;
			float responsibility[brain->all_neurons[i]->inbound.size()];
			
			for(int x = 0; x<brain->all_neurons[i]->inbound.size(); ++x)
			{
				responsibility[x] = -1.0f;
				if(brain->all_neurons[i]->inbound[x]->currentPassed > 0.0f)
				totalExcitatoryCP += brain->all_neurons[i]->inbound[x]->currentPassed;
			}
			
			if(totalExcitatoryCP == 0.0f)
			break;
			
			for(int x = 0; x<brain->all_neurons[i]->inbound.size(); ++x)
			{
				responsibility[x] = brain->all_neurons[i]->inbound[x]->currentPassed/totalExcitatoryCP;
			}
			
			for(int x = 0; x<brain->all_neurons[i]->inbound.size(); ++x)
			{
				if(responsibility[x] > 0.0f)
				{
						brain->all_neurons[i]->inbound[x]->Dsts_HOMEOSTASIS_addsub(nextOperantCond * responsibility[x]);
				
						//This fabs() is not actually required, but I'm gonna put it in just to emphasize the fact that
						//both of the support scores have to be positive.
						brain->all_neurons[i]->inbound[x]->excitatory_support_score += FLOAT_ABS(nextOperantCond * responsibility[x]);
					
					//	FILE *f = fopen("added.txt","a+");
					//	fprintf(f,"added %f = %f * %f, it is now %f\n",FLOAT_ABS(nextOperantCond * responsibility[x]),nextOperantCond , responsibility[x],brain->all_neurons[i]->inbound[x]->excitatory_support_score);
					//	fclose(f);
						
				}
				
			}
					
		}
		reset_firing_juice();
		nextOperantCond = 0.0f;	
	
		for(int i = 0; i<brain->all_neurons.size(); ++i)
		{
			brain->all_neurons[i]->resetInboundCP();
		}
	}
	
	
	
}



void creature::summation_cycle()
{
	if(!this->brain)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return;
	}

	neuron* N = NULL;
	for(int i = 0; i<brain->all_neurons.size(); ++i)
	{
		N = brain->all_neurons[i];
		if(N->activated)
		{
			//propagate signal
			for(int x = 0; x<N->outbound.size(); ++x)
			{
				//ensure that after summing the new signal, the postsynaptic neuron's sum does not exceed
				//the maximum allowed sum or goes below the minimum allowed sum.
				if(N->outbound[x]->postsynaptic->sum + (N->outbound[x]->str_aggregate) >= SUM_MIN
				 && (N->outbound[x]->postsynaptic->sum + N->outbound[x]->str_aggregate) <= SUM_MAX
				)
				{
					N->outbound[x]->postsynaptic->sum += (N->outbound[x]->str_aggregate);
					
					//change currentPassed for the synapse
					N->outbound[x]->currentPassed += (N->outbound[x]->str_aggregate);
				
					//decrease the juice by some amount
					N->outbound[x]->juice_level -= juice_decrement;
				}
				
			}
			
			N->activated = false;
			
		
		}
	}
	
}


vector<creature*> census;

#define BWL_NUM 10
#define TRIAL_CYCLES 400000


void init_bwlorgs()
{
	for(int i = 0; i<BWL_NUM; ++i)
	{
		creature* b = new creature;
		b->init_creature(true);
		census.push_back(b);
	}
}

void cycle_bwlorgs()
{
	for(int i = 0; i<census.size(); ++i)
	{
		census[i]->cycle();
	}
}


#endif
