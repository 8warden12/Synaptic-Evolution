/*
TODO:
-implement long-lasting synaptic changes due to sensitization/habituation. Refer to pp191-192 of Kandel's book

-implement plastic decrements and increments of juice

-implement graded action potentials for sensory inputs. In other words, modify 
sensory neurons so that they don't fire every single timestep; they should fire 
more rapidly if a potent stimulus is applied to the organism (make the activation
probabilistic?).

-Vary the amount of operant conditioning signals that are sent to neurons in response to how potent
the feedback is 

-When change strengths due to operant conditioning, also look at the presynaptic neuron and see if
it has fired significantly. If it has not, don't apply any plasticity(?)

-Mechanism for changing signs of connections
*/
#include "curses.h"
#undef getch

#include "THE_MATRIX.C"
#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <conio.h>
#include <string>
#include <sstream>
#include <unistd.h>

void print_world()
{
	for(int i = 0; i<WORLD_Y; ++i)
	{
		for(int x = 0; x<WORLD_X; ++x)
		{
			printw("%c",world[i][x]);
		}
		printw("\n");
	}
	printw("timesteps: %d\nTotal food eaten: %d\nTotal posion eaten: %d\nRatio of food eaten to poison eaten: %f\n",timesteps,foodEaten,poisonEaten,(float)foodEaten/poisonEaten);
}






#define DEBUG_NOPRINT 0

// takes in a creature, which the function extracts parameters from
void trial(const creature* params)
{
	init_bwlorgs();
	
	for(int i = 0; i<DEBUG_NOPRINT; ++i)
	{
		spawnItems();
		cycle_bwlorgs();
		++timesteps;
//		getch();
	}
	
	for(int i = 0; i<TRIAL_CYCLES; ++i)
	{
		usleep(10000);
		print_world();
		refresh();
		clear();		
		spawnItems();
		cycle_bwlorgs();
		++timesteps;
	}
	
}



int k = 12;

void init_simulation()
{
	initscr();
}

int main()
{
	//1597596106; bad one
	getch(); //prompt keystroke before starting simulation
	int seed = time(0);//1597437118;
	srand(seed);
	FILE *f = fopen("seed.txt","a+");
	fprintf(f,"%d\n",seed);
	fclose(f);
	
	init_world();
	init_simulation();
	rand();
	
	
	creature* god_daddy = new creature;
	god_daddy->init_creature(true);
	
	creature* son = new creature;
	
	trial(son);
	
	return 0;
}
