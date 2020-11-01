#ifndef NC2_C
#define NC2_C
int timesteps = 0;
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <deque>
#include <cassert>
#include "SIMULATOR_CONSTANTS.C"
#include "curses.h"
#include <stdint.h>
//#include "CONSTANT.C"
using namespace std;





//threshold will vary later on


struct neuron;



struct connection{//connections between neurons
	
	float excitatory_support_score = 0.0f;
	float inhibitory_support_score = 0.0f;
	//Used for the DEBATE. Both are positive
	
	/*
	The new synaptic strength system (2020-07-29)
	Synaptic strength is divided into two components:
	Long-term strength (str_lts), which is the baseline strength, and
	Short-term change, (str_stc), which is a change in the short term
	of the long-term strength. When added together, str_lts and str_stc become
	str_agg, which is the aggregate strength.
	*/
	float str_lts = 0.0f;
	float str_stc = 0.0f;
	float str_aggregate = 0.0f;
	
	float stablization_confidence = 0.0f;
	//This value tracks how confident we are that the current short-term strength
	//change should be made semi-permanent. The higher this value, the more
	//confident we are. 
	
	//float lts = 0.0f;
	//this is the baseline strength
	
	//float sts_HEBB = 0.0f;
	//the synaptic strength associated with hebbian plasticity
	
	//float sts_aggregate = 0.0f;
	//the sum of all the sts es. This is the strength used in calculations.
	
	//vintage.. but this is now used
	float currentPassed = 0.0f;
	
	
	neuron* presynaptic;
	neuron* postsynaptic;
	
	//like a "mailbox" that keeps track of incoming SE signals
	//positive signals means excitatory strengthen, negative signal means inhibitory strengthen,/
	//zero means weaken
	vector<float> SE_signal_mailbox;
	bool feedback_received = false;
	
	
	float juice_level = 100.0f;
	//Juice is a new concept introduced on 2020-07-08 in order to facilitate differentiation between
	//habituation and sensitization. Every time a signal passes through this connection, this
	//variable decreases by a fixed amount. Over time, this variable will also regenerate. However,
	//it will never exceed 100.0f. When the connection's postsynaptic neuron fires, if the connection
	//is excitatory, then the program will look at the juice level. If it is below some threshold, 
	//the synaptic strength will temporarily increase. Otherwise, the strength will temporarily
	//decrease. Over many firings of the postsynaptic neuron, habituation or sensitization may occur.
	//TODO: figure out what to do if the connection is inhibitory (if anything)


	//void Dsts_HEBB_addsub(float delta);
	//void Dsts_OPCON_addsub(float delta);

	void Dstr_stc_addsub(float delta);
	void Dstr_stc_factor(float factor_delta);

	void Dsts_HOMEOSTASIS_addsub(float delta);
	
	
	void make_stc_longterm();
	
	
	
	//UNUSED
	int learning_feedback_change_counter = 0;
	//This variable will tell you if there have been three learning feedback
	//synaptic changes in a row that are of the same alighment. 
	//+ means went towards excitatory, - means went towards inhibitory
	
	
};

void connection::make_stc_longterm()
{
	
}

//TODO: weaken or strengthen neighbouring synapses accordingly
void connection::Dstr_stc_addsub(float delta)
{
	//Ensure that the connection will not change polarity. Before and after the change, the sign should be the same.
	
	if(str_aggregate + delta < weight_max && str_aggregate + delta > weight_min)
	{	
		str_stc += delta;
		str_aggregate += delta;
	}
}

void connection::Dstr_stc_factor(float factor_delta)
{
	float total_change = (str_stc * factor_delta) - str_stc; //How much the aggregate strength will change by
	if(str_aggregate + total_change < weight_max && str_aggregate + total_change > weight_min)
	{
		str_stc *= factor_delta;
		str_aggregate += total_change;
	}
	
}

enum NEURON_TYPE {
	NORMAL = 0,
	INPUT = 1,
	OUTPUT = 2,
	PATTERN_GENERATOR = 3,
};



//unused
//amount that the real AFR is allowed to stray from the universal AFR. Also as a percent of CACHE_LENGTH
//#define STRAY_LIMIT 0.2f


