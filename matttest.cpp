#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <atomic>
#include <iostream>

#include "cubiomes/finders.h"
#include <math.h>
#include <string> 
#include <sstream>
#include <thread>

class Random2
{
private:
	uint64_t seed;

public:
	Random2(int64_t seed) {
		this->seed = (uint64_t)((uint64_t)seed ^ 0x5deece66du) & ((1LLu << 48) - 1);
	}

	inline int NextInt(int n)
	{
		//if (n <= 0) throw new ArgumentException("n must be positive");

		if ((n & -n) == n)  // i.e., n is a power of 2
			return (int)((n * (int64_t)Next(31)) >> 31u);

		long bits, val;
		do
		{
			bits = Next(31);
			val = bits % (int32_t)n;
		}
		while (bits - val + (n - 1) < 0);

		return (int)val;
	}

	inline double NextDouble()
	{
		return (((int64_t)Next(26) << 27u) + Next(27))
		  / (double)(1LLu << 53u);
	}

	inline int32_t Next(int bits)
	{
		seed = (seed * 0x5deece66du + 0xBu) & ((1LLu << 48u) - 1);

		return (int32_t)(seed >> (48u - bits));
	}
	
	inline int64_t nextLong()
	{
		return ((int64_t) Next(32) << 32) + Next(32);
	}
};

int64_t _getChunkSeed(int64_t seed, int x, int z) {
	int64_t lul = seed;
	setSeed(&lul);
	int64_t a = nextLong(&lul) / 2 * 2 + 1;
	int64_t b = nextLong(&lul) / 2 * 2 + 1;
	//long b = r->nextLong()/2*2+1;
	return ((x*a + z*b)^seed);// & ((1L << 48) - 1);
}

bool isSeed12Eye(int64_t seed, int range) {
	for(int x = -range; x <= range; x++) {
		for (int z = -range; z <= range; z++) {
			int64_t ChunkSeed = _getChunkSeed(seed, x, z);
			//printf("[%d,%d]: %lld\n", x, z, ChunkSeed);
			if(ChunkSeed == 5723062625172717733LLu) { //seed 4, chunk X -34 Z 46
				printf("Chunk seed found at X %d Z %d\n", x, z);
				return true;
			}
		}
	}
	//printf("exec?");
	return false;
}

void searchSeeds(int instance, int64_t startSeed, int64_t finalSeed, int range) {
	printf("%lld vs %lld\n", startSeed, finalSeed);
	for(int64_t seed = startSeed; seed < finalSeed; seed++) {
		if(isSeed12Eye(seed, range)) {
			std::cout << "SEED " << seed << " has 12 eyes!!!" << std::endl;
		}
	}
}

int main(int argc, char *argv[]) {
    LayerStack g;
    initBiomes();
    setupGenerator(&g, MC_1_7);
	printf("%i", CLOCKS_PER_SEC);
	//int64_t taskCount = 5;
	
	int64_t run = 0; //9.9b machine
    int64_t startSeed = -100000;
    int64_t finalSeed = 100000;
    int64_t totalSeeds = finalSeed - startSeed;
	
	int range = 200;
	
	int threadNumber = 24;
	int64_t step = totalSeeds / threadNumber;
	std::thread threads[threadNumber];

    clock_t startTime = clock();
	for(int i = 0; i < threadNumber; i++) {
		if(i + 1 == threadNumber) {
			int remainder = totalSeeds % threadNumber;
			
			threads[i] = std::thread(searchSeeds, i, startSeed + (step * i), startSeed + step * (i+1) + remainder, range);
		}
		else
			threads[i] = std::thread(searchSeeds, i, startSeed + (step * i), startSeed + step * (i+1), range);
	}
	
	for(int i = 0; i < threadNumber; i++) {
		threads[i].join();
	}
    clock_t endTime = clock();

    double total_t = ((double)(endTime - startTime))/1000;
    printf("Total time spent: %f seconds | Total seeds: %lld | Seeds/second: %f", total_t, totalSeeds, (double)totalSeeds / total_t);
    return 0;
}