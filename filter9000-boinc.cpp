#include <iostream>
#include <time.h>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <thread>

#include "Utils/Data.h"
#include "Pieces/Stairs2.h"
#include "Pieces/Straight.h"
#include "Pieces/Crossing.h"
#include "Pieces/LeftTurn.h"
#include "Pieces/RightTurn.h"
#include "Pieces/StairsStraight.h"
#include "Pieces/Stairs.h"
#include "Pieces/Prison.h"
#include "Pieces/ChestCorridor.h"
#include "Pieces/RoomCrossing.h"	

#include "cubiomes/layers.h"
#include "cubiomes/finders.h"
#include "cubiomes/generator.h"

#include "Utils/Random.h"


#ifdef BOINC
  #include "boinc/boinc_api.h"
#if defined _WIN32 || defined _WIN64
  #include "boinc/boinc_win.h"
#endif
#endif

void setFirstPiece(Data* data) {
	data->reset();
	
	int64_t worldSeed = data->seed;
	//The chunk where the base pos of the stronghold is at
	int chunkX = data->StartChunkX;
	int chunkZ = data->StartChunkZ;

	data->rng->setSeed(worldSeed);
	
	int64_t var7 = data->rng->nextLong();
	int64_t var9 = data->rng->nextLong();
	int64_t var13 = (int64_t)chunkX * var7;
	int64_t var15 = (int64_t)chunkZ * var9;
	int64_t internalSeed = var13 ^ var15 ^ worldSeed;

	data->rng->setSeed(internalSeed);

	data->rng->Next(32); //Minecraft and its spaghetti. USELESS RNG CALL. SERIOUSLY?
	
	Stairs2::GeneratePiece(data);
	data->priorityComponentType = 1;
	Stairs2::BuildComponent(data);
	data->priorityComponentType = 0;
}

void BuildComponent(Data* data, PieceInfo pieceInfo) {
	int componentType = pieceInfo.componentType;

	if(componentType == CROSSING_PIECE) Crossing::BuildComponent(data, pieceInfo);
	else if(componentType == LEFTTURN_PIECE) LeftTurn::BuildComponent(data, pieceInfo);
	else if(componentType == STRAIGHT_PIECE) Straight::BuildComponent(data, pieceInfo);
	else if(componentType == STAIRSSTRAIGHT_PIECE) StairsStraight::BuildComponent(data, pieceInfo);
	else if(componentType == ROOMCROSSING_PIECE) RoomCrossing::BuildComponent(data, pieceInfo);
	else if(componentType == PRISON_PIECE) Prison::BuildComponent(data, pieceInfo);
	else if(componentType == STAIRS_PIECE) Stairs::BuildComponent(data, pieceInfo);
	else if(componentType == RIGHTTURN_PIECE) LeftTurn::BuildComponent(data, pieceInfo);
	else if(componentType == CHESTCORRIDOR_PIECE) ChestCorridor::BuildComponent(data, pieceInfo);
	else if(componentType == CORRIDOR_PIECE || componentType == LIBRARY_PIECE) {}
	else //Unknown piece?!? WTF?
		std::cout << "COULD NOT BUILD COMPONENT TYPE " << componentType << std::endl;
}

PieceInfo getLastPiece(Data* threadData, int64_t seed, int startChunkX, int startChunkZ) {
	threadData->seed = seed;
	threadData->StartChunkX = startChunkX;
	threadData->StartChunkZ = startChunkZ;
	
	setFirstPiece(threadData);
	
	while(threadData->pieces_pending.size() != 0) {			
		int randomPieceNumber = threadData->rng->nextInt(threadData->pieces_pending.size());

		PieceInfo pieceChosen = threadData->pieces_pending.at(randomPieceNumber);
		threadData->pieces_pending.erase(threadData->pieces_pending.begin() + randomPieceNumber);

		BuildComponent(threadData, pieceChosen);
		
		if(threadData->portalFound)
			break;
	}
	PieceInfo lastPiece = threadData->pieces[threadData->pieceCnt - 1];
	return lastPiece;
}