struct neuron{// neurons
	vector<connection*> inbound;//connections coming into the neuron
	vector<connection*> outbound;//connections sent out by the neuron
	
	int neuronID = 0;//ID = index
	
	float sum = 0.0f;//Sum of the signals coming into the neuron, neuron will activate if this breaches the threshhold
	float resting_timer = 0.0f;//how long the neuron has been at rest (for calculating plasticity)
	
	bool activated = false; //whether the neuron is activated or not
	
	bool hasActivated = false; //whether the neuron has activated since the last feedback
	
	NEURON_TYPE type = NORMAL;
	
	deque<int> recent_cache;//the record in the nth index occured n steps ago
	deque<int> baseline_cache; //the LONG cache, to be used as a baseline

	float baselineRate = 0.0f;
	float recentRate = 0.0f;
	
	
	//used for suppression/encouragement
	bool SEchanged = false;
	
	
	int SE_phaseTimer = 0;
	
	
	float firing_juice = 100.0f;
	//This is similar to the juice variable used for connections. Every time the neuron fires,
	//the juice goes down by some value. It also replenishes constantly by a constant value.
	//To determine the rapidness of fire of a neuron, look at how depleted the firing juice is.
	//Smaller values represent more firings.
	
	uint32_t bitflags;
	//used for efficient flagging
	
	void resetInboundCP();
};

bool connectionExists(const neuron* presynaptic, const neuron* postsynaptic);

void neuron::resetInboundCP()
{
	for(int x = 0; x<inbound.size(); ++x)
	{
		inbound[x]->currentPassed = 0.0f;
	}
}

neuron* newNeuron()
{
	neuron *n = new neuron;
	n->recent_cache.push_back(0);
	n->baseline_cache.push_back(0);
	//fill activation_cache with CACHE_LENGTH 0s
	//n->activation_cache.insert(n->activation_cache.begin(),CACHE_LENGTH,0);
	
	return n;
}

connection* newConnection(neuron* presynaptic, neuron* postsynaptic, float str_lts, float str_stc)// create a new connection between 2 neurons (referenced by their IDs)
{
	assert(presynaptic != postsynaptic);
	assert(!connectionExists(presynaptic,postsynaptic));
	
	connection *c = new connection;
	c->presynaptic = presynaptic;
	c->postsynaptic = postsynaptic;
	c->str_lts = str_lts;
	c->str_stc = str_stc;
	c->str_aggregate = c->str_lts + c->str_stc;
	//c->recalcitrance = recalcitrance;
	presynaptic->outbound.push_back(c); 
	postsynaptic->inbound.push_back(c);
	
	return c;
}

//This is a more "high-level" version of Dsts_HEBB_addsub: when this function modifies strengths, it will
//induce heterosynaptic plasticity, which will modify neighbouring synapses (inputs) to a neuron
//as well.
void connection::Dsts_HOMEOSTASIS_addsub(float delta)
{
	Dstr_stc_addsub(delta);
	//Induce heterosynaptic plasticity on the other input synapses of the postsynaptic neuron

	
	neuron *n = this->postsynaptic;
	
	assert(n->inbound.size() != 1);
	
	float change =  -0.2 * delta/(float)(n->inbound.size() - 1);
	for(int i = 0; i<n->inbound.size(); ++i)
	{
		if(n->inbound[i] != this)
		{
			n->inbound[i]->Dstr_stc_addsub(change);
		}
	}
}

struct network{// brain networks
	vector<neuron*> input_neurons;//neurons that receive external input
	vector<neuron*> output_neurons;//neurons that directly affect ations made by the "organism"
		
	vector<neuron*> all_neurons;//all neurons in the network, regardless of role
	
};


int save_network(const network* n, const char* filename, bool append = false)
{
	if(!n)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return -1;
	}
	FILE *f = NULL;
	if(append)
	{
		f = fopen(filename,"a+");
	}else{
		f = fopen(filename,"w");
	}

	if(!f)
	{
		assert("Could not open file" == "lol");
		return -1;
	}
	fprintf(f,"Neurons: %d\n",n->all_neurons.size());
	
	neuron* N = NULL;
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		N = n->all_neurons[i];
		fprintf(f,"Index: %d Type: %d Num_IB: %d Num_OB: %d\n",
		i, //index
		N->type, //type
		N->inbound.size(), //number of inbound
		N->outbound.size()); //number of outbound
		for(int x = 0; x<N->inbound.size(); ++x)
		{
			fprintf(f,"SenderID: %d str_lts: %f str_stc: %f\n",
			N->inbound[x]->presynaptic->neuronID,
			N->inbound[x]->str_lts,
			N->inbound[x]->str_stc);
		}
		fputs("\n\n",f);
	}
	
	fclose(f);
	return 0;
}

