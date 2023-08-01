#define _DEFAULT_SOURCE
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include "malloc.h"
#include "printfmt.h"


#define ALIGN4(s) (((((s) -1) >> 2) << 2) + 4)
#define REGION2PTR(r) ((r) + 1)
#define PTR2REGION(ptr) ((struct region *) (ptr) -1)

// APUNTAN AL PRIMER BLOQUE PEQUEÑO/MEDIANO/GRANDE (este bloque tiene sus respectiva
// regiones en una lista enlazada y a su vez tiene el bloque siguiente en una lista
// enlzada tambien) --> ver campos `next` (apunta a la siguiente region) y `next->block`
// apunta al siguiente bloque (solo las primeras regiones de cada bloque) o NULL para las regiones internas del bloque.
struct region *small_blocks_list = NULL;
struct region *medium_blocks_list = NULL;
struct region *big_blocks_list = NULL;

int amount_of_mallocs = 0;
int amount_of_frees = 0;
int requested_memory = 0;

int SIZE_OF_HEADER = sizeof(
        struct region);  // tamaño del header (contiene los metadatos de cada region)


// return an initialized region that represents a block of the size
// passed in block_size
static struct region *
get_new_block(size_t block_size)
{
	struct region *first_region = mmap(NULL,
	                                   block_size,
	                                   PROT_READ | PROT_WRITE,
	                                   MAP_PRIVATE | MAP_ANONYMOUS,
	                                   -1,
	                                   0);
	if (first_region == MAP_FAILED)
		return NULL;
	first_region->ptr =
	        (void *) first_region +
	        SIZE_OF_HEADER;     // el puntero debe saltearse el header
	first_region->free = true;  // la primera region esta libre
	first_region->next = NULL;
	first_region->size = block_size;
	first_region->next_block = NULL;
	return first_region;
}


// finds the next free region
// that holds the requested size
static struct region *
find_free_region(size_t size)
{
	// empiezo siempre por la lista de bloques pequeños
	int nro_de_lista = 0;
	// si el tamaño de la region a buscar es mas grande que la de un bloque
	// pequeño, podemos obviar esta lista (estamos seguros que no puede
	// estar ahi) y pasar a la de bloques medianos
	if (size >= SMALL_BLOCK_SIZE) {
		nro_de_lista = 1;
	}
	// si el tamaño de la region a buscar es mas grande que la de un
	// bloque mediano, podemos obviar esta lista (estamos seguros
	// que no puede estar ahi) y pasar a la de bloques grandes
	if (size >= MEDIUM_BLOCK_SIZE) {
		nro_de_lista = 2;
	}

#ifdef FIRST_FIT
	// mientras no se hayan terminado de recorrer las tres listas
	while (nro_de_lista < 3) {
		// veo en que lista estoy segun el nro_de_lista
		struct region *actual_list;
		switch (nro_de_lista) {
		case 0:
			actual_list = small_blocks_list;
			break;
		case 1:
			actual_list = medium_blocks_list;
			break;
		case 2:
			actual_list = big_blocks_list;
			break;
		}
		// como cada lista apunta al primer bloque de ese tamaño, el
		// primer bloque va a ser la lista actual (a donde apunte)
		struct region *actual_block = actual_list;
		// mientras haya un bloque en la lista de bloques
		while (actual_block != NULL) {
			// la primer region del bloque es lo mismo a lo que apunta el actual_block
			struct region *actual_region = actual_block;
			// mientras haya una region en el bloque
			while (actual_region != NULL) {
				// si la region tiene el tamaño pedido y esta libre
				if (actual_region->size > size &&
				    actual_region->free) {
					return actual_region;
				}
				// avanzo a la siguiente region
				actual_region = actual_region->next;
			}
			// avanzo al siguiente bloque
			actual_block = actual_block->next_block;
		}
		// termine de recorrer un tamaño de bloques, avanzo a la siguiente lista de bloques
		nro_de_lista++;
	}
	// no se encontro ninguna region libre, devuelve NULL
	return NULL;
#endif

#ifdef BEST_FIT
	// Your code here for "best fit"
	struct region *best_region = NULL;
	// mientras no se hayan terminado de recorrer las tres listas
	while (nro_de_lista < 3) {
		// veo en que lista estoy segun el nro_de_lista
		struct region *actual_list;
		switch (nro_de_lista) {
		case 0:
			actual_list = small_blocks_list;
			break;
		case 1:
			actual_list = medium_blocks_list;
			break;
		case 2:
			actual_list = big_blocks_list;
			break;
		}
		// como cada lista apunta al primer bloque de ese tamaño, el
		// primer bloque va a ser la lista actual (a donde apunte)
		struct region *actual_block = actual_list;
		// mientras haya un bloque en la lista de bloques
		while (actual_block != NULL) {
			// la primer region del bloque es lo mismo a lo que apunta el actual_block
			struct region *actual_region = actual_block;
			// mientras haya una region en el bloque
			while (actual_region != NULL) {
				// si la region tiene el tamaño pedido y esta libre
				if (actual_region->size > size &&
				    actual_region->free) {
					// si todavia no hay mejor region, la actual va a pasar a ser la mejor
					if (best_region == NULL) {
						best_region = actual_region;
					} else if (actual_region->size <
					           best_region->size) {
						best_region = actual_region;  // si el tamaño de la region es menor que el tamaño del mejor
					}
				}
				// avanzo a la siguiente region
				actual_region = actual_region->next;
			}
			// avanzo al siguiente bloque
			actual_block = actual_block->next_block;
		}
		// termine de recorrer bloque, avanzo a la siguiente lista de bloques
		nro_de_lista++;
	}
	return best_region;
#endif
	return NULL;
}


