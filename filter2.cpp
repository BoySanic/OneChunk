#include "cubiomes/javarnd.h"
#include <iostream>
#include "cubiomes/layers.h"
#include "cubiomes/finders.h"
#include "cubiomes/generator.h"
#include <math.h>
#include <vector>
#include <iomanip>
#include <sstream>
#include <string>
#include <fstream>
#include <thread>
std::vector<int64_t> results;
void getStrongholdPositions(int64_t worldSeed, int SH, Pos* out, LayerStack* g)
{
	const char* isStrongholdBiome = getValidStrongholdBiomes();
	

	applySeed(g, worldSeed);
	Layer *l = &g->layers[L_RIVER_MIX_4];
	int64_t seedCopy = worldSeed;
    int64_t* seedPtr = &seedCopy;
	setSeed(seedPtr);
	long double angle = nextDouble(seedPtr) * PI * 2.0;
	int var6 = 1;
	
	//1 here determines how many strongholds to generate
	for (int var7 = 0; var7 < SH; ++var7)
	{
		long double distance = (1.25 * (double)var6 + nextDouble(seedPtr)) * 32.0 * (double)var6;
		int x = (int)round(cos(angle) * distance);
		int z = (int)round(sin(angle) * distance);
		
		//std::cout << "X - " << x << " Z - " << z << std::endl;
		
		Pos biomePos = findBiomePosition(MC_1_7, l, NULL, (x << 4) + 8, (z << 4) + 8, 112, isStrongholdBiome, seedPtr, NULL);

		out[var7].x = biomePos.x >> 4;
		out[var7].z = biomePos.z >> 4;
		//std::cout << "biome pos X " << biomePos.x << " Z " << biomePos.z << std::endl;
		//std::cout << "adding " << (2 * PI / 3.0) << std::endl;
		angle += 2 * PI / 3.0;
	}
}

bool attemptSpawn(const LayerStack *g, int *cache, int64_t worldSeed, int PortalX, int PortalZ)
{
    Pos spawn;
    const Layer *l = &g->layers[L_RIVER_MIX_4];
    const char *isSpawnBiome = getValidSpawnBiomes();
    int64_t seedCopy = worldSeed;
    int64_t* seedPtr = &seedCopy;
    setSeed(seedPtr);
    int found;
    spawn = findBiomePosition(MC_1_7, l, cache, 0, 0, 256, isSpawnBiome,
            seedPtr, &found);

    if (!found)
    {
        //printf("Unable to find spawn biome.\n");
        spawn.x = spawn.z = 8;
    }
    for (int i = 0; i < 1000; i++)
    {
        spawn.x += nextInt(seedPtr, 64) - nextInt(seedPtr, 64);
        spawn.z += nextInt(seedPtr, 64) - nextInt(seedPtr, 64);
        int cx = (int)(spawn.x / 16);
        int cz = (int)(spawn.z / 16);
        if(cx <= PortalX + 1 && cx >= PortalX - 1 && cz >= PortalZ - 1 && cz <= PortalZ + 1){
            
            return true;
        }
    }
    return false;
}
FILE *fp;
void threadWork(std::vector<std::string> data, int threadNumber, int offset, int amount){
    int64_t structureSeed;
    int ChunkX;
    int ChunkZ;
    LayerStack g;
	setupGenerator(&g, MC_1_7);

    std::string thread = std::to_string(threadNumber);
    thread += "out.txt";
    
    for(int i = offset; i < offset + amount; i++){
        std::string line = data[i];
        std::istringstream iss(line);
        //std::cout << "Thread " << threadNumber << " Testing " << structureSeed << std::endl;
        if(!(iss >> structureSeed >> ChunkX >> ChunkZ)){break;}
        for (int64_t upperBits = 0; upperBits < 1L << 16; upperBits++) {

            Pos* position = (Pos*)malloc(sizeof(Pos));
            int64_t worldSeed = (upperBits << 48) | structureSeed;
            if(threadNumber == 1){
                    //printf("%lld %d %lld\n", structureSeed, upperBits, worldSeed);
            }
            getStrongholdPositions(worldSeed, 1, position, &g);
            if((position->x < ChunkX + 7 && position->x > ChunkX - 7) && (position->z < ChunkZ + 7 && position->z > ChunkZ - 7)){
                applySeed(&g, worldSeed);
                bool found = attemptSpawn(&g, NULL, worldSeed, ChunkX, ChunkZ);
                if(found){
                    fprintf(fp, "%d %lld %d %d %d %d\n", threadNumber, (worldSeed), position->x, position->z, ChunkX, ChunkZ);
                    fflush(fp);
                }
            }
            delete position;
            position = NULL;
        }
        if(i % 10 == 0){
            std::cout << "Thread " << threadNumber << " index " << i << std::endl;
        }
    }  
    std::cout << "Thread " << threadNumber << " closing" << std::endl;
}
int main(int argc, char **argv ){
    clock_t start = clock();
    int test = 1L << 16;
    printf("%d test", test);
    int threadCount = 24;
    char* filename;
    for (int i = 1; i < argc; i += 2) {
		const char *param = argv[i];
		if (strcmp(param, "-t") == 0 || strcmp(param, "--threads") == 0) {
			threadCount = atoi(argv[i + 1]);
		} else if (strcmp(param, "-f") == 0 || strcmp(param, "--file") == 0) {
			filename = argv[i + 1];
		} 
	    else {
			fprintf(stderr,"Unknown parameter: %s\n", param);
		}
	}
    fp = fopen("out.txt", "w+");
    std::string line;
    std::fstream infile;
    infile.open(filename, std::ios::in);
    double seconds_per_structure_seed = 0.0;
    std::vector<std::thread> threads;
    initBiomes();
    std::cout << "Begin loading threads" << std::endl;  
    int thread = 0;
    int curr = 0;

    std::vector<std::string> arr;
    while(std::getline(infile, line)){
        int ChunkX = 0;
        int ChunkZ = 0;
        long structureSeed = 0;
        std::istringstream iss(line);
        if(!(iss >> structureSeed >> ChunkX >> ChunkZ)){break;}
        if(!((ChunkX > 63 || ChunkX < -63) || (ChunkZ > 63 || ChunkZ < -63) || (ChunkX < 25 && ChunkX > -25) && (ChunkZ < 25 && ChunkZ > -25))){
            arr.push_back(line);
        }
        //printf("Completed structure seed %d out of %d\n", curr, max);
        //printf("Seconds Per Structure Seed: %f\n", (double)(((double)end - (double)start)/1000000.0));
        //curr++;
    }
    infile.close();
    int structureSeedsPerThread = arr.size()/threadCount;
    int remainder = arr.size() % threadCount;
    int amount = structureSeedsPerThread + remainder;
    std::vector<std::string> tArr;
    int offset = 0;
    for(int i = 0; i < threadCount; i++){
        std::cout << "Thread " << i << " started with " << amount << " seeds" << std::endl;
        curr = 0;
        threads.push_back(std::thread(threadWork, arr, i, offset, amount));
        offset += amount;
        amount = structureSeedsPerThread;
    }
    for(int thNum = 0; thNum < threadCount; thNum++){
        threads[thNum].join();
    }   
    for(int i = 0; i < results.size(); i++){
        fprintf(fp, "%ld\n", results[i]);
        fflush(fp);
    }
    fclose(fp);
    clock_t end = clock();
    printf("File took %f seconds to complete.\n", (double)(((double)end - (double)start)/1000000.0));
    return 0;
}