network* load_network(const char* filename)
{
	FILE *f = fopen(filename,"r");
	network* net = new network;
	
	if(!f)
	{
		printf("ERROR");
		assert("Could not open file" == "lol");
		return NULL;
	}
	int neuronNum = 0;
	fscanf(f,"Neurons: %d\n",&neuronNum);
	
	neuron* N = NULL;

	for(int i = 0; i<neuronNum; ++i)
	{
		net->all_neurons.push_back(newNeuron());
		net->all_neurons[net->all_neurons.size()-1]->neuronID = i;
	}
	
	for(int i = 0; i<neuronNum; ++i)
	{
		int index = 0,
		    type = 0,
		    inboundNum = 0,
		    outboundNum = 0;
		    
		fscanf(f,"Index: %d Type: %d Num_IB: %d Num_OB: %d\n",
		&index,
		&type, //type
		&inboundNum, //number of inbound
		&outboundNum); //number of outbound
		net->all_neurons[i]->type = (NEURON_TYPE)type;
		if(type == INPUT)
		{
			net->input_neurons.push_back(net->all_neurons[index]);
		}else if(type == OUTPUT)
		{
			net->output_neurons.push_back(net->all_neurons[index]);
		}
		
		for(int x = 0; x<inboundNum; ++x)
		{
			int senderIndex = 0;
			float str_lts = 0,
			      str_stc = 0;
			    
			fscanf(f,"SenderID: %d str_lts: %f str_stc: %f\n",
			&senderIndex,
			&str_lts,
			&str_stc);
		//	printf("scanned %f\n",str_lts);
			newConnection(net->all_neurons[senderIndex],net->all_neurons[index],str_lts,str_stc);
		}
		fscanf(f,"\n\n");
		//fseek(f,2,SEEK_CUR);
	}
	
	fclose(f);
	return net;
}

network* loadNetwork(const char* filename)
{
	network* n = new network;
	FILE* f = fopen(filename,"r");
	if(!f)
	return NULL;
	
	int num_neurons = 0;
	int num_inbound = 0;
	int num_outbound = 0;
	int otherIndex = 0;
	
	float sts_hebb = 0.0f;
	float sts_se = 0.0f;
	
	float lts = 0.0f;
	
	fscanf(f,"Neurons: %d\n",&num_neurons);

	for(int i = 0; i<num_neurons; ++i)
		n->all_neurons.push_back(newNeuron());
	
	for(int i = 0; i<num_neurons; ++i)
	{	
		fscanf(f,"Index: %d Type: %d Num_IB: %d Num_OB: %d\n",&n->all_neurons[i]->neuronID,
		&n->all_neurons[i]->type,&num_inbound,&num_outbound);
		
		if(n->all_neurons[i]->neuronID == INPUT)
		{
			n->input_neurons.push_back(n->all_neurons[i]);
		}else if(n->all_neurons[i]->neuronID == OUTPUT)
		{
			n->output_neurons.push_back(n->all_neurons[i]);
		}
		
		for(int x = 0; x<num_inbound; ++x)
		{
			fscanf(f,"SenderID: %d sts_HEBB: %f sts_SE: %f LTS: %f\n",&otherIndex,&sts_hebb,&sts_se,&lts);
			newConnection(n->all_neurons[otherIndex],n->all_neurons[i],sts_hebb,lts);
		}
	}
	fclose(f);
	return n;
}




void display_minimalist(const network* n, bool using_curses)
{
	if(!n)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return;
	}
	
	if(using_curses)
	{
		for(int i = 0; i<n->all_neurons.size(); ++i)
		{
			if(n->all_neurons[i]->activated)
			{
				printw("1");
			}else{
				printw("0");
			}
		}
		printw("\n");
		refresh();
	}else{
		for(int i = 0; i<n->all_neurons.size(); ++i)
		{
			if(n->all_neurons[i]->activated)
			{
				printf("1");
			}else{
				printf("0");
			}
		}
		printf("\n");
	}
	
}

