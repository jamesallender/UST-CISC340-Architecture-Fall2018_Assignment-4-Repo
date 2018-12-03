#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8 /* number of machine registers */

#define ADD 0
#define NAND 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000

// Enums
enum dirty_bit {dirty, clean};
enum valid_bit {valid, invalid};
enum action_type {cache_to_processor, processor_to_cache, memory_to_cache, cache_to_memory,
cache_to_nowhere};
enum access_type {read_mem, write_mem};
enum hit_or_miss {hit, miss};

// Structures
typedef struct cacheEntryStruct {
    enum dirty_bit dirtyBit;
	enum valid_bit validBit;
	int tag;
	int *data;
	int cyclesSinceLastUse;
} cacheEntryType;

typedef struct stateStruct {
    int pc;
	int mem[NUMMEMORY];
	int reg[NUMREGS];
	int numMemory;
	cacheEntryType **cacheArr;
	int sets;
	int ways;
	int wordsPerBlock;
	int discriptiveFlag;
} stateType;

// Function Headers
int field0(int instruction);
int field1(int instruction);
int field2(int instruction);
int opcode(int instruction);
int getTag(int address, stateType* state);
int getSet(int address, stateType* state);
int getBlkOffset(int address, stateType* state);
int searchCache(int address, stateType* state);
int alocateCacheLine(int address, stateType* state);
int cacheSystem(int address, stateType* state, enum access_type action, int write_value);
int signExtend(int num);
void run(stateType* state);
void print_action(int address, int size, enum action_type type);
void printCache(stateType* state);
void incrementCyclesSinceLastUse(stateType* state);
int memToCache(int address, stateType* state);
void cacheToMem(int address, stateType* state);
int getAddressBase(int address, stateType* state);

// Functions
char* getDirtyBitName(enum dirty_bit bit) 
{
   switch (bit) 
   {
      case dirty: return "dirty";
      case clean: return "clean";
      default: return"invalid dirty clean enum val";
   }
}
char* getValidBitName(enum valid_bit bit) 
{
   switch (bit) 
   {
      case valid: return "valid";
      case invalid: return "invalid";
      default: return"invalid vaid invalid enum val";
   }
}
char* getAccessTypeName(enum access_type bit) 
{
   switch (bit) 
   {
      case read_mem: return "read_mem";
      case write_mem: return "write_mem";
      default: return"invalid read write enum val";
   }
}
char* printHittOrMiss(enum hit_or_miss found) 
{
   switch (found) 
   {
      case hit: return "hit";
      case miss: return "miss";
      default: return"invalid hit miss enum val";
   }
}
enum hit_or_miss isHittOrMiss(int found) 
{
   if (found == -1){
   	return miss;
   }
   else if (found > -1){
   	return hit;
   }
   else{
     abort();
   }
}



int field0(int instruction){
    return( (instruction>>19) & 0x7);
}

int field1(int instruction){
    return( (instruction>>16) & 0x7);
}

int field2(int instruction){
    return(instruction & 0xFFFF);
}

int opcode(int instruction){
    return(instruction>>22);
}

int getTag(int address, stateType* state){
	int words_per_blk = state->wordsPerBlock;
	int num_of_set = state->sets;

	int bits_blkOffset = log(words_per_blk)/log(2);
	int bits_set = log(num_of_set)/log(2);

	int tag = address >> (bits_blkOffset + bits_set);

	return tag;
}

int getSet(int address, stateType* state){
	int num_of_set = state->sets;
	int bits_needed = log(num_of_set) / log(2);
	int mask = 0;



	for(int i=0; i<bits_needed ;i++){
		mask += pow(2,i);
	}

	int set = (address & mask);

	return set;
}

int getBlkOffset(int address, stateType* state){
	int words_per_blk = state->wordsPerBlock;
	int bits_needed = log(words_per_blk) / log(2);
	int mask = 0;

	for(int i=0; i<bits_needed ;i++){
		mask += pow(2,i);
	}

	int offset = (address & mask);

	return offset;
}

/*
* Log the specifics of each cache action.
*
* address is the starting word address of the range of data being transferred.
* size is the size of the range of data being transferred.
* type specifies the source and destination of the data being transferred.
*
* cache_to_processor: reading data from the cache to the processor
* processor_to_cache: writing data from the processor to the cache
* memory_to_cache: reading data from the memory to the cache
* cache_to_memory: evicting cache data by writing it to the memory
* cache_to_nowhere: evicting cache data by throwing it away
*/
void print_action(int address, int size, enum action_type type){
	printf("transferring word [%i-%i] ", address, address + size - 1);
	if (type == cache_to_processor) {
		printf("from the cache to the processor\n");
	} else if (type == processor_to_cache) {
		printf("from the processor to the cache\n");
	} else if (type == memory_to_cache) {
		printf("from the memory to the cache\n");
	} else if (type == cache_to_memory) {
		printf("from the cache to the memory\n");
	} else if (type == cache_to_nowhere) {
		printf("from the cache to nowhere\n");
	}
}