// asks for a new block and appends it to its corresponding list
// return NULL in case the maximum memory is passed or the list
// contains the maxium amount of blocks
static struct region *
grow_heap(size_t size_of_block)
{
	// si al agrandar el heap se supera la cantidad maxima de memoria pedida, se devuelve null
	if (requested_memory + size_of_block > MAX_MEMORY) {
		return NULL;  // el errno es manejado en la funcion que se le devuelve el NULL (malloc)
	}
	struct region *new_block =
	        get_new_block(size_of_block);  // pido nuevo bloque con mmap
	// me fijo a que lista corresponde el nuevo bloque
	switch (size_of_block) {
	case SMALL_BLOCK_SIZE:
		if (small_blocks_list == NULL) {
			small_blocks_list = new_block;
		} else {
			struct region *actual_block = small_blocks_list;
			size_t amount_of_blocks = 0;
			// recorro hasta encontrar el ultimo bloque en la lista
			while (actual_block->next_block != NULL) {
				actual_block = actual_block->next_block;
				amount_of_blocks++;
			}
			// si la lista de bloques supera la cantidad maxima de bloques chicos
			if (amount_of_blocks > MAX_AMOUNT_OF_SMALL_BLOCKS) {
				return NULL;  // el errno es manejado en la funcion que se le devuelve el NULL (malloc)
			}
			actual_block->next_block =
			        new_block;  // agrego el nuevo bloque al final de la lista
		}
		break;
	case MEDIUM_BLOCK_SIZE:
		if (medium_blocks_list == NULL) {
			medium_blocks_list = new_block;
		} else {
			struct region *actual_block = medium_blocks_list;
			size_t amount_of_blocks = 0;
			// recorro hasta encontrar el ultimo bloque en la lista
			while (actual_block->next_block != NULL) {
				actual_block = actual_block->next_block;
				amount_of_blocks++;
			}
			// si la lista de bloques supera la cantidad maxima de bloques medianos
			if (amount_of_blocks > MAX_AMOUNT_OF_MEDIUM_BLOCKS) {
				return NULL;  // el errno es manejado en la funcion que se le devuelve el NULL (malloc)
			}
			actual_block->next_block =
			        new_block;  // agrego el nuevo bloque al final de la lista
		}
		break;
	case BIG_BLOCK_SIZE:
		if (big_blocks_list == NULL) {
			big_blocks_list = new_block;
		} else {
			struct region *actual_block = big_blocks_list;
			size_t amount_of_blocks = 0;
			// recorro hasta encontrar el ultimo bloque en la lista
			while (actual_block->next_block != NULL) {
				actual_block = actual_block->next_block;
				amount_of_blocks++;
			}
			// si la lista de bloques supera la cantidad maxima de bloques grandes
			if (amount_of_blocks > MAX_AMOUNT_OF_BIG_BLOCKS) {
				return NULL;  // el errno es manejado en la funcion que se le devuelve el NULL (malloc)
			}
			actual_block->next_block =
			        new_block;  // agrego el nuevo bloque al final de la lista
		}
		break;
	}
	return new_block;
}

// returns the size of smallest block that can hold the required_size of the
// region or 0 in case the required size is < 0 or > BIG_BLOCK_SIZE
static size_t
get_size_of_block(size_t required_size)
{
	if (0 < required_size && required_size < SMALL_BLOCK_SIZE) {
		return SMALL_BLOCK_SIZE;
	} else if (SMALL_BLOCK_SIZE <= required_size &&
	           required_size < MEDIUM_BLOCK_SIZE) {
		return MEDIUM_BLOCK_SIZE;
	} else if (MEDIUM_BLOCK_SIZE <= required_size &&
	           required_size < BIG_BLOCK_SIZE) {
		return BIG_BLOCK_SIZE;
	}
	return 0;
}