void firingSumDisplay(const network* n)
{
	if(!n)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return;
	}
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		printf("Neuron %d has sum %f and is activated? %d\n",i,n->all_neurons[i]->sum,n->all_neurons[i]->activated);
	}
}

void display_caches(const network* n)
{
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		printf("Neuron %d has cache size %d: ",i,n->all_neurons[i]->recent_cache.size());
		for(int x = 0; x<n->all_neurons[i]->recent_cache.size(); ++x)
		{
			printf("%d ",n->all_neurons[i]->recent_cache[x]);
		}
		printf("\n");
	}
}

void display_nstats(const network* n)
{
	if(!n)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return;
	}
	
	printf("%d %d %d\n",n->all_neurons.size(),n->input_neurons.size(),n->output_neurons.size());
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		printf("neuron %d has %d inbound and %d outbound connections."
		"sum = %f, TSA = %f\n",
		n->all_neurons[i]->neuronID,n->all_neurons[i]->inbound.size(),n->all_neurons[i]->outbound.size(),
		n->all_neurons[i]->sum,n->all_neurons[i]->resting_timer);

	}
}

void display_connections(const network* n)
{
	if(!n)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return;
	}
	
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		printf("Neuron %d has %d outbound and %d inbound connections: \n",n->all_neurons[i]->neuronID,n->all_neurons[i]->outbound.size(),n->all_neurons[i]->inbound.size());
		for(int x = 0; x<n->all_neurons[i]->outbound.size(); ++x)
		{
			printf("to neuron %d str_lts = %.4f, str_stc = %.4f\n",n->all_neurons[i]->outbound[x]->postsynaptic->neuronID,
			n->all_neurons[i]->outbound[x]->str_lts,n->all_neurons[i]->outbound[x]->str_stc
			);
		}

		for(int x = 0; x<n->all_neurons[i]->inbound.size(); ++x)
		{
			printf("from neuron %d str_lts = %.4f, str_stc = %.4f\n",n->all_neurons[i]->inbound[x]->presynaptic->neuronID,
			n->all_neurons[i]->inbound[x]->str_lts,n->all_neurons[i]->inbound[x]->str_stc
			);
		}
		putchar('\n');
	}
}

bool connectionExists(const neuron* presynaptic,const neuron* postsynaptic)//for checking if a connection already exists
{
	for(int i = 0; i<presynaptic->outbound.size(); ++i)
	{
		if(presynaptic->outbound[i]->postsynaptic == postsynaptic)
		return true;
	}
	
	return false;
}

void deleteConnection(neuron* presynaptic, neuron* postsynaptic)
{
	
	connection *c = NULL;
	for(int i = 0; i<presynaptic->outbound.size(); ++i)
	{
		if(presynaptic->outbound[i]->postsynaptic == postsynaptic)
		{
			c = presynaptic->outbound[i];
			presynaptic->outbound.erase(presynaptic->outbound.begin()+i);
			break;
		}
	}
	
	for(int i = 0; i<postsynaptic->inbound.size(); ++i)
	{
		if(postsynaptic->inbound[i]->presynaptic == presynaptic)
		{
			postsynaptic->inbound.erase(postsynaptic->inbound.begin()+i);
			break;
		}
	}
	
	delete c;
	c = NULL;
	
	
}

void delete_neuron(neuron* n)
{
	int num_o = n->outbound.size();
	int num_i = n->inbound.size();
	
	for(int i = 0; i<n->outbound.size(); ++i)
	{
		deleteConnection(n,n->outbound[i]->postsynaptic);
		--i;
	}
	
	for(int i = 0; i<n->inbound.size(); ++i)
	{
		deleteConnection(n->inbound[i]->presynaptic,n);
		--i;
	}
}