void printCache(stateType* state){
	if(state->discriptiveFlag == 1){
		// loop through all sets of cache
		printf("\nCache Contents:\n");
		for (int i = 0; i < state->sets; i++ ){
			printf("Set: %d\n", i);
			// loop through all the ways of a set
			for (int k = 0; k < state->ways; k++ ){
				printf("Way: %d\n", k);
				printf("tag: %d\n", state->cacheArr[i][k].tag);
				printf("cyclesSinceLastUse: %d\n", state->cacheArr[i][k].cyclesSinceLastUse);
				printf("dirtyBit: %s\n", getDirtyBitName(state->cacheArr[i][k].dirtyBit));
				printf("validBit: %s\n", getValidBitName(state->cacheArr[i][k].validBit));
				printf("data:\t");
				for (int l = 0; l < state->wordsPerBlock; l++ ){
					printf("%d", state->cacheArr[i][k].data[l]); 
					// printf("%p",(void *)&state->cacheArr[i][k].data[l]); 
					if (l != state->wordsPerBlock-1){
						printf(" | "); 
					}
				}
				printf("\n\n");
			}
			printf("-------------\n");
		}
	}
}

// return >= 0 way contaning the address if the given address is in cache otherwise -1
int searchCache(int address, stateType* state){
	int set = getSet(address, state);
	int tag = getTag(address, state);
	// loop through all the ways of the address's set
	for (int way = 0; way < state->ways; way++ ){
		// if the tag is found in the set return 1
		if (state->cacheArr[set][way].validBit == valid && state->cacheArr[set][way].tag == tag){
			return way;
		}
	}	
	// otherwise return -1
	return -1;
}

// Returns the way in the cach set corisponding to the given address that can be overwriten
int alocateCacheLine(int address, stateType* state){
	int set = getSet(address, state);
	int lru = 0;
	// loop through all the ways of a set
	for (int k = 0; k < state->ways; k++ ){
		// If the current way is invalid return it to be overwriten
		if (state->cacheArr[set][k].validBit == invalid){
			return k;
		}
		// If the current line's cyclesSinceLastUse is greater than the current 
		// lru's cyclesSinceLastUse change the lru to the current way
		if (state->cacheArr[set][k].cyclesSinceLastUse > state->cacheArr[set][lru].cyclesSinceLastUse){
			lru = k;
		}
	}	
	// check if the lru way needs to be written back to memory
	if (state->cacheArr[set][lru].dirtyBit == dirty){
		// write each word in the block to memory
		for (int l = 0; l < state->wordsPerBlock; l++ ){
			state->mem[getAddressBase(address, state) + l] = state->cacheArr[set][lru].data[l];
		}	
		print_action(address, state->wordsPerBlock, cache_to_memory);
	}
	return lru;
}

int getAddressBase(int address, stateType* state){
	int words_per_blk = state->wordsPerBlock;
	int bits_needed = log(words_per_blk) / log(2);
	int mask = 0;

	for(int i=0; i<bits_needed ;i++){
		mask += pow(2,i);
	}
	int baseAddress = (address & !mask);

	return baseAddress;
}

// increment the cyclesSinceLastUse for all items in cache
void incrementCyclesSinceLastUse(stateType* state){
	// loop through all sets of cache
	for (int i = 0; i < state->sets; i++ ){
		// loop through all the ways of a set
		for (int k = 0; k < state->ways; k++ ){
			state->cacheArr[i][k].cyclesSinceLastUse = state->cacheArr[i][k].cyclesSinceLastUse + 1;
		}
	}
}

// int blockOffset = getBlkOffset(address, state);

int memToCache(int address, stateType* state){
	int tag = getTag(address, state);
	int set = getSet(address, state);
	int blkOffset = getBlkOffset(address, state);

	int way_to_write = alocateCacheLine(address, state);

	state->cacheArr[set][way_to_write].dirtyBit = clean;
	state->cacheArr[set][way_to_write].validBit = valid;
	state->cacheArr[set][way_to_write].tag = tag;
	state->cacheArr[set][way_to_write].cyclesSinceLastUse = 0;

	for(int i=0; i<state->wordsPerBlock; i++){
		state->cacheArr[set][way_to_write].data[i] = state->mem[getAddressBase(address, state) + i];
	}

	print_action(getAddressBase(address, state), state->wordsPerBlock, memory_to_cache);
	return way_to_write;
}

