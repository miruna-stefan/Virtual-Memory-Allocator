// Stefan Miruna Andreea 314CA
#pragma once
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* useful macro for handling error codes */
#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);				        \
		}							\
	} while (0)

typedef  unsigned int uint;
typedef struct dll_node_t dll_node_t;
struct dll_node_t {
	void *data;
	dll_node_t *prev, *next;
};

typedef struct {
	dll_node_t *head;
	unsigned int data_size;
	unsigned int size;
} list_t;

typedef struct {
	uint64_t start_address;
	size_t size;
	uint8_t perm;
	void *rw_buffer;
} miniblock_t;

typedef struct {
	uint64_t start_address;
	size_t size;
	list_t *miniblock_list;
} block_t;

typedef struct {
	uint64_t arena_size;
	list_t *alloc_list;
} arena_t;

list_t *dll_create(unsigned int data_size);
static dll_node_t *create_node(const void *new_data, uint data_size);
dll_node_t *dll_get_nth_node(list_t *list, unsigned int n);
void dll_add_nth_node(list_t *list, uint n,
					  const void *new_data, uint data_size);
dll_node_t *dll_remove_nth_node(list_t *list, unsigned int n);
void dll_free(list_t **pp_list);

miniblock_t *create_miniblock(uint64_t start_address, size_t size);
block_t *create_block(uint64_t start_address, size_t size);

arena_t *alloc_arena(const uint64_t size);
void dealloc_arena(arena_t *arena);

int alloc_validity(arena_t *arena, const uint64_t address, const uint64_t size);
void no_neighbours_alloc(arena_t *arena, const uint64_t address,
						 const uint64_t size);
void left_neighbour_only_alloc(const uint64_t address,
							   const uint64_t size, dll_node_t *left_neighbour);
void right_neighbour_only_alloc(const uint64_t address, const uint64_t size,
								dll_node_t *right_neighbour);
void
two_neighbours_alloc(const uint64_t address, const uint64_t size,
					 dll_node_t *right_neighbour, dll_node_t *left_neighbour);
void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size);

void remove_block_with_only_one_miniblock(arena_t *arena, int cnt,
										  list_t *mini_list);
void free_first_or_last(dll_node_t *curr_block_node,
						list_t *mini_list, uint pos);
void free_middle_miniblock(arena_t *arena, dll_node_t *curr_block_node,
						   list_t *mini_list, uint pos, int cnt);
void free_block(arena_t *arena, const uint64_t address);

int verify_read_permission(dll_node_t *curr_mini);
void print_data(uint64_t reduced_size, int8_t **data);
void read(arena_t *arena, uint64_t address, uint64_t size);
int verify_write_permission(dll_node_t *curr_mini);
void write(arena_t *arena, const uint64_t address,
		   const uint64_t size, int8_t *data);

void pmap(const arena_t *arena);
void mprotect(arena_t *arena, uint64_t address, char *permission);