network* newNetwork(int neurons, int input_neurons, int output_neurons, float density)//returns a pointer to the network
{
	network* n = new network;
	
	for(int i = 0; i<neurons; ++i)//put neurons in the network
	{
		n->all_neurons.push_back(newNeuron());
		n->all_neurons[i]->neuronID = i;
	}
	
	for(int i = 0; i<input_neurons; ++i)
	{
		n->input_neurons.push_back(n->all_neurons[i]);//the first neurons become input_neurons
		n->all_neurons[i]->type = INPUT;
	}
	
	for(int i = 0; i<output_neurons; ++i)
	{
		n->output_neurons.push_back(n->all_neurons[neurons-(i+1)]);//the last neurons become output_neurons
		n->all_neurons[neurons-(i+1)]->type = OUTPUT;
	}
	
	int r1 = 0;//ID of the other neuron
	
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		for(int x = 0; x<n->all_neurons.size()*density; ++x)
		{
			if(n->all_neurons.size() > 0)
			{
				r1 = rand()%n->all_neurons.size();
				if(i != r1 && !connectionExists(n->all_neurons[i],n->all_neurons[r1]))
				newConnection(n->all_neurons[i],n->all_neurons[r1],
				0.5f,//sts_HEBB
				0.5f//LTS
				);
			}
		}
	}
	
/*	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		set_init_connection_str(n->all_neurons[i]);
	}
	*/
	return n;

}

void insertInputVals(const network *n, vector<bool> inputVals)
{
	assert(n->input_neurons.size() >= inputVals.size());
	for(int i = 0; i<n->input_neurons.size(); ++i)//assigning activations from inputVals
	{
		n->input_neurons[i]->activated = inputVals[i];
	}
}

//copies networks. The topologyOnly flag states whether the weight strengths
//will be copied
network* networkcpy(const network *source, bool topologyOnly)
{
	network *n = new network;

	for(int i = 0; i<source->all_neurons.size(); ++i)
	{
	//	printf("%d ",source->all_neurons[i]->neuronID);
		n->all_neurons.push_back(newNeuron());
		n->all_neurons[i]->neuronID = i;
	}
	for(int i = 0; i<source->all_neurons.size(); ++i)
	{
		
		n->all_neurons[i]->type = source->all_neurons[i]->type;
		if(n->all_neurons[i]->type == INPUT)
			n->input_neurons.push_back(n->all_neurons[i]);
			
		if(n->all_neurons[i]->type == OUTPUT)
			n->output_neurons.push_back(n->all_neurons[i]);
		
		for(int x = 0; x<source->all_neurons[i]->outbound.size(); ++x)
		{
		//	assert(source->all_neurons[i]->outbound[x]->postsynaptic->neuronID < n->all_neurons.size());
		//	printf("X %d %d %d %d %d\n",x,source->all_neurons[i]->outbound.size(),source->all_neurons[i]->outbound[x]->sts,source->all_neurons[i]->outbound[x]->postsynaptic->neuronID,n->all_neurons.size());
			
			if(topologyOnly)
			{
				newConnection(n->all_neurons[i],n->all_neurons[source->all_neurons[i]->outbound[x]->postsynaptic->neuronID],
				0.256512f,
				0.256512f);
			}else{
				newConnection(n->all_neurons[i],n->all_neurons[source->all_neurons[i]->outbound[x]->postsynaptic->neuronID],
				source->all_neurons[i]->outbound[x]->str_lts,
				source->all_neurons[i]->outbound[x]->str_stc);
			}
		}
	}
	return n;
}



void delete_network(network *n)
{
	if(!n)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return;
	}
	
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		for(int x = 0; x<n->all_neurons[i]->outbound.size(); ++x)
		{
			delete n->all_neurons[i]->outbound[x];
		}
		delete n->all_neurons[i];
	}
	
	n = NULL;
}

void logNeurons(const network* n)
{
	FILE*f = fopen("log.txt","a+");
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		fprintf(f,"%0.3f ",n->all_neurons[i]->sum);
	}
	fprintf(f,"\n");
	fclose(f);
}


void logActivated(const network* n)
{
	FILE *f = fopen("log.txt","a+");
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		if(n->all_neurons[i]->activated)
		{
			fprintf(f,"1");
		}else{
			fprintf(f,"0");
		}
	}
	fprintf(f,"\n");
	fclose(f);
}

