#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <atomic>
#include <iostream>
#include <thread>
#include <algorithm>
#include "finders.h"
int main(int argc, char *argv[]) {
    LayerStack g;
    initBiomes();
    setupGenerator(&g, MC_1_7);
    long long startSeed = 0;
    long long finalSeed = 10000;
    long long totalSeeds = finalSeed - startSeed;

    clock_t startTime = clock();
    for (long long seed = startSeed; seed < finalSeed; seed++) {
        applySeed(&g, seed);
        Pos position = getSpawn(MC_1_7, &g, NULL, seed);
        std::cout << "seed: " << seed << " Spawn: X:" << position.x << " Z: " << position.z <<std::endl;
    }
    clock_t endTime = clock();
    double total_t = ((double)(endTime - startTime));
    printf("Total time spent: %f seconds | Total seeds: %lld | Seeds/second: %f", total_t / 1000, totalSeeds, totalSeeds / total_t * 1000);
    return 0;
}