// returns the block that cointains the region and updates the
// list_of_region_ptr (passed as parameter) to the list that cointains the
// region. Return NULL in case the region is not found
static struct region *
find_list_and_block_of_region(void *ptr, struct region **list_of_region_ptr)
{
	struct region *region = PTR2REGION(ptr);
	size_t size_of_block = get_size_of_block(region->size);
	int list_to_check = 0;
	switch (size_of_block) {
	case MEDIUM_BLOCK_SIZE:
		list_to_check = 1;
		break;
	case BIG_BLOCK_SIZE:
		list_to_check = 2;
		break;
	}
	// recorro las listas de bloques
	while (list_to_check < 3) {
		struct region *actual_list = NULL;
		switch (list_to_check) {
		case 0:
			actual_list = small_blocks_list;
			break;
		case 1:
			actual_list = medium_blocks_list;
			break;
		case 2:
			actual_list = big_blocks_list;
			break;
		}
		// recorro los bloques de la lista
		struct region *actual_block = actual_list;
		while (actual_block != NULL) {
			struct region *actual_region = actual_block;
			// recorro las regiones del bloque
			while (actual_region != NULL) {
				// si la region es igual a la region que se quiere liberar
				if (REGION2PTR(actual_region) == ptr) {
					// actualiza el puntero pasado a la funcion con la lista donde esta la region
					*list_of_region_ptr = actual_list;
					// devuelvo el bloque en donde esta la region a ser liberada
					return actual_block;
				}
				actual_region = actual_region->next;
			}
			actual_block = actual_block->next_block;
		}
		list_to_check++;
	}
	return NULL;
}


// Recibe la lista en donde se encuentra un bloque y se fija
// si el bloque entero esta libre (contiene una unica region que engloba al bloque entero)
// ,devuelve la memoria al sistema y actualiza la lista correspondiente
static void
free_entire_block(struct region *list_of_region)
{
	// actual_block = primer bloque de la lista
	struct region *actual_block = list_of_region;
	// si es el primer bloque de la lista y esta vacio
	if (actual_block->next == NULL && actual_block->free) {
		// el inicio de la lista apunta al siguiente bloque (NULL si no hay mas bloques)
		list_of_region = actual_block->next_block;
		// se libera el bloque
		munmap(actual_block, actual_block->size);
		return;
	}
	// mientras el siguiente bloque no sea nulo
	while (actual_block->next_block != NULL) {
		// si la primera region del siguiente bloque no tiene siguiente region
		// (es una sola region que ocupa todo el bloque) y ademas esta libre
		if (actual_block->next_block->next == NULL &&
		    actual_block->next_block->free) {
			struct region *new_next_block_in_list =
			        actual_block->next_block->next_block;
			// se libera bloque
			munmap(actual_block->next_block,
			       actual_block->next_block->size);
			// se saca el siguiente del bloque actual y se lo actualiza con el siguiente del siguiente
			actual_block->next_block = new_next_block_in_list;
		}
		actual_block = actual_block->next_block;
	}
}

void *
malloc(size_t size)
{
	if (size > 0 && size < MIN_SIZE) {
		size = MIN_SIZE;  // si el tamaño pedido es menor que el minimo de una region, devolvemos el minimo
	}
	size_t size_of_block = get_size_of_block(size);
	// si el tamaño pedido es mayor al tamaño del bloque mas grande o es 0
	if (!size_of_block) {
		errno = ENOMEM;
		return NULL;
	}

	struct region *requested_memory_region;

	// aligns to multiple of 4 bytes
	size = ALIGN4(size);

	requested_memory_region = find_free_region(size);
	// si no encuentra regiones libres o es la primera vez que se hace malloc
	if (!requested_memory_region) {
		requested_memory_region =
		        grow_heap(size_of_block);  // seria pedir otro bloque
		if (!requested_memory_region) {  // significa que se alcanzo la
			                         // cantidad maxima de memoria o bloques en una lista
			errno = ENOMEM;
			return NULL;
		}
	}
	// la region libre encontrada pasa a estar ocupada
	requested_memory_region->free = false;

	// SPLIT FREE REGIONS
	// si la region que sobra es mayor al minimo tamaño por el cual vale la pena dividirla
	if (requested_memory_region->size - size >= MIN_SIZE) {
		struct region *new = (void *) requested_memory_region + size;
		new->free = true;
		new->size =
		        requested_memory_region->size -
		        size;  // si tenemos en cuenta que el size incluye el HEADER
		new->ptr = requested_memory_region->ptr + size;
		new->next = requested_memory_region->next;
		new->next_block = NULL;
		requested_memory_region->next = new;
		requested_memory_region->size = size;
	}
	// updates statistics
	amount_of_mallocs++;
	requested_memory += requested_memory_region->size;
	return REGION2PTR(requested_memory_region);
}

