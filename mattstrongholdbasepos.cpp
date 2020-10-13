#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <atomic>
#include <iostream>
#include <thread>
#include <algorithm>
#include "cubiomes/finders.h"
#include <math.h>
#include <string> 
#include <sstream>

double calculateDistanceBetweenPoints(
  double x1, 
  double y1, 
  double x2, 
  double y2) {       
    return sqrt((y2 - y1) * (y2 - y1) + (x2 - x1) * (x2 - x1));
}

static const char* getValidSpawnBiomes()
{
    static const int biomesToSpawnIn[] = {forest, plains, taiga, taiga_hills, wooded_hills, jungle, jungle_hills};
    static char isValid[256];
    unsigned int i;

    if (!isValid[biomesToSpawnIn[0]])
        for (i = 0; i < sizeof(biomesToSpawnIn) / sizeof(int); i++)
            isValid[ biomesToSpawnIn[i] ] = 1;

    return isValid;
}

int testSpawnPositions(Pos* out, int64_t seed, const int mcversion, LayerStack *g, int *cache, int64_t worldSeed, double range)
{
    const char *isSpawnBiome = getValidSpawnBiomes();
	
	applySeed(g, seed);
    Pos spawn;
    int found;
    int i;
	int results = 0;
	
    Layer *l = &g->layers[L_RIVER_MIX_4];

    setSeed(&worldSeed);
    spawn = findBiomePosition(mcversion, l, cache, 0, 0, 256, isSpawnBiome,
            &worldSeed, &found);
	
    if (!found)
    {
        spawn.x = spawn.z = 8;
    }
	
    for (i = 0; i < 1000; i++)// && !canCoordinateBeSpawn(worldSeed, g, cache, spawn); i++)
	{
		spawn.x += nextInt(&worldSeed, 64) - nextInt(&worldSeed, 64);
		spawn.z += nextInt(&worldSeed, 64) - nextInt(&worldSeed, 64);
	}
    return 0;
}

void searchSeeds(std::string instance, long long startSeed, long long finalSeed) {
	LayerStack g;
	
	setupGenerator(&g, MC_1_7);
	
	Pos positions[1001] = {0}; //cache for the positions func
	FILE* arq = fopen(std::string(instance).c_str(), "a");
	
    for (long long seed = startSeed; seed < finalSeed; seed++) {		
        int ret = testSpawnPositions((Pos*)&positions, seed, MC_1_7, &g, NULL, seed, 15.0);
		if(ret >= 10)
			fprintf(arq, "%lld\n", seed);
    }
	fclose(arq);
}

std::string NumberToString ( int Number )
{
	std::ostringstream ss;
	ss << Number;
	return ss.str();
}

int main(int argc, char *argv[]) {
    initBiomes();

    long long startSeed = 1;
    long long finalSeed = 1000000;
    long long totalSeeds = finalSeed - startSeed;
	
	std::cout << totalSeeds << std::endl;
	
	int threadNumber = 24;
	long long step = totalSeeds / threadNumber;
	
	std::cout << step << std::endl;
	std::thread threads[24];

    clock_t startTime = clock();
	//searchSeeds("rofl", 4, 5);
	for(int i = 0; i < threadNumber; i++) {
		if(i + 1 == threadNumber) {
			int remainder = totalSeeds % threadNumber;
			threads[i] = std::thread(searchSeeds, NumberToString(i), startSeed + step * i, startSeed + ((i + 1) * step) + remainder);
		}
		else
			threads[i] = std::thread(searchSeeds, NumberToString(i), startSeed + step * i, startSeed + ((i + 1) * step));
	}
	
	for(int i = 0; i < threadNumber; i++) {
		threads[i].join();
	}	
    clock_t endTime = clock();
    printf("start:%ld end:%ld\n", startTime, endTime);
    double total_t = ((double)(endTime - startTime));
    printf("Total time spent: %f seconds | Total seeds: %lld | Seeds/second: %f\n", total_t/(CLOCKS_PER_SEC*10), totalSeeds, totalSeeds / total_t*(CLOCKS_PER_SEC*10));
    printf("CLOCKS: %ld", CLOCKS_PER_SEC);
    return 0;
}