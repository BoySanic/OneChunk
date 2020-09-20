int next(ulong* seed, short bits)
{
	*seed = (*seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
	return *seed >> (48 - bits);
}

int nextInt(ulong* seed, short bound)
{
	int bits, value;
	do {
		*seed = (*seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
		bits = *seed >> 17;
		value = bits % bound;
	} while(bits - value + (bound - 1) < 0);
	return value;
}

int nextIntUnknown(ulong* seed, short bound)
{
	if((bound & -bound) == bound) {
		*seed = (*seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
		return (int)((bound * (*seed >> 17)) >> 31);
	}

	int bits, value;
	do {
		*seed = (*seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
		bits = *seed >> 17;
		value = bits % bound;
	} while(bits - value + (bound - 1) < 0);
	return value;
}
List biomes;
int[] GenLayerZoomGetInts(int minX, int minZ, int differenceX, int differenceZ){
    
}
int[] GenBiomeGetInts(int minX, int minZ, int differenceX, int differenceZ){
    int[] parentResult = GenLayerZoomGetInts(minX, minZ, differenceX, differenceZ);
    //Implement
}
void createSpawnPosition(ulong seed){
    //0, 0. 256, biomes, random
    int var6 = 0 - 256 >> 2;
    int var7 = 0 - 256 >> 2;
    int var8 = 0 + 256 >> 2;
    int var9 = 0 + 256 >> 2;
    int var10 = var8 - var6 + 1;
    int var11 = var9 - var7 + 1;
    int[] var12 = GenBiomeGetInts(var6, var7, var10, var11);
}
int main(int argc, char *argv[]){
    uint64_t start;
    uint64_t count;
    for (int i = 1; i < argc; i += 2) {
		const char *param = argv[i];
        if (strcmp(param, "-s") == 0 || strcmp(param, "--start") == 0) {
			sscanf(argv[i + 1], "%llu", &START);
		} else if (strcmp(param, "-e") == 0 || strcmp(param, "--count") == 0) {
			sscanf(argv[i + 1], "%llu", &COUNT);
		} else {
			fprintf(stderr,"Unknown parameter: %s\n", param);
		}
    }
    for(uint64_t i = start; i < start + count; i++){
        createSpawnPosition(seed);
    }
}