Position getCenterPos(BoundingBox box) {
	Position ret;
	ret.x = (box.end.x + box.start.x) / 2;
	ret.z = (box.end.z + box.start.z) / 2;
	return ret;
}
FILE *fp;
void getStrongholdPositions(LayerStack* g, int64_t* worldSeed, int SH, Data* data, int* cache, BoundingBox* boxCache, int desiredX, int desiredZ)
{
	static const char* isStrongholdBiome = getValidStrongholdBiomes();
	
	int64_t copy = *worldSeed;

	applySeed(g, *worldSeed);
	
	Layer *l = &g->layers[L_RIVER_MIX_4];
	
	setSeed(worldSeed);
	long double angle = nextDouble(worldSeed) * PI * 2.0;
	int var6 = 1;
	
	//SH here determines how many strongholds to generate
	for (int var7 = 0; var7 < SH; ++var7)
	{
		long double distance = (1.25 * (double)var6 + nextDouble(worldSeed)) * 32.0 * (double)var6;
		int x = (int)round(cos(angle) * distance);
		int z = (int)round(sin(angle) * distance);
		
		Pos biomePos = findBiomePosition(MC_1_7, l, cache, (x << 4) + 8, (z << 4) + 8, 112, isStrongholdBiome, worldSeed, NULL);

		x = biomePos.x >> 4;
		z = biomePos.z >> 4;
		
		PieceInfo lastPiece = getLastPiece(data, copy, x, z);
		if(lastPiece.componentType == PORTALROOM_PIECE) {
			int pos1X, pos1Z, pos2X, pos2Z;
			
			if(lastPiece.coordBaseMode == 0) {
				pos1X = lastPiece.box.start.x + 3;
				pos1Z = lastPiece.box.start.z + 12;
				pos2X = lastPiece.box.start.x + 7;
				pos2Z = lastPiece.box.start.z + 8;
			}
			
			else if(lastPiece.coordBaseMode == 1) {
				pos1X = lastPiece.box.start.x + 7;
				pos1Z = lastPiece.box.start.z + 7;
				pos2X = lastPiece.box.start.x + 3;
				pos2Z = lastPiece.box.start.z + 3;
			}
			
			else if(lastPiece.coordBaseMode == 2) {
				pos1X = lastPiece.box.start.x + 3;
				pos1Z = lastPiece.box.start.z + 7;
				pos2X = lastPiece.box.start.x + 7;
				pos2Z = lastPiece.box.start.z + 3;
			}
			
			else if(lastPiece.coordBaseMode == 3) {
				pos1X = lastPiece.box.start.x + 12;
				pos1Z = lastPiece.box.start.z + 7;
				pos2X = lastPiece.box.start.x + 8;
				pos2Z = lastPiece.box.start.z + 3;
			}
			
			if((pos1X - 8) >> 4 == desiredX && (pos2X - 8) >> 4 == desiredX) {
				if((pos1Z - 8) >> 4 == desiredZ && (pos2Z - 8) >> 4 == desiredZ) {
					Position center = getCenterPos(lastPiece.box);
					fprintf(fp, "%lld %d %d %d %d\n", copy, center.x >> 4, center.z >> 4, center.x, center.z);
				}
			}
			
			/*if((lastPiece.box.start.x >> 4 == desiredX && lastPiece.box.start.z >> 4 == desiredZ) || (lastPiece.box.end.x >> 4 == desiredX && lastPiece.box.end.z >> 4 == desiredZ)) {
				Position center = getCenterPos(lastPiece.box);
				printf("%lld %d %d %d %d\n", copy, center.x >> 4, center.z >> 4, center.x, center.z);
			}
			/*std::cout << "Portal room for Stronghold " << var7 << " is at " << center.x << " " << center.z << ", and there are " << data->pieceCnt << " pieces that were generated before the portal room itself.\n\n" << std::endl;*/
		}
		angle += 2 * PI / 3.0;
	}
}

const char* getValidVillageBiomes()
{
    static char validVillageBiomes[256];

    if (!validVillageBiomes[plains])
    {
        int id;
        for (id = 0; id < 256; id++)
        {
            if (biomeExists(id) && (biomes[id].id == plains || biomes[id].id == desert || biomes[id].id == savanna))
                validVillageBiomes[id] = 1;
        }
    }

    return validVillageBiomes;
}

bool CanActuallySpawn(int64_t seed, int x, int z, Layer* layer, int* cache) {
	const char* isValid = getValidVillageBiomes();
	return areBiomesViable(layer, cache, x * 16 + 8, z * 16 + 8, 0, isValid);
}

void doSeed(int64_t seed, int x, int z, LayerStack g, int* cache, Data* threadData, BoundingBox* boxCache) {
	if(!CanActuallySpawn(seed, x, z, &g.layers[L_RIVER_MIX_4], cache))
		return;
	
	getStrongholdPositions(&g, &seed, 1, threadData, cache, boxCache, x, z);
}



time_t start;
int64_t total;
std::vector<std::string> arr;
time_t elapsed_chkpoint = 0;

struct checkpoint_vars {
    unsigned long long offset;
    time_t elapsed_chkpoint;
};

int main(int argc, char **argv) {
	fp = fopen("out.txt", "w+");
	initBiomes();

    int64_t checkpointOffset = 0;
	std::vector<std::thread> threads;

    #ifdef BOINC
        BOINC_OPTIONS options;
        boinc_options_defaults(options);
        options.normal_thread_priority = true;
        boinc_init_options(&options);
    #endif
    FILE *checkpoint_data = boinc_fopen("filter9000-checkpoint.txt", "rb");
    if(!checkpoint_data){
        fprintf(stderr, "No checkpoint to load\n");
    }
    else{
        #ifdef BOINC
            boinc_begin_critical_section();
        #endif

        struct checkpoint_vars data_store;
        fread(&data_store, sizeof(data_store), 1, checkpoint_data);
        checkpointOffset = data_store.offset;
        elapsed_chkpoint = data_store.elapsed_chkpoint;
        fprintf(stderr, "Checkpoint loaded, task time %d s, seed pos: %llu\n", elapsed_chkpoint, checkpointOffset);
        fclose(checkpoint_data);

        #ifdef BOINC
            boinc_end_critical_section();
        #endif
    }

    #ifdef BOINC
	APP_INIT_DATA aid;
	boinc_get_init_data(aid);
    #endif

	std::string line;
	std::ifstream infile(filename);	
    while(std::getline(infile, line)){
        int ChunkX = 0;
        int ChunkZ = 0;
        int64_t structureSeed = 0;
        std::istringstream iss(line);
        if(!(iss >> structureSeed >> ChunkX >> ChunkZ)){break;}
        arr.push_back(line);
    }
	infile.close();

    total = arr.size();
    start_time = time(NULL);


    int64_t structureSeed;
    int ChunkX;
    int ChunkZ;

    int* cache = (int*)malloc(sizeof(int) * 16 * 256 * 256);
	Data* data = new Data();
	BoundingBox* boxCache = (BoundingBox*)malloc(sizeof(BoundingBox));
	
	LayerStack g;
	setupGenerator(&g, MC_1_7);
    for(int i = 0+checkpointOffset; i < total; i++){
        time_t elapsed = time(NULL) - start_time;
		std::string line = arr[i];
        std::istringstream iss(line);
        if(!(iss >> structureSeed >> ChunkX >> ChunkZ)){break;}
		for (int64_t upperBits = 0; upperBits < 1L << 16; upperBits++) {
			int64_t worldSeed = (upperBits << 48) | structureSeed;
			applySeed(&g, worldSeed);
			doSeed(worldSeed, ChunkX, ChunkZ, g, cache, data, boxCache);
		}
        if(i % 10 || boinc_time_to_checkpoint()){
            #ifdef BOINC
		        boinc_begin_critical_section(); // Boinc should not interrupt this
            #endif
            // Checkpointing section below
			boinc_delete_file("filter9000-checkpoint.txt"); // Don't touch, same func as normal fdel
            FILE *checkpoint_data = boinc_fopen("filter9000-checkpoint.txt", "wb");
			struct checkpoint_vars data_store;
			data_store.offset = offset;
            data_store.elapsed_chkpoint = elapsed_chkpoint + elapsed;
            fwrite(&data_store, sizeof(data_store), 1, checkpoint_data);
            fclose(checkpoint_data);
            checkpointTemp = 0;
            #ifdef BOINC
                boinc_end_critical_section();
                boinc_checkpoint_completed(); // Checkpointing completed
            #endif
        }
    }
    #ifdef BOINC
        boinc_begin_critical_section();
    #endif
    time_t elapsed = (time(NULL) - start_time) + elapsed_chkpoint;
    double done = (double)total;
    double speed = done / (double) elapsed;
    fprintf(stderr, "\nSpeed: %.2lf/s\n", speed );
    fprintf(stderr, "Done.\n");
    fprintf(stderr, "Processed: %llu seeds in %.2lfs seconds.\n", COUNT, (double) elapsed_chkpoint + (double) elapsed );
    fprintf(stderr, "Have %llu output seeds.\n", outCount);
    fflush(stderr);
    outSeeds.close();
    boinc_delete_file("filter9000-checkpoint.txt");
    #ifdef BOINC
        boinc_end_critical_section();
    #endif
    boinc_finish(0);

    time_t end = time(NULL);
    printf("File took %f seconds to complete.\n", (double)(((double)end - (double)start)));
}