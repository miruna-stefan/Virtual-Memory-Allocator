// Stefan Miruna Andreea 314CA
#include "vma.h"

#define NMAX 1000
#define MAX_DUMMY 20
#define MPROTECT_MAX 40

// function that checks if there is still a parameter to be read
int verify_parameter_existence(void)
{
	char ch;
	scanf("%c", &ch);

	if (ch == '\n') {
		// the command is not followed by a parameter
		return 0;
	}

	// there is still at least one parameter to be read
	return 1;
}

/* function that calls the "alloc_arena" function only
after checking if the command is valid */
arena_t *safe_alloc_arena_command(void)
{
	//check if the fisrt parameter exists
	if (verify_parameter_existence() == 0) {
		printf("Invalid command. Please try again.\n");
		return NULL;
	}

	uint64_t arena_size;
	scanf("%lu", &arena_size);

	arena_t *arena = alloc_arena(arena_size);
	return arena;
}

/* function that calls the "alloc_block" function only
after checking if the command is valid */
void safe_alloc_block_command(arena_t *arena)
{
	uint64_t address;
	size_t size;

	//check if the fisrt parameter exists
	if (verify_parameter_existence() == 0) {
		printf("Invalid command. Please try again.\n");
		return;
	}

	scanf("%lu", &address);

	//check if the second parameter exists
	if (verify_parameter_existence() == 0) {
		printf("Invalid command. Please try again.\n");
		return;
	}

	scanf("%lu", &size);

	alloc_block(arena, address, size);
}

/* function that calls the "free_block" function only
after checking if the command is valid */
void safe_free_block_command(arena_t *arena)
{
	uint64_t address;

	//check if the fisrt parameter exists
	if (verify_parameter_existence() == 0) {
		printf("Invalid command. Please try again.\n");
		return;
	}

	scanf("%lu", &address);

	free_block(arena, address);
}

/* function that calls the "write" function only
after checking if the command is valid */
void safe_write_command(arena_t *arena)
{
	uint64_t address;
	size_t size;

	//check if the fisrt parameter exists
	if (verify_parameter_existence() == 0) {
		printf("Invalid command. Please try again.\n");
		return;
	}

	scanf("%lu", &address);

	//check if the second parameter exists
	if (verify_parameter_existence() == 0) {
		printf("Invalid command. Please try again.\n");
		return;
	}

	scanf("%lu", &size);

	//check if the third parameter exists
	if (verify_parameter_existence() == 0) {
		printf("Invalid command. Please try again.\n");
		return;
	}

	/* read the string (character by character) that needs to
	be written in the rw_buffers at a specific address */
	int8_t *data = malloc(size * sizeof(int8_t));
	// defensive programming
	DIE(NULL == data, "malloc failed\n");

	for (size_t i = 0; i < size; i++)
		scanf("%c", &data[i]);

	write(arena, address, size, data);

	free(data);
}

/* function that calls the "read" function only
after checking if the command is valid */
void safe_read_command(arena_t *arena)
{
	uint64_t address;
	size_t size;

	//check if the fisrt parameter exists
	if (verify_parameter_existence() == 0) {
		printf("Invalid command. Please try again.\n");
		return;
	}

	scanf("%lu", &address);

	//check if the second parameter exists
	if (verify_parameter_existence() == 0) {
		printf("Invalid command. Please try again.\n");
		return;
	}

	scanf("%lu", &size);

	read(arena, address, size);
}

/* function that calls the "mprotect" function only
after checking if the command is valid */
void safe_mprotect_command(arena_t *arena)
{
	//check if the fisrt parameter exists
	if (verify_parameter_existence() == 0) {
		printf("Invalid command. Please try again.\n");
		return;
	}

	uint64_t address;
	scanf("%lu", &address);

	//check if the second parameter exists
	if (verify_parameter_existence() == 0) {
		printf("Invalid command. Please try again.\n");
		return;
	}

	//read permission
	char new_permission[MPROTECT_MAX];
	fgets(new_permission, MPROTECT_MAX, stdin);
	mprotect(arena, address, new_permission);
}

int main(void)
{
	char command[NMAX];
	arena_t *arena;

	while (1) {
		scanf("%s", command);
		if (strcmp(command, "ALLOC_ARENA") == 0) {
			arena = safe_alloc_arena_command();
			continue;
		}

		if (strcmp(command, "ALLOC_BLOCK") == 0) {
			safe_alloc_block_command(arena);
			continue;
		}

		if (strcmp(command, "PMAP") == 0) {
			pmap(arena);
			continue;
		}

		if (strcmp(command, "FREE_BLOCK") == 0) {
			safe_free_block_command(arena);
			continue;
		}

		if (strcmp(command, "DEALLOC_ARENA") == 0) {
			dealloc_arena(arena);
			break;
		}

		if (strcmp(command, "WRITE") == 0) {
			safe_write_command(arena);
			continue;
		}

		if (strcmp(command, "READ") == 0) {
			safe_read_command(arena);
			continue;
		}

		if (strcmp(command, "MPROTECT") == 0) {
			safe_mprotect_command(arena);
			continue;
		}

		printf("Invalid command. Please try again.\n");
	}

	return 0;
}