void cacheToMem(int address, stateType* state){
	int tag = getTag(address, state);
	int set = getSet(address, state);
	int blkOffset = getBlkOffset(address, state);

	// Get the way of the address
	int way_to_write = searchCache(address, state);

	// if the address is in cache and valid
	if (way_to_write != -1){
		// check if the way_to_write way needs to be written back to memory (is dirty)
		if (state->cacheArr[set][way_to_write].dirtyBit == dirty){
			// write each word in the block to memory
			for (int word = 0; word < state->wordsPerBlock; word++ ){
				state->mem[getAddressBase(address, state) + word] = state->cacheArr[set][way_to_write].data[word];
			}	
			print_action(address, state->wordsPerBlock, cache_to_memory);
		}
	}

	// make cache entry invalid
	state->cacheArr[set][way_to_write].validBit = invalid;

	print_action(address, state->wordsPerBlock, memory_to_cache);
}


	// * address is the starting word address of the range of data being transferred.
	// * size is the size of the range of data being transferred.
	// * type specifies the source and destination of the data being transferred.
	// *
	// * cache_to_processor: reading data from the cache to the processor
	// * processor_to_cache: writing data from the processor to the cache
	// * memory_to_cache: reading data from the memory to the cache
	// * cache_to_memory: evicting cache data by writing it to the memory
	// * cache_to_nowhere: evicting cache data by throwing it away
	// */
	// print_action(address, 1, action_type);
	// print_action(address, state->wordsPerBlock, cache_to_nowhere);

int cacheSystem(int address, stateType* state, enum access_type action, int write_value){
	incrementCyclesSinceLastUse(state);

	int tag = getTag(address, state);
	int set = getSet(address, state);
	int blkOffset = getBlkOffset(address, state);

	int whereInCache = searchCache(address, state);


	//processor read from mem
	if(action == read_mem){
		int readValue;
		// hit
		if(isHittOrMiss(whereInCache) == hit){
			// read hit
			readValue = state->cacheArr[set][whereInCache].data[blkOffset];
			printf("read hit\n");
		}
		// miss
		else{
			// read miss
			int blockWay = memToCache(address, state);
			readValue = state->cacheArr[set][blockWay].data[blkOffset];
			printf("read miss\n");
		}
		printCache(state); // REMOVE
		print_action(address, 1, cache_to_processor);
		return readValue;
	}

	//process write to mem
	else if(action == write_mem){
		if(isHittOrMiss(whereInCache) == hit){
			// write hit
			state->cacheArr[set][whereInCache].data[blkOffset] = write_value;
			printf("write hit\n");
		}
		else{
			// write miss
			int blockWay = memToCache(address, state);
			state->cacheArr[set][blockWay].data[blkOffset] = write_value;
			printf("write miss\n");
		}
		printCache(state); // REMOVE
		print_action(address, 1, processor_to_cache);
		return -1;
	}

	else{
		printf("Error in cache access enum\n");
		abort();
	}
}

int signExtend(int num){
	// convert a 16-bit number into a 32-bit integer
	if (num & (1<<15) ) {
		num -= (1<<16);
	}
	return num;
}

void run(stateType* state){

	// Reused variables;
	int instr = 0;
    int regA = 0;
	int regB = 0;
	int offset = 0;
	int branchTarget = 0;
	int aluResult = 0;

	int total_instrs = 0;

	// Primary loop
    while(1){
		total_instrs++;

		//printState(state);
		
		// Instruction Fetch
		// instr = state->mem[state->pc];
		instr = cacheSystem(state->pc, state, read_mem, -1);

		/* check for halt */
		if (opcode(instr) == HALT) {
		    printf("machine halted\n");
			break;
		}

		// Increment the PC
		state->pc = state->pc+1;
		
		// Set reg A and B
		regA = state->reg[field0(instr)];
		regB = state->reg[field1(instr)];

		// Set sign extended offset
		offset = signExtend(field2(instr));

		// Branch target gets set regardless of instruction
		branchTarget = state->pc + offset;

		/**
		 *
		 * Action depends on instruction
		 *
		 **/
		// ADD
		if(opcode(instr) == ADD){
			// Add
			aluResult = regA + regB;
			// Save result
			state->reg[field2(instr)] = aluResult;
		}
		// NAND
		else if(opcode(instr) == NAND){
			// NAND
			aluResult = ~(regA & regB);
			// Save result
			state->reg[field2(instr)] = aluResult;
		}
		// LW or SW
		else if(opcode(instr) == LW || opcode(instr) == SW){
			// Calculate memory address
			aluResult = regB + offset;
			if(opcode(instr) == LW){
				// Load
				// state->reg[field0(instr)] = state->mem[aluResult];
				state->reg[field0(instr)] = cacheSystem(aluResult, state, read_mem, -1);
			}else if(opcode(instr) == SW){
				// Store
				// state->mem[aluResult] = regA;
				cacheSystem(aluResult, state, write_mem, regA);
			}
		}
		// JALR
		else if(opcode(instr) == JALR){
			// rA != rB for JALR to work
			if(field0(instr) != field1(instr)){
				// Save pc+1 in regA
				state->reg[field0(instr)] = state->pc;
				//Jump to the address in regB;
				state->pc = state->reg[field1(instr)];
			}
		}
		// BEQ
		else if(opcode(instr) == BEQ){
			// Calculate condition
			aluResult = (regA == regB);

			// ZD
			if(aluResult){
				// branch
				state->pc = branchTarget;
			}
		}	
    } // While
	// print_stats(total_instrs);
}

