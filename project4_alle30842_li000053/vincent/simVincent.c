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

#define WORDSIZE 32

#define NOOPINSTRUCTION 0x1c00000

enum dirty_bit {dirty, clean};
enum valid_bit {valid, invalid};

typedef struct blockStruct {
    enum dirty_bit dirtyBit;
	enum valid_bit validBit;
	int tag;
	int *data;
} blockType;

typedef struct stateStruct {
    int pc;
	int mem[NUMMEMORY];
	int reg[NUMREGS];
	int numMemory;
	blockType **cacheArr;
	int sets;
	int ways;
	int wordsPerBlock;
} stateType;


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

int tag(int address, stateType* state){
	int words_per_blk = state->wordsPerBlock;
	int num_of_set = state->sets;

	int bits_blkOffset = log(words_per_blk)/log(2);
	int bits_set = log(num_of_set)/log(2);

	int tag = address >> (bits_blkOffset + bits_set);

	return tag;
}

int set(int address, stateType* state){
	int num_of_set = state->sets;
	int bits_needed = log(num_of_set) / log(2);
	int mask = 0;



	for(int i=0; i<bits_needed ;i++){
		mask += pow(2,i);
	}

	int set = (address & mask);

	return set;
}

int blkOffset(int address, stateType* state){
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
enum action_type {cache_to_processor, processor_to_cache, memory_to_cache, cache_to_memory,
cache_to_nowhere};

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


enum access_type {read_mem, write_mem};

int cacheSystem(int address, stateType* state, enum access_type action){
	state->cacheArr = 0; // change this line

	return 1;
}


int alocateCacheLine(int address, stateType* state, enum access_type action){
	state->cacheArr = 0; // change this line

	return 1;
}



// void printInstruction(int instr){
//     char opcodeString[10];
//     if (opcode(instr) == ADD) {
// 		strcpy(opcodeString, "add");
//     } else if (opcode(instr) == NAND) {
// 		strcpy(opcodeString, "nand");
//     } else if (opcode(instr) == LW) {
// 		strcpy(opcodeString, "lw");
//     } else if (opcode(instr) == SW) {
// 		strcpy(opcodeString, "sw");
//     } else if (opcode(instr) == BEQ) {
// 		strcpy(opcodeString, "beq");
//     } else if (opcode(instr) == JALR) {
// 		strcpy(opcodeString, "jalr");
//     } else if (opcode(instr) == HALT) {
// 		strcpy(opcodeString, "halt");
//     } else if (opcode(instr) == NOOP) {
// 		strcpy(opcodeString, "noop");
//     } else {
// 		strcpy(opcodeString, "data");
//     }

//     printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
// 	field2(instr));
// }

// void printState(stateType *statePtr){
//     int i;
// 	printf("\n@@@\nstate:\n");
// 	printf("\tpc %d\n", statePtr->pc);
// 	printf("\tmemory:\n");
// 	for(i = 0; i < statePtr->numMemory; i++){
// 		printf("\t\tmem[%d]=%d\n", i, statePtr->mem[i]);
// 	}	
// 	printf("\tregisters:\n");
// 	for(i = 0; i < NUMREGS; i++){
// 		printf("\t\treg[%d]=%d\n", i, statePtr->reg[i]);
// 	}
// 	printf("end state\n");
// }

int signExtend(int num){
	// convert a 16-bit number into a 32-bit integer
	if (num & (1<<15) ) {
		num -= (1<<16);
	}
	return num;
}

// void print_stats(int n_instrs){
// 	printf("INSTRUCTIONS: %d\n", n_instrs);
// }

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
		instr = state->mem[state->pc];

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
				state->reg[field0(instr)] = state->mem[aluResult];
			}else if(opcode(instr) == SW){
				// Store
				state->mem[aluResult] = regA;
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

	while((cin = getopt(argc, argv, "f:b:s:a:")) != -1){
		switch(cin)
		{
			case 'f':
				fname=(char*)malloc(strlen(optarg));
				fname[0] = '\0';

				strncpy(fname, optarg, strlen(optarg)+1);
				printf("File: %s\n", fname);
				break;
			case 'b':
				blockSizeInWords = *optarg - '0';
				printf("blockSizeInWords: %d\n", blockSizeInWords);
				// printf("blockSizeInWords: %s\n", blockSizeInWords);
				break;
			case 's':
				numSets = *optarg - '0';
				printf("numSets: %d\n", numSets);
				// printf("numSets: %s\n", numSets);
				break;
			case 'a':
				associativity = *optarg - '0';
				printf("associativity: %d\n", associativity);
				// printf("associativity: %s\n", associativity);
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

	/*
	if(argc == 1){
		fname = (char*)malloc(sizeof(char)*100);
		printf("Enter the name of the machine code file to simulate: ");
		fgets(fname, 100, stdin);
		fname[strlen(fname)-1] = '\0'; // gobble up the \n with a \0
	}
	else if (argc == 2){
        
	    int strsize = strlen(argv[1]);

		fname = (char*)malloc(strsize);
		fname[0] = '\0';

		strcat(fname, argv[1]);
	}else{
		printf("Please run this program correctly\n");
		exit(-1);
	}
	*/

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


	state->sets = numSets;
	state->ways = associativity;
	state->wordsPerBlock = blockSizeInWords;


	// Alocate our cache array the size of the number of sets/lines we have contaning pointers to the array of blocks in each set
	state->cacheArr = malloc(state->sets * sizeof(blockType*));

	// Alocate the size of the array of the sub arrays contaning the pointers to the block structs
	for (int i = 0; i < state->sets; i++ ){
		state->cacheArr[i] = (blockType*)malloc(state->ways * sizeof(blockType*));

		// Alocate the structs pointed to by each pointer of the sub array
		for (int k = 0; k < state->ways; k++ ){
			blockType block;
			block.dirtyBit = clean;
			block.validBit = invalid;
			block.data = (int*)malloc(state->wordsPerBlock * sizeof(int));
			for (int l = 0; l < state->wordsPerBlock; l++ ){
				block.data[l] = 0;
			}
			state->cacheArr[i][k] = block;
		}
	}


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
