//NW_T.C
//NETWORK TOPOLOGY FUNCTIONS
//CREATING CERTAIN NETWORK TOPOLOGIES

#include <random>
#include <math.h>
#include <assert.h>
#include "NC2.c"
#include "AFMATH.C"
using namespace std;

#ifndef NW_T_C
#define NW_T_C

//removes all outbound connections from neurons in source to neurons in dest
void cleave_efferents(vector<neuron*> source, vector<neuron*> dest)
{
	
	for(int i = 0; i<dest.size(); ++i)
	{
		dest[i]->bitflags = 0;
		SETBIT(dest[i]->bitflags,2);
	}
	
	for(int i = 0; i<source.size(); ++i)
	{
		for(int x = 0; x<source[i]->outbound.size(); ++x)
		{
			if(TESTBIT(source[i]->outbound[x]->postsynaptic->bitflags,2))
			{
				printf("deleting some connection\n");
				deleteConnection(source[i],source[i]->outbound[x]->postsynaptic);
			}else{
			//	printf("1");
			}
		}
	}

	for(int i = 0; i<dest.size(); ++i)
	{
		dest[i]->bitflags = 0;
	}
	
}

/*
This function takes in the distance between two regions and returns a probability, in the form
of "1 in x chance", that two neurons in those two zones will be connected.

int distance 
float base_probability - the probability of a connection occuring for a neuron within the same subdivision
float decrease_factor - how drastically the probability drops with an increase in sbd difference. Higher = more drastic


*/
float generate_circular_connection_chance(int distance,float base_probability,float decrease_factor)
{
//	printf("%d\n",distance);
	if(distance >= 0)
	{
		return (base_probability)/(pow(distance,decrease_factor)+1.0f);	
	}else if(distance < 0){
		return (base_probability/2.0f)/(pow(-distance,decrease_factor*2.0f)+ 1.0f);
	}	
}

/*
Creates a circular neural network
int neurons - how many neurons
int subdivisions - how many "small words" there will be. Note: must divide neurons

TODO:

DONE:
-make connections go primarily in one direction
-add a parameter that specifies density
-integrate input neurons
-integrate output neurons
*/
network* generate_network_circular(int neurons, int input_neurons, int output_neurons, int subdivisions, float base_connection_chance)
{
	//make sure parameters are valid
	assert(subdivisions > 0);
	assert(neurons % subdivisions == 0);
	
	//make sure input and output zones fit into a subdivision
	assert(input_neurons <= neurons/subdivisions);
	assert(output_neurons <= neurons/subdivisions);
		
	network* net = new network;
	int index[neurons]; //given a neuron's index, this array will hold the subdivision that it belongs to
	
	//Each neuron has its "subdivision". This is meant to create small-world connectivity
	for(int i = 0; i<neurons; ++i)
	{
		index[i] = (int)floor((float)i/(neurons/subdivisions));
	}
	
	//create neurons
	for(int i = 0; i<neurons; ++i)
	{
		net->all_neurons.push_back(newNeuron());
		net->all_neurons[i]->neuronID = i;
		net->all_neurons[i]->type = NORMAL;
	}
	
	//for terseness
	int sbd = subdivisions;
	
	//creating connections will run in O(n^2) time for now
	for(int i = 0; i<net->all_neurons.size(); ++i)
	{
		//DONE: let it be so that circular for loops wrap around and remain valid
	
		//the difference between the SBDs of the post- and pre-synaptic neurons
		//if negative, then it is "backward" (going against the overall loop direction)
		int current_sbd_difference = -2; 
		
		//counting purposes
		int neuron_sbd_counter = 0;
		
		//   starting index is two subdivisions back 
		for(int outboundIndex = (i - 2*neurons/sbd); outboundIndex <= (i + 2*neurons/sbd); ++outboundIndex)
		{
			int actualIndex = (outboundIndex + neurons) % neurons; //make the range be in [0,neurons)
			
			//index[actualIndex] = index[i] is a positive integer that equals the difference in subdivisions between the neurons
			if(rand() % (int)floor((10.0f/generate_circular_connection_chance(current_sbd_difference, base_connection_chance, 1.2))) < 10)
			{
				//no need to check if a connection of the same direction already exists 
				//because each ordered pair of neurons will only be checked at most once
				if(actualIndex != i) //make sure there are no self loops
				newConnection(net->all_neurons[i],net->all_neurons[actualIndex],0.01,0.0);
			}
			
			
			if(++neuron_sbd_counter == neurons/sbd)
			{
				++current_sbd_difference;
				neuron_sbd_counter = 0;
			}	
		}
		
	}
	
	//input neurons will be in the 0th divison, output neurons will be in the (n-1)th division
	
	//begin counting input neurons at index 0
	for(int i = 0; i<input_neurons; ++i)
	{
		net->input_neurons.push_back(net->all_neurons[i]);
		net->all_neurons[i]->type = INPUT;
	}
	
	//begin counting output neurons (backwards) at index n-1
	for(int i = 0; i<output_neurons; ++i)
	{
		net->output_neurons.push_back(net->all_neurons[net->all_neurons.size()-i-1]);	
		net->all_neurons[i]->type = OUTPUT;
	}
	
	cleave_efferents(net->input_neurons,net->output_neurons);
	
	return net;
}

#endif