void logdump(const network* n, const char* file)
{
	FILE *f = fopen(file,"w");
	if(!n)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return;
	}
	
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		if(n->all_neurons[i]->activated)
		{
			fprintf(f,"1");
		}else{
			fprintf(f,"0");
		}
	}
	fprintf(f,"\n");

	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		fprintf(f,"Neuron %d has sum %f and is activated? %d\n",i,n->all_neurons[i]->sum,n->all_neurons[i]->activated);
	}

	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		fprintf(f,"Neuron %d has cache size %d: ",i,n->all_neurons[i]->recent_cache.size());
		for(int x = 0; x<n->all_neurons[i]->recent_cache.size(); ++x)
		{
			fprintf(f,"%d ",n->all_neurons[i]->recent_cache[x]);
		}
		fprintf(f,"\n");
	}

	fprintf(f,"%d %d %d\n",n->all_neurons.size(),n->input_neurons.size(),n->output_neurons.size());
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		fprintf(f,"neuron %d has %d inbound and %d outbound connections."
		"sum = %f, TSA = %f\n",
		n->all_neurons[i]->neuronID,n->all_neurons[i]->inbound.size(),n->all_neurons[i]->outbound.size(),
		n->all_neurons[i]->sum,n->all_neurons[i]->resting_timer);

	}

	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		fprintf(f,"Neuron %d has the following connections: \n",n->all_neurons[i]->neuronID);
		for(int x = 0; x<n->all_neurons[i]->outbound.size(); ++x)
		{
			fprintf(f,"to neuron %d str_lts = %f, str_stc = %f\n",n->all_neurons[i]->outbound[x]->postsynaptic->neuronID,
			n->all_neurons[i]->outbound[x]->str_lts,n->all_neurons[i]->outbound[x]->str_stc
			);
		}

		for(int x = 0; x<n->all_neurons[i]->inbound.size(); ++x)
		{
			fprintf(f,"from neuron %d str_lts = %f, str_stc = %f\n",n->all_neurons[i]->inbound[x]->presynaptic->neuronID,
			n->all_neurons[i]->inbound[x]->str_lts,n->all_neurons[i]->inbound[x]->str_stc
			);
		}
	}
	fprintf(f,"\n\n\n\n");
	fclose(f);
}

//saves every piece of data about a network to a file
void network_snapshot(const network* n, const char* filename)
{
	if(!n)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return;
	}
	FILE *f = NULL;
	f = fopen(filename,"w");

	if(!f)
	{
		return;
	}
	fprintf(f,"Neurons: %d\n",n->all_neurons.size());
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		fprintf(f,"Index: %d\nType: %d\nNum_IB: %d\nNum_OB: %d\n",i,n->all_neurons[i]->type,
		n->all_neurons[i]->inbound.size(),
		n->all_neurons[i]->outbound.size());
		fprintf(f,"Sum: %.2f\nactivated? %d\nbaseline rate: %.2f\nrecent rate: %.2f\nresting timer: %f\n",
			n->all_neurons[i]->sum,
			n->all_neurons[i]->activated,
			n->all_neurons[i]->baselineRate,
			n->all_neurons[i]->recentRate,
			n->all_neurons[i]->resting_timer
		);
		fprintf(f,"INBOUND CONNECTIONS:\n");
		for(int x = 0; x<n->all_neurons[i]->inbound.size(); ++x)
		{
			fprintf(f,"Sender ID: %d str_lts: %f str_stc: %.2f CurrentPassed: %.2f\n",n->all_neurons[i]->inbound[x]->presynaptic->neuronID,
			n->all_neurons[i]->inbound[x]->str_lts,
			n->all_neurons[i]->inbound[x]->str_stc,
			n->all_neurons[i]->inbound[x]->currentPassed);
		}
		fprintf(f,"OUTBOUND CONNECTIONS:\n");
		
		for(int x = 0; x<n->all_neurons[i]->outbound.size(); ++x)
		{
			fprintf(f,"Receiver ID: %d str_lts: %.2f str_stc: %.2f CurrentPassed: %.2f\n",n->all_neurons[i]->outbound[x]->postsynaptic->neuronID,
			n->all_neurons[i]->outbound[x]->str_lts,
			n->all_neurons[i]->outbound[x]->str_stc,
			n->all_neurons[i]->outbound[x]->currentPassed
			);
			
		}
		fputs("\n\n",f);
	}
	
	fclose(f);
}

#endif