void
free(void *ptr)
{
	if (ptr == NULL) {
		return;  // si se hace free(NULL) no hace nada (no hace falta settear errno = ENOMEM)
	}

	// un puntero en donde voy a guardar la lista en donde se encuentra la region a ser liberada
	struct region *list_of_region = NULL;
	struct region *block =
	        find_list_and_block_of_region(ptr, &list_of_region);
	if (!block) {
		return;  // no se encontro la region --> invalid free/ double free (comportamiento no definido)
	}
	struct region *curr =
	        PTR2REGION(ptr);  // obtengo el puntero a la region a ser liberada
	curr->free = true;        // la region pasa a estar libre
	size_t freed_size = curr->size;

	// COALESCE REGIONS
	struct region *act = block;
	// recorro las regiones del bloque en donde se encuentra la region a ser liberada
	while (act != NULL) {
		// si hay dos regiones seguidas libres se hace coalescing
		if (act->next != NULL && act->free && act->next->free) {
			act->size += act->next->size;  // se suman los tamaños
			act->next =
			        act->next->next;  // el actual apunta al siguiente del siguiente
		}
		act = act->next;
	}
	// ver si hay que liberar el bloque
	free_entire_block(list_of_region);


	// updates statistics
	amount_of_frees++;
	requested_memory = requested_memory - freed_size;
}

void *
calloc(size_t nmemb, size_t size)
{
	size_t total_size = nmemb * size;
	void *ptr = malloc(total_size);
	if (ptr == NULL) {
		return NULL;
	}
	memset(ptr, 0, total_size);
	return ptr;
}

void *
realloc(void *ptr, size_t size)
{
	// Si ptr es igual a NULL, el comportamiento es igual a malloc(size)
	if (ptr == NULL) {
		return malloc(size);
	}
	// Si size es igual a cero (y ptr no es NULL) debería ser equivalente a free(ptr)
	if (size == 0) {
		free(ptr);
		return NULL;
	}
	// se verifica que la region haya sido pedida con malloc previamente
	struct region *list_of_region = NULL;
	struct region *block_of_region =
	        find_list_and_block_of_region(ptr, &list_of_region);
	if (!block_of_region) {
		errno = ENOMEM;
		return NULL;
	}
	struct region *region = PTR2REGION(ptr);
	size_t size_of_block = get_size_of_block(size);
	// si el tamaño es mayor al bloque grande o es 0
	if (size_of_block == 0) {
		errno = ENOMEM;
		return NULL;
	}
	if (size <= MIN_SIZE) {
		size = MIN_SIZE;
	}
	// si el tamaño es igual al tamaño de la region devuelve la misma
	if (size == region->size) {
		return ptr;
	}
	// hay que achicar la region?
	if (size < region->size) {
		// Se puede hacer splitting?
		if (region->size - size >= MIN_SIZE) {
			struct region *new = (void *) region + size;
			new->free = true;
			new->size = region->size - size;
			new->ptr = region->ptr + size;
			new->next = region->next;
			new->next_block = NULL;
			region->next = new;
			region->size = size;
			requested_memory -= new->size;
			// COALESCE REGIONS IN BLOCK OF ORIGINAL REGION ?
			struct region *act = block_of_region;
			// recorro las regiones del bloque en donde se encuentra la region a ser liberada
			while (act != NULL) {
				// si hay dos regiones seguidas libres se hace coalescing
				if (act->next != NULL && act->free &&
				    act->next->free) {
					act->size +=
					        act->next->size;  // se suman los tamaños
					act->next =
					        act->next->next;  // el actual apunta al siguiente del siguiente
				}
				act = act->next;
			}
		}
		return ptr;
	}
	// hay que agrandar la region
	// me fijo si juntando las dos regiones entra el tamaño pedido
	if (region->next && region->next->free &&
	    region->next->size + region->size >= size) {
		requested_memory += size - region->size;
		region->size += region->next->size;
		region->next->size -= size;
		region->next->ptr = region->ptr + size;
		return ptr;
	}


	// la siguiente region no esta libre por lo que se debe copiar la
	// informacion en otra region del tamaño adecuado y devolver esta nueva
	// region
	void *new_ptr = malloc(size);
	if (new_ptr == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	memcpy(new_ptr, ptr, region->size);
	free(ptr);
	return new_ptr;
}

void
get_stats(struct malloc_stats *stats)
{
	stats->mallocs = amount_of_mallocs;
	stats->frees = amount_of_frees;
	stats->requested_memory = requested_memory;
}
