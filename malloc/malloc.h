#ifndef _MALLOC_H_
#define _MALLOC_H_

#define MIN_SIZE 256
#define SMALL_BLOCK_SIZE 16384
#define MEDIUM_BLOCK_SIZE 1048576
#define BIG_BLOCK_SIZE 33554432
#define MAX_MEMORY 2000000000
// CANTIDAD ARBITRARIA MAXIMA DE BLOQUES DE CADA TIPO
// 2000000000 --> 33.3% ~= 666000000
#define MAX_AMOUNT_OF_SMALL_BLOCKS                                             \
	40649  // 666000000 / SMALL_BLOCK_SIZE ~= 40649
#define MAX_AMOUNT_OF_MEDIUM_BLOCKS 635  // 666000000 / MEDIUM_BLOCK_SIZE ~= 635
#define MAX_AMOUNT_OF_BIG_BLOCKS 19      // 666000000 / BIG_BLOCK_SIZE ~= 19

#include <stdbool.h>

struct malloc_stats {
	int mallocs;
	int frees;
	int requested_memory;
};

struct region {
	bool free;    // true si region esta vacia, false si esta ocupada
	size_t size;  // tamaño de la region
	struct region *next;        // puntero a proxima region en el bloque
	struct region *next_block;  // puntero a proximo bloque
	void *ptr;                  // puntero a una region del bloque
};

void *malloc(size_t size);

void free(void *ptr);

void *calloc(size_t nmemb, size_t size);

void *realloc(void *ptr, size_t size);

void get_stats(struct malloc_stats *stats);

#endif  // _MALLOC_H_
