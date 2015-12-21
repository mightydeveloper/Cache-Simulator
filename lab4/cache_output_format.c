#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define BYTES_PER_WORD 4
#define MAX_BLOCK_WORD 8

typedef struct line {
    bool valid;
    uint32_t content[MAX_BLOCK_WORD];   // tag bit + index bit (block offset masked by 0)
    int used;                           // used count for LRU
} line;


// statistics variables
int total_reads = 0;
int total_writes = 0;
int write_backs = 0;
int reads_hits = 0;
int write_hits = 0;
int reads_misses = 0;
int write_misses = 0;

// global variables
int capacity = 256;                     // capacity in Bytes
int way = 4;                            // associativity
int blocksize = 8;                      // block size in Bytes
int set = 8;                            // index
int words = 1;                          // #of words per block.

unsigned int usedcount = 0;

// functions
void cdump();
void sdump();
void xdump(line** cache);
void checkhit(int32_t address, line** cache, bool write);
int blockoffset(int blocksize);
int calcindexbits();
void sample_input2_hardcode(line** cache);
void readfile(char *filename, line** cache);

// utility functions
char** str_split(char *a_str, const char a_delim);




/***************************************************************/
/*                                                             */
/* Procedure : cdump                                           */
/*                                                             */
/* Purpose   : Dump cache configuration                        */   
/*                                                             */
/***************************************************************/
void cdump(){

	printf("Cache Configuration:\n");
    printf("-------------------------------------\n");
	printf("Capacity: %dB\n", capacity);
	printf("Associativity: %dway\n", way);
	printf("Block Size: %dB\n", blocksize);
	printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : sdump                                           */
/*                                                             */
/* Purpose   : Dump cache stat                                 */
/*                                                             */
/***************************************************************/
void sdump() {
	printf("Cache Stat:\n");
    printf("-------------------------------------\n");
	printf("Total reads: %d\n", total_reads);
	printf("Total writes: %d\n", total_writes);
	printf("Write-backs: %d\n", write_backs);
	printf("Read hits: %d\n", reads_hits);
	printf("Write hits: %d\n", write_hits);
	printf("Read misses: %d\n", reads_misses);
	printf("Write misses: %d\n", write_misses);
	printf("\n");
}


/***************************************************************/
/*                                                             */
/* Procedure : xdump                                           */
/*                                                             */
/* Purpose   : Dump current cache state                        */ 
/*                                                             */
/* Cache Design                                                */
/*                                                             */
/* 	    cache[set][assoc][word per block]                      */
/*                                                             */
/*                                                             */
/*       ----------------------------------------              */
/*       I        I  way0  I  way1  I  way2  I                 */
/*       ----------------------------------------              */
/*       I        I  word0 I  word0 I  word0 I                 */
/*       I  set0  I  word1 I  word1 I  work1 I                 */
/*       I        I  word2 I  word2 I  word2 I                 */
/*       I        I  word3 I  word3 I  word3 I                 */
/*       ----------------------------------------              */
/*       I        I  word0 I  word0 I  word0 I                 */
/*       I  set1  I  word1 I  word1 I  work1 I                 */
/*       I        I  word2 I  word2 I  word2 I                 */
/*       I        I  word3 I  word3 I  word3 I                 */
/*       ----------------------------------------              */
/*                                                             */
/*                                                             */
/***************************************************************/
void xdump(line** cache)
{
	int i,j,k = 0;

	printf("Cache Content:\n");
    printf("-------------------------------------\n");
	for(i = 0; i < way;i++)
	{
		if(i == 0)
		{
			printf("          ");
		}
		printf("WAY[%d]      ",i);
        for (k = 0; k < words-1; k++) {
            printf("            ");
        }
	}
	printf("\n");

	for(i = 0 ; i < set;i++)
	{
		printf("SET[%d]:   ",i);
		for(j = 0; j < way;j++)
		{
            for (k = 0; k < words; k++) {
                printf("0x%08x  ", cache[i][j].content[k]);
            }
			
		}
		printf("\n");
	}
	printf("\n");
}




int main(int argc, char *argv[]) {                              
    
	line** cache;
	int i, j, k;	
	
    capacity = 256;                     // capacity in Bytes
    way = 4;                            // associativity
    blocksize = 8;                      // block size in Bytes
    set = capacity/way/blocksize;       // index
    words = blocksize / BYTES_PER_WORD;	// #of words per block.
    
    // input argument parsing
    if (argc < 2)
    {
        printf("Error: usage: %s -c cap:assoc:bsize [-x] input_trace\n", argv[0]);
        exit(1);
    }
    int count = 1;
    char** tokens;
    bool dumpcachecontent = false;
    bool c_option_provided = false;
    
    while (count != argc-1) {
        if (strcmp(argv[count], "-c") == 0) {
            c_option_provided = true;
            tokens = str_split(argv[++count],':');
            capacity = (int)strtol(*(tokens), NULL, 10);
            way = (int)strtol(*(tokens+1), NULL, 10);
            blocksize = (int)strtol(*(tokens+2), NULL, 10);
            
            set = capacity/way/blocksize;
            words = blocksize / BYTES_PER_WORD;
        }
        
        else if(strcmp(argv[count], "-x") == 0) {
            dumpcachecontent = true;
        } else {
            printf("Error: usage: %s -c cap:assoc:bsize [-x] input_trace\n", argv[0]);
            exit(1);
        }
        count++;
    }
    
    if (!c_option_provided) {
        printf("Error: -c option must be provided!\n");
        printf("Usage: %s -c cap:assoc:bsize [-x] input_trace\n", argv[0]);
        exit(1);
    }
    
    

	// allocate
    cache = (line**) malloc(sizeof(line*) * set);
    for(i = 0; i < set; i++) {
        cache[i] = (struct line*) malloc(sizeof(line) * way);
    }
    
    // initialize
	for(i = 0; i < set; i++) {
        for(j = 0; j < way; j ++) {
            for (k = 0; k < words; k++) {
                cache[i][j].content[k] = 0x0;
                cache[i][j].valid = false;
                cache[i][j].used = UINT32_MAX;
            }
        }
	}
    // run
    //sample_input2_hardcode(cache);
    
    readfile(argv[argc-1], cache);
    

	// test example
    cdump();
    sdump();
    if(dumpcachecontent) xdump(cache);

    return 0;
}



void readfile(char *filename, line** cache) {
    FILE *inputfile;
    inputfile = fopen(filename, "r");
    
    if (inputfile == NULL) {
        printf("Error: %s file is not found", filename);
        exit(1);
    }
    
    
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    
    while ((read = getline(&line, &len, inputfile)) != -1) {
        bool writemode = true;
        if (line[0] == 'R') {
            writemode = false;
        } else if (line[0] == 'W') {
            writemode = true;
        } else {
            printf("Wrong input : %s", line);
            exit(1);
        }
    
        int32_t addr = (int32_t) strtol(line+2,NULL,0);
        
        checkhit(addr, cache, writemode);
    }
}







void checkhit(int32_t address, line** cache, bool write) { // write -> true, read -> false
    // count totals
    if (write) total_writes++;
    else total_reads++;
    
    // calculate index location, block offset bit
#define BYTEOFFSET 2
    int blockoffsetbit = (address & 0XFFFFFFF0) >> BYTEOFFSET;
    int index = (address & 0XFFFFFFF0) >> (blockoffset(blocksize)+BYTEOFFSET);
    unsigned int mask = 0;
    for (unsigned i=0; i < calcindexbits(); i++) {
        mask |= 1 << i;
    }
    index = index & mask; // extract index bits
    
    unsigned int mask2 = 0;
    for (unsigned i=0; i < words; i++) {
        mask2 |= 1 << i;
    }
    blockoffsetbit = blockoffsetbit & mask2; // extract block offset bits
    // end of index, block offset calcuation
    

    
    
    //cache[set][assoc][word per block]
    int i;
    bool hit = false;
    for (i=0; i<way; i++) {
        if (cache[index][i].content[blockoffsetbit] == (address&0XFFFFFFF0) && cache[index][i].valid) {
            // Hit!!
            hit = true;
            cache[index][i].used = usedcount++;
            
            if (write) write_hits++;
            else reads_hits++;
            break;
        }
    }
    if (!hit) {
        // Miss!!
        if (write) write_misses++;
        else reads_misses++;
        
        // Miss handling
        // find LRU index
        int LRUindex = 0;
        bool evict = true;
        unsigned int LRUcount = UINT32_MAX;
        for (i=0; i<way; i++) {
            if (!cache[index][i].valid) { // empty spot found
                LRUindex = i;
                evict = false;
                break;
            }
            
            if (cache[index][i].used < LRUcount) { // found 'less' recently used line
                LRUindex = i;
                LRUcount = cache[index][i].used;
            }
        }
        if (evict && write) { // write-back
            write_backs++;
        }
        
        
        // fill in the contents of cache
        cache[index][LRUindex].valid = true;
    
        unsigned int mask3 = 0;
        for (unsigned i=BYTEOFFSET; i <BYTEOFFSET+words; i++) {
            mask3 |= 1 << i;
        }
        
        for (i=0; i<words; i++) { // update whole chunk of block words.
            
            cache[index][LRUindex].content[i] = ((address&0XFFFFFFF0)&(~mask3)) | (i<<2);
        }
        cache[index][LRUindex].used = usedcount++;
        
    }
    
    
    
}




int blockoffset(int blocksize) {
    switch (blocksize) {
        case 4:
            return 0;
            break;
        case 8:
            return 1;
            break;
        case 16:
            return 2;
            break;
        case 32:
            return 3;
            break;
        default:
            printf("Wrong blocksize value : %d\n", blocksize);
            break;
    }
    return 0;
}

int calcindexbits() {
    int num = capacity/way/blocksize;
    int r = 0;
    while (num >>= 1)
    {
        r++;
    }
    return r;       // r = log_2(num)
}





void sample_input2_hardcode(line** cache) {
    
    checkhit(0x10001000, cache, false);
    checkhit(0x10001020, cache, false);
    checkhit(0x10001040, cache, false);
    checkhit(0x10001060, cache, false);
    checkhit(0x10001000, cache, false);
    checkhit(0x10001040, cache, false);
    checkhit(0x10001080, cache, false);
    checkhit(0x100010a0, cache, false);
    checkhit(0x10001004, cache, true);
    checkhit(0x10001024, cache, true);
    checkhit(0x10001044, cache, true);
    checkhit(0x10001064, cache, true);
    checkhit(0x10001024, cache, false);
    checkhit(0x10001064, cache, false);
    checkhit(0x10001084, cache, false);
    checkhit(0x100010a4, cache, false);
    checkhit(0x10001025, cache, true);
    checkhit(0x10001026, cache, true);
    checkhit(0x10001027, cache, true);

}












//  Utility function

char** str_split(char *a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;
    
    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }
    
    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);
    
    /* Add space for terminating null string so caller
     *        knows where the list of returned strings ends. */
    count++;
    
    result = malloc(sizeof(char*) * count);
    
    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);
        
        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }
    
    return result;
}



