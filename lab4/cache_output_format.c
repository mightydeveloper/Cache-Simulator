#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define BYTES_PER_WORD 4
#define MAX_BLOCK_WORD 8

typedef struct line {
    bool valid;
    uint32_t content[MAX_BLOCK_WORD];   // tag bit + index bit (block offset masked by 0)
    int used;                           // used count for LRU
} line;




/***************************************************************/
/*                                                             */
/* Procedure : cdump                                           */
/*                                                             */
/* Purpose   : Dump cache configuration                        */   
/*                                                             */
/***************************************************************/
void cdump(int capacity, int assoc, int blocksize){

	printf("Cache Configuration:\n");
    printf("-------------------------------------\n");
	printf("Capacity: %dB\n", capacity);
	printf("Associativity: %dway\n", assoc);
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
void sdump(int total_reads, int total_writes, int write_backs,
	int reads_hits, int write_hits, int reads_misses, int write_misses) {
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
void xdump(int set, int way, int words, line** cache)
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
	int capacity = 256;                     // capacity in Bytes
	int way = 4;                            // associativity
	int blocksize = 8;                      // block size in Bytes
	int set = capacity/way/blocksize;       // index
	int words = blocksize / BYTES_PER_WORD;	// #of words per block.

	// allocate
    cache = (line**) malloc(sizeof(line*) * set);
    for(i = 0; i < set; i++) {
        cache[i] = (struct line*) malloc(sizeof(line) * way);
    }
    
    // init
	for(i = 0; i < set; i++) {
        for(j = 0; j < way; j ++) {
            for (k = 0; k < words; k++) {
                cache[i][j].content[k] = 0x0;
            }
        }
	}

	// test example
    cdump(capacity, way, blocksize);
    sdump(0, 0, 0, 0, 0, 0, 0);
    xdump(set, way, words, cache);

    return 0;
}