int main(int argc, char** argv){

	/** Get command line arguments **/
    char* fname;

	opterr = 0;

	int cin = 0;
	int blockSizeInWords;
	int numSets;
	int associativity;
	int discriptiveFlag = 0;

	while((cin = getopt(argc, argv, "f:b:s:a:d")) != -1){
		switch(cin)
		{
			case 'f':
				fname=(char*)malloc(strlen(optarg));
				fname[0] = '\0';
				strncpy(fname, optarg, strlen(optarg)+1);
				break;
			case 'b':
				blockSizeInWords = atoi(optarg);
				break;
			case 's':
				numSets = atoi(optarg);
				
				break;
			case 'a':
				associativity = atoi(optarg);
				break;
			case 'd':
				printf("Got discriptiveFlag!");
				discriptiveFlag = 1;
				break;
			case '?':
				if(optopt == 'f'){
					printf("Option -%c requires an argument.\n", optopt);
				}
				else if(isprint(optopt)){
					printf("Unknown option `-%c'.\n", optopt);
				}
				else{
					printf("Unknown option character `\\x%x'.\n", optopt);
					return 1;
				}
				break;
			default:
				printf("aborting\n");
				abort();
		}
	}

	if (discriptiveFlag == 1){
		printf("File: %s\n", fname);
		printf("numSets: %d\n", numSets);
		printf("blockSizeInWords: %d\n", blockSizeInWords);
		printf("associativity: %d\n", associativity);
		printf("discriptiveFlag: %d\n", discriptiveFlag);
	}

	FILE *fp = fopen(fname, "r");
	if (fp == NULL) {
		printf("Cannot open file '%s' : %s\n", fname, strerror(errno));
		return -1;
	}
	
	/* count the number of lines by counting newline characters */
	int line_count = 0;
	int c;
    while (EOF != (c=getc(fp))) {
        if ( c == '\n' ){
            line_count++;
		}
    }
	// reset fp to the beginning of the file
	rewind(fp);

	// Instantiate the state and the cache 2d array and its structs
	stateType* state = (stateType*)malloc(sizeof(stateType));
	state->pc = 0;

	state->discriptiveFlag = discriptiveFlag;
	state->sets = numSets;
	state->ways = associativity;
	state->wordsPerBlock = blockSizeInWords;

	// Alocate our cache array the size of the number of sets/lines we have contaning pointers to the array of blocks in each set
	state->cacheArr = malloc(state->sets * sizeof(cacheEntryType*));

	// Alocate the size of the array of the sub arrays contaning the pointers to the block structs
	for (int set = 0; set < state->sets; set++ ){
		state->cacheArr[set] = (cacheEntryType*)malloc(state->ways * sizeof(cacheEntryType));

		// Alocate the structs pointed to by each pointer of the sub array
		for (int way = 0; way < state->ways; way++ ){
			state->cacheArr[set][way].dirtyBit = clean;
			state->cacheArr[set][way].validBit = invalid;
			state->cacheArr[set][way].tag = 0;
			state->cacheArr[set][way].cyclesSinceLastUse = 0;
			state->cacheArr[set][way].data = (int*)malloc(state->wordsPerBlock * sizeof(int));
			for (int word = 0; word < state->wordsPerBlock; word++ ){
				state->cacheArr[set][way].data[word] = 0;
			}
		}
	}

	printCache(state);

	memset(state->mem, 0, NUMMEMORY*sizeof(int));
	memset(state->reg, 0, NUMREGS*sizeof(int));

	state->numMemory = line_count;

	char line[256];
	
	int i = 0;
	while (fgets(line, sizeof(line), fp)) {
        /* note that fgets doesn't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */
		state->mem[i] = atoi(line);
		i++;
    }
    fclose(fp);
	
	/** Run the simulation **/
	run(state);

	free(state);
	free(fname);

}
