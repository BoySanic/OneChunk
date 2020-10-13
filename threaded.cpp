#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <atomic>
#include <iostream>
#include <algorithm>
#include <thread>
#include "gen/layers.h"
#include "gen/generator.h"
#include "gen/javarnd.h"



Biome biomesToSpawnIn[7] = {};
void getIntsFake(LayerStack g, int* out, int var6, int var7, int var10, int var11, int64_t seed) {
    applySeed(&g, seed);
    g.layers[42].getMap(&g.layers[42], out, var6, var7, var10, var11);
    //printf("[getInts] Total time spent: %.0f ms\n", total_t);
}
void searchSeeds(int tasknum, long long startSeed, long long finalSeed)
{
    LayerStack g = setupGeneratorMC17();
    //printf("0x%x\n", &g);
    int* map = allocCache(&g.layers[42], 256, 256);

    for (long long seed = startSeed; seed < finalSeed; seed++) {
        getIntsFake(g, map, -64, -64, 129, 129, seed); 
        std::cout << "Map stuff: " << map[0] << " " << map[1] << " " << map[2]<< " " << map[3]<< " " << map[4]<< " " << map[5]<< " " << map[6] << " "<< map[7] << " "<< map[8] << " "<< map[9]<< std::endl;
        int ChunkPos[3] = {};
        int var14 = 0;
        int64_t seedTemp = seed;
        int trueCounter = 0;
        int timesSet = 0;

        for(int i = 0; i < 16641; i++){
            int var16 = -64 + i % 129 << 2;
            int var17 = -64 + i / 129 << 2;
            Biome biomeIndex;
            if(map[i] < sizeof(biomes)/sizeof(biomes[0]) && map[i] >= 0){ 
                timesSet++;
                biomeIndex = biomes[map[i]];
            }
            else
                biomeIndex = biomes[0];
            
            if(std::any_of(std::begin(biomesToSpawnIn), std::end(biomesToSpawnIn), [=](Biome n){return n.id == biomeIndex.id;})){
                trueCounter++;
            }
            if(std::any_of(std::begin(biomesToSpawnIn), std::end(biomesToSpawnIn), [=](Biome n){return n.id == biomeIndex.id;}) && (ChunkPos == NULL || nextInt(&seedTemp, var14 + 1) == 0)){
                ChunkPos[0] = var16;
                ChunkPos[1] = 0;
                ChunkPos[2] = var17;
                var14++;
            }
        }
        int x;
        int y = 64;
        int z;
        if(ChunkPos != NULL){
            x = ChunkPos[0];
            z = ChunkPos[2];
        }
        else{
            std::cout << "Anger error about no spawn biome found" << std::endl;
        }
        std::cout << "chunkstuff x:" << x << " y: " << y << std::endl;  
        int attempts = 0;
        while(true){
            x += nextInt(&seedTemp, 64) - nextInt(&seedTemp, 64);
            z += nextInt(&seedTemp, 64) - nextInt(&seedTemp, 64);
            attempts++;
            if(attempts == 241){
                break;
            }
        }
        std::cout << "seed: " << seed << "HOLY COW THE SPAWN POS IS X: " << x << " Z: " << z << std::endl;
        std::cout << var14 << " if check" << std::endl;
        std::cout << timesSet << " times Set" << std::endl;
    }
    printf("Task %d finished.\n", tasknum);
    freeGenerator(g);
    free(map);
}
/*
    ocean = 0, plains, desert, extremeHills, forest, taiga, swampland, river, hell, sky, // 0-9
    frozenOcean, frozenRiver, icePlains, iceMountains, mushroomIsland, mushroomIslandShore, beach, desertHills, forestHills, taigaHills,  // 10-19
    extremeHillsEdge, jungle, jungleHills, jungleEdge, deepOcean, stoneBeach, coldBeach, birchForest, birchForestHills, roofedForest, // 20-29
    coldTaiga, coldTaigaHills, megaTaiga, megaTaigaHills, extremeHillsPlus, savanna, savannaPlateau, mesa, mesaPlateau_F, mesaPlateau, // 30-39
*/
int main(int argc, char *argv[]) {
    initBiomes();
    long long startSeed = 4;
    long long finalSeed = 5;
    biomesToSpawnIn[0] = biomes[4];
    biomesToSpawnIn[1] = biomes[5];
    biomesToSpawnIn[2] = biomes[1];
    biomesToSpawnIn[3] = biomes[19];
    biomesToSpawnIn[4] = biomes[18];
    biomesToSpawnIn[5] = biomes[21];
    biomesToSpawnIn[6] = biomes[22];
    int threadNumber = 1; //number of threads to spawn.
    long long totalSeeds = finalSeed - startSeed;
    long long seedStep = totalSeeds / threadNumber;

    //Inits the static database - gives biomes an ID, its own layer func, and also sets the parents of each layer
    clock_t startTime = clock();
    std::thread threads[24];
    for (int thNum = 0; thNum < threadNumber; thNum++) {
        threads[thNum] = std::thread(searchSeeds, thNum, startSeed + seedStep * thNum, startSeed + seedStep * (thNum+1));
    }

    for(int thNum = 0; thNum < threadNumber; thNum++){
        threads[thNum].join();
    }
    clock_t endTime = clock();
    double total_t = ((double)(endTime - startTime));
    printf("Total time spent: %f seconds | Total seeds: %lld | Seeds/second: %f", total_t / 1000, totalSeeds, totalSeeds / total_t * 1000);
    return 0;
}