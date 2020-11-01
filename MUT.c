//MUT (mutation).c: For dealing with mutating networks

#ifndef MUT_C
#define MUT_C

#include "NC2.C"
#include <assert.h>
#include <vector>
#include <stdlib.h>

void mutate_network(
	network* net,			//the network to be mutated
	float synapse_mut_pc,	//the percentage of synapses that may be removed
	float neuron_mut_pc,		//the percentage of neurons that may be removed
	float new_neuron_connection_density //for new neurons that are made, what should their connection density be?
)
{
	
	assert(synapse_mut_pc < 1.0f);
	assert(neuron_mut_pc < 1.0f);
	
	
	//initialize a vector of all connections
	vector<connection*> all_connections;
	for(int i = 0; i<net->all_neurons.size(); ++i)
	{
		for(int x = 0; x<net->all_neurons[i]->outbound.size(); ++x)
		{
			all_connections.push_back(net->all_neurons[i]->outbound[x]);
		}
	}
	
	
	//calculate constants	
	const int mut_synaptic_num = all_connections.size() * synapse_mut_pc;
	const int mut_neuron_num = net->all_neurons.size() * neuron_mut_pc;
	const int new_neuron_total_connections = net->all_neurons.size() * new_neuron_connection_density; 
	//the total number of connections (both inbound and outbound) that a new neuron will have

	//neuron insertion
	
	for(int i = 0; i<mut_neuron_num; ++i)
	{
		neuron* n = newNeuron();
		n->type = NORMAL;
		//form the inbound connections
		for(int x = 0; x<new_neuron_total_connections/2; ++x)
		{
			int PR_index = rand()%net->all_neurons.size();
			if(!connectionExists(net->all_neurons[PR_index],n))
			{
				newConnection(net->all_neurons[PR_index],n,0.01,0.0);
			}
		}
		
		//form the outbound connections
		for(int x = 0; x<new_neuron_total_connections/2; ++x)
		{
			int PO_index = rand()%net->all_neurons.size(); 
			if(!connectionExists(n,net->all_neurons[PO_index]))
			{
				newConnection(n,net->all_neurons[PO_index],0.01,0.0);
			}
		}		
		
		net->all_neurons.push_back(n);
	}
	
	//neuron deletion
	/*
		Select non-input, non-output neurons to delete at random
	*/
	
	for(int i = 0; i<mut_neuron_num; ++i)
	{
		redo:
		int cullIndex = rand()%net->all_neurons.size();
		if(net->all_neurons[cullIndex]->type == NORMAL)
		{
			for(int x = 0; x<net->all_neurons[cullIndex]->outbound.size(); ++x)
			{
					
			}
			
			
			delete_neuron(net->all_neurons[cullIndex]);		
			//gc
			neuron* n = net->all_neurons[cullIndex];
			net->all_neurons.erase(net->all_neurons.begin()+cullIndex);
			
			delete n;
			n = NULL;
			
		}else{
			goto redo;
		}
	}	

	//synapse insertion
	
	/*
		Attempt to link random pairs of neurons. Will fail upon coming across a pair of neurons
		that are already connected IN THE SAME DIRECTION
	*/
	
	for(int i = 0; i<mut_synaptic_num; ++i)
	{
		int pre_index = rand()%net->all_neurons.size();
		int post_index = rand()%net->all_neurons.size();
		
		if(!connectionExists(net->all_neurons[pre_index],net->all_neurons[post_index]) && pre_index != post_index)
		{
			newConnection(net->all_neurons[pre_index],net->all_neurons[post_index],0.01,0.0);
			//Do not insert newly-anointed connections to all_connections(?)
		}
				
	}
		
	//synapse deletion
	
	/*DEPRECATED: Put all synapses into a vector, and purge them randomly. I cannot just first choose a random
	  neuron, then choose a random outbound synapse for that neuron, because if a neuron only has 
	  one outbound connection, that connection will have a disproportionate higher chance of being
	  purged (1/n, where n is the number of neurons)
	*/	
	
	
	/*
		NEW: Traverse through each outbound synapse in the network (i.e. all of them, only once)
		and roll the dice to see whether it will be cleaved or not.
	*/
	for(int i = 0; i<net->all_neurons.size(); ++i)
	{
		for(int x = 0; x<net->all_neurons[i]->outbound.size(); ++x)
		{
			
		}
	}
	
	assert(all_connections.size() > 0);
	for(int i = 0; i<mut_synaptic_num; ++i)
	{
		//get a random connection index to be removed
		int cullIndex = rand()%all_connections.size();
		connection* c = all_connections[cullIndex];
		deleteConnection(c->presynaptic,c->postsynaptic);
		all_connections.erase(all_connections.begin() + cullIndex);
	}
	
	
	
}



#endif
