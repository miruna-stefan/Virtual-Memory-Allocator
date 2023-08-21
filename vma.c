// Stefan Miruna Andreea 314CA
#include "vma.h"

// create a doubly linked list
list_t *dll_create(unsigned int data_size)
{
	list_t *list;
	list = malloc(sizeof(list_t));
	// defensive programming
	DIE(NULL == list, "malloc failed\n");

	list->head = NULL;
	list->data_size = data_size;
	list->size = 0;

	return list;
}

// create a new node for a doubly linked list
static dll_node_t *create_node(const void *new_data, uint data_size)
{
	dll_node_t *node;

	node = malloc(sizeof(dll_node_t));
	// defensive programming
	DIE(NULL == node, "malloc failed\n");

	node->data = malloc(data_size);
	// defensive programming
	DIE(NULL == node->data, "malloc failed\n");

	memcpy(node->data, new_data, data_size);
	node->next = NULL;
	node->prev = NULL;

	return node;
}

// reach the node that is situated on the nth position
dll_node_t *dll_get_nth_node(list_t *list, unsigned int n)
{
	if (!list) {
		fprintf(stderr, "The list hasn't been created yet\n");
		return NULL;
	}

	if (!list->size) {
		fprintf(stderr, "No elements in the list\n");
		return NULL;
	}

	if (n >= list->size)
		n = n % list->size;

	dll_node_t *node;
	node = list->head;

	for (uint i = 0; i < n; i++)
		node = node->next;

	return node;
}

// create a new node and insert in the list on the nth position
void
dll_add_nth_node(list_t *list, uint n, const void *new_data, uint data_size)
{
	if (!list) {
		fprintf(stderr, "The list hasn't been created yet\n");
		return;
	}

	if (n >= list->size)
		n = list->size;

	dll_node_t *node, *prev_node;
	node = create_node(new_data, data_size);

	if (list->size == 0) {
		list->head = node;
		list->size++;
		return;
	}

	if (n == 0) {
		// insert in the first place
		dll_node_t *curr;
		curr = list->head;
		node->next = curr;
		curr->prev = node;
		list->head = node;
	} else {
		// insert at the end of the list
		if (n == list->size) {
			prev_node = list->head;
			while (prev_node->next)
				prev_node = prev_node->next;
			node->prev = prev_node;
			prev_node->next = node;
		} else {
			// insert on the nth position
			prev_node = dll_get_nth_node(list, n - 1);
			node->next = prev_node->next;
			prev_node->next->prev = node;
			prev_node->next = node;
			node->prev = prev_node;
		}
	}

	list->size++;
}

// eliminate the nth pointer in a doubly linked list without freeing the memory
dll_node_t *dll_remove_nth_node(list_t *list, unsigned int n)
{
	if (!list) {
		fprintf(stderr, "The list hasn't been created yet\n");
		return NULL;
	}

	if (!list->size) {
		fprintf(stderr, "No elements in the list\n");
		return NULL;
	}

	if (n >= list->size)
		n = list->size - 1;

	dll_node_t *removed;

	// remove the only node in the list
	if (list->size == 1) {
		removed = list->head;
		list->head = NULL;
		list->size--;
		return removed;
	}

	if (n == list->size - 1) {
		// remove last node
		removed = dll_get_nth_node(list, n);
		removed->prev->next = NULL;
	} else {
		if (n == 0) {
			// remove first node
			removed = list->head;
			list->head = list->head->next;
			list->head->prev = NULL;
		} else {
			removed = dll_get_nth_node(list, n);
			removed->prev->next = removed->next;
			removed->next->prev = removed->prev;
	}
	}

	list->size--;

	return removed;
}

//free the memory allocated from a list
void dll_free(list_t **pp_list)
{
	if (!pp_list || !*pp_list)
		return;

	dll_node_t *node;

	// free a list of miniblocks
	if ((*pp_list)->data_size == sizeof(miniblock_t))
		while ((*pp_list)->size > 0) {
			node = dll_remove_nth_node(*pp_list, 0);
			free(((miniblock_t *)node->data)->rw_buffer);
			free((miniblock_t *)node->data);
			free(node);
		}

	// free a list of blocks
	if ((*pp_list)->data_size == sizeof(block_t))
		while ((*pp_list)->size > 0) {
			node = dll_remove_nth_node(*pp_list, 0);
			free((block_t *)node->data);
			free(node);
		}

	free(*pp_list);
	*pp_list = NULL;
}

miniblock_t *create_miniblock(uint64_t start_address, size_t size)
{
	miniblock_t *new_miniblock;
	new_miniblock = malloc(sizeof(miniblock_t));
	// defensive programming
	DIE(NULL == new_miniblock, "malloc failed\n");

	new_miniblock->start_address = start_address;
	new_miniblock->size = size;
	new_miniblock->perm = 6; // by default, a new miniblock has RW- permission
	new_miniblock->rw_buffer = calloc(size, 1);
	// defensive programming
	DIE(NULL == new_miniblock->rw_buffer, "malloc failed\n");

	return new_miniblock;
}

block_t *create_block(uint64_t start_address, size_t size)
{
	block_t *new_block = malloc(sizeof(block_t));
	// defensive programming
	DIE(NULL == new_block, "malloc failed\n");

	new_block->start_address = start_address;
	new_block->size = size;
	new_block->miniblock_list = dll_create(sizeof(miniblock_t));
	miniblock_t *first_miniblock = create_miniblock(start_address, size);
	list_t *mini_list = new_block->miniblock_list;
	dll_add_nth_node(mini_list, 0, first_miniblock, sizeof(miniblock_t));
	free((miniblock_t *)first_miniblock);

	return new_block;
}

arena_t *alloc_arena(const uint64_t size)
{
	arena_t *arena = malloc(sizeof(arena_t));
	// defensive programming
	DIE(NULL == arena, "malloc failed\n");

	arena->arena_size = size;
	arena->alloc_list = dll_create(sizeof(block_t));

	return arena;
}

void dealloc_arena(arena_t *arena)
{
	dll_node_t *curr = arena->alloc_list->head;
	while (curr) {
		list_t *mini_list = ((block_t *)curr->data)->miniblock_list;
		dll_free(&mini_list);
		curr = curr->next;
	}
	dll_free(&arena->alloc_list);
	free(arena);
}

/* function that checks if the address and size for theblock allocation is valid
(0) = not valid, (1) = valid */
int alloc_validity(arena_t *arena, const uint64_t address, const uint64_t size)
{
	if (address >= arena->arena_size) {
		printf("The allocated address is outside the size of arena\n");
		return 0;
	}

	if (address + size > arena->arena_size) {
		printf("The end address is past the size of the arena\n");
		return 0;
	}

	// check whether the zone has already been allocated
	dll_node_t *current_block = arena->alloc_list->head;
	if (arena->alloc_list->head) {
		// the arena has at least one block
		while (current_block) {
			uint64_t start = ((block_t *)current_block->data)->start_address;
			uint64_t end = start + ((block_t *)current_block->data)->size;
			if ((start > address && address + size > start) ||
				(address >= start && address < end)) {
				printf("This zone was already allocated.\n");
				return 0;
			}
			current_block = current_block->next;
		}
	}

	return 1;
}

// allocate a new block, as it has no neighbours
void
no_neighbours_alloc(arena_t *arena, const uint64_t address, const uint64_t size)
{
	block_t *new_block = create_block(address, size);

	dll_node_t *curr = arena->alloc_list->head;
	/* counts how many nodes there are before the position
	where the new node should be inserted */
	int cnt = 0;
	while (curr && ((block_t *)curr->data)->start_address < address) {
		cnt++;
		curr = curr->next;
	}
	dll_add_nth_node(arena->alloc_list, cnt, new_block, sizeof(block_t));
	free((block_t *)new_block);
}

// allocate a miniblock that only has a left neighbour
void left_neighbour_only_alloc(const uint64_t address,
							   const uint64_t size, dll_node_t *left_neighbour)
{
	miniblock_t *new_miniblock = create_miniblock(address, size);
	list_t *mini_list = ((block_t *)left_neighbour->data)->miniblock_list;
	unsigned int mini_size = mini_list->size;
	dll_add_nth_node(mini_list, mini_size, new_miniblock, sizeof(miniblock_t));
	free((miniblock_t *)new_miniblock);
	((block_t *)left_neighbour->data)->size += size;
}

// allocate a miniblock that only has a right neighbour
void right_neighbour_only_alloc(const uint64_t address, const uint64_t size,
								dll_node_t *right_neighbour)
{
	miniblock_t *new_miniblock = create_miniblock(address, size);
	list_t *mini_list = ((block_t *)right_neighbour->data)->miniblock_list;
	dll_add_nth_node(mini_list, 0, new_miniblock, sizeof(miniblock_t));
	free((miniblock_t *)new_miniblock);
	((block_t *)right_neighbour->data)->size += size;
	((block_t *)right_neighbour->data)->start_address = address;
}

// allocate a miniblock that has neighbours on both sides
void
two_neighbours_alloc(const uint64_t address, const uint64_t size,
					 dll_node_t *right_neighbour, dll_node_t *left_neighbour)
{
	miniblock_t *new_miniblock = create_miniblock(address, size);
		// left_neighbour's list of miniblocks
		list_t *min_list = ((block_t *)left_neighbour->data)->miniblock_list;
		unsigned int min_dim = min_list->size;
		dll_add_nth_node(min_list, min_dim, new_miniblock, sizeof(miniblock_t));
		free((miniblock_t *)new_miniblock);

		dll_node_t *removed;
		// merge the miniblock_lists
		while (((block_t *)right_neighbour->data)->miniblock_list->size > 0) {
			list_t *right_min_list; // right neighbour's miniblock list
			right_min_list = ((block_t *)right_neighbour->data)->miniblock_list;
			removed = dll_remove_nth_node(right_min_list, 0);
			dll_node_t *prev_node = min_list->head;
			while (prev_node->next)
				prev_node = prev_node->next;
			removed->prev = prev_node;
			prev_node->next = removed;
			min_list->size++;
		}

		// remove right neighbour block
		free(((block_t *)right_neighbour->data)->miniblock_list);
		removed = right_neighbour;
		removed->prev->next = removed->next;
		/* protection in case the right neighbour
		is the last node in the block list */
		if (removed->next)
			removed->next->prev = removed->prev;

		// modify left_neighbour's block_size
		size_t right_size = ((block_t *)right_neighbour->data)->size;
		((block_t *)left_neighbour->data)->size += size + right_size;

		// free right neighbour
		free((block_t *)right_neighbour->data);
		free(removed);
}

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	// error handling
	if (alloc_validity(arena, address, size) == 0)
		return;

	// checks if the arena is empty
	if (!arena->alloc_list->head) {
		// create a new block and make it the head of the list
		block_t *new_block = create_block(address, size);
		dll_node_t *new_block_node;
		new_block_node = create_node((block_t *)new_block, sizeof(block_t));
		arena->alloc_list->head = new_block_node;
		arena->alloc_list->size++;
		free(new_block);
		return;
	}

	/* look for neighbours in order to decide whether
	it is an individual block or a miniblock */
	dll_node_t *left_neighbour = NULL;
	dll_node_t *right_neighbour = NULL;

	dll_node_t *current_node = arena->alloc_list->head; // the head of the arena
	while (current_node) {
		uint64_t start = ((block_t *)current_node->data)->start_address;
		uint64_t end = start + ((block_t *)current_node->data)->size;
		if (end == address)
			left_neighbour = current_node;
		if (start == address + size)
			right_neighbour = current_node;
		current_node = current_node->next;
		}

	// if there are no neighbours, alloc a new block
	if (!left_neighbour && !right_neighbour)
		no_neighbours_alloc(arena, address, size);

	// if the new block has a neighbour only on the left side
	if (left_neighbour && !right_neighbour)
		left_neighbour_only_alloc(address, size, left_neighbour);

	// if the new block has a neighbour only on the right side
	if (!left_neighbour && right_neighbour)
		right_neighbour_only_alloc(address, size, right_neighbour);

	// if the new block has both a left and a right neighbour
	if (left_neighbour && right_neighbour) {
		two_neighbours_alloc(address, size, right_neighbour, left_neighbour);

		// update the number of blocks in the arena
		arena->alloc_list->size--;
	}
}

/* used for blocks that contain only one node in the miniblock list, this
function removes and frees both the miniblock and the block in itself */
void
remove_block_with_only_one_miniblock(arena_t *arena, int cnt, list_t *mini_list)
{
	dll_node_t *removed_block_node;
	removed_block_node = dll_remove_nth_node(arena->alloc_list, cnt);
	miniblock_t *mini = (miniblock_t *)mini_list->head->data;
	free(mini->rw_buffer);
	free(mini);
	free(mini_list->head);
	free(mini_list);
	free((block_t *)removed_block_node->data);
	free(removed_block_node);
}

// frees the first or last miniblock node in the list
void
free_first_or_last(dll_node_t *curr_block_node, list_t *mini_list, uint pos)
{
	dll_node_t *removed = dll_remove_nth_node(mini_list, pos);
	if (pos == 0) {
		/* the current block takes the start address of the block
		situated after the removed one in the list */
		uint64_t rm_next_start;
		rm_next_start = ((miniblock_t *)removed->next->data)->start_address;
		((block_t *)curr_block_node->data)->start_address = rm_next_start;
	}
	size_t removed_size;
	removed_size = ((miniblock_t *)removed->data)->size;
	((block_t *)curr_block_node->data)->size -= removed_size;
	free(((miniblock_t *)removed->data)->rw_buffer);
	free((miniblock_t *)removed->data);
	free(removed);
}

/* frees a node situated in the middle of a miniblock list and then splits
the list */
void free_middle_miniblock(arena_t *arena, dll_node_t *curr_block_node,
						   list_t *mini_list, uint pos, int cnt)
{
	dll_node_t *removed = dll_remove_nth_node(mini_list, pos);
	size_t removed_size;
	removed_size = ((miniblock_t *)removed->data)->size;
	((block_t *)curr_block_node->data)->size -= removed_size;
	uint64_t rmv_next_start;
	rmv_next_start = ((miniblock_t *)removed->next->data)->start_address;
	uint64_t new_block_address = rmv_next_start;
	size_t new_block_size = ((miniblock_t *)removed->next->data)->size;
	free(((miniblock_t *)removed->data)->rw_buffer);
	free((miniblock_t *)removed->data);
	free(removed);

	block_t *new_block = create_block(new_block_address, new_block_size);
	dll_add_nth_node(arena->alloc_list, cnt + 1, new_block, sizeof(block_t));
	free((block_t *)new_block);
	block_t *curr_block_info = (block_t *)curr_block_node->next->data;
	list_t *new_block_mini_list = curr_block_info->miniblock_list;

	while (pos <= (mini_list->size - 1)) {
		//insert the node at the end of the new_block's miniblock list
		removed = dll_remove_nth_node(mini_list, pos);
		((block_t *)curr_block_node->data)->size -= removed_size;
		((block_t *)curr_block_node->next->data)->size += removed_size;
		dll_node_t *prev_node = new_block_mini_list->head;
		while (prev_node->next)
			prev_node = prev_node->next;
		removed->prev = prev_node;
		prev_node->next = removed;
		removed->next = NULL;
		new_block_mini_list->size++;
	}

	/* the "create_block" function automatically created an associated
	miniblock, that we need to eliminate */
	dll_node_t *trash = dll_remove_nth_node(new_block_mini_list, 0);
	size_t trash_size = ((miniblock_t *)trash->data)->size;
	((block_t *)curr_block_node->next->data)->size -= trash_size;
	free(((miniblock_t *)trash->data)->rw_buffer);
	free((miniblock_t *)trash->data);
	free(trash);
}

// function that frees the memory for an already allocated zone
void free_block(arena_t *arena, const uint64_t address)
{
	if (address > arena->arena_size) {
		printf("Invalid address for free.\n");
		return;
	}

	if (arena->alloc_list->size == 0) {
		printf("Invalid address for free.\n");
		return;
	}

	// find the node which contains the miniblock that needs to be freed
	dll_node_t *curr_block_node = arena->alloc_list->head;
	int cnt = 0;
	while (curr_block_node->next) {
		if (((block_t *)curr_block_node->data)->start_address <= address &&
			address < ((block_t *)curr_block_node->next->data)->start_address)
			break;
		cnt++;
		curr_block_node = curr_block_node->next;
	}

	/* if the block that we are looking for is the last one,
	the while-loop is left before incrementing cnt */
	if (!curr_block_node->next)
		cnt++;

	list_t *mini_list = ((block_t *)curr_block_node->data)->miniblock_list;
	// if the block only contains one miniblock
	if (((block_t *)curr_block_node->data)->start_address == address &&
		mini_list->size == 1) {
		remove_block_with_only_one_miniblock(arena, cnt, mini_list);
		return;
	}

	// find our miniblock's position in the miniblock_list
	uint pos = 0, found = 0;
	dll_node_t *curr_mini = mini_list->head;
	while (curr_mini) {
		if (((miniblock_t *)curr_mini->data)->start_address == address) {
			found = 1;
			break;
		}
		pos++;
		curr_mini = curr_mini->next;
	}

	if (found == 0) {
		printf("Invalid address for free.\n");
		return;
	}

	/* remove a miniblock that is situated at the
	beginning or at the end of the list */
	if (pos == 0 || pos == mini_list->size - 1) {
		free_first_or_last(curr_block_node, mini_list, pos);
		return;
	}

	/* remove a miniblock that is situated in the middle of
	the miniblock_list and then split the list */
	free_middle_miniblock(arena, curr_block_node, mini_list, pos, cnt);
}

/* function that returns 1 if a miniblock has read
permissions or 0 if it doesn't */
int verify_read_permission(dll_node_t *curr_mini)
{
	if (((miniblock_t *)curr_mini->data)->perm != 4 &&
		((miniblock_t *)curr_mini->data)->perm != 6 &&
		((miniblock_t *)curr_mini->data)->perm != 7) {
		printf("Invalid permissions for read.\n");
		return 0;
	}
	return 1;
}

void print_data(uint64_t reduced_size, int8_t **data)
{
	for (uint i = 0; i < reduced_size; i++)
		if ((*data)[i] != 0)
			printf("%c", (*data)[i]);
	printf("\n");
	free(*data);
}

void read(arena_t *arena, uint64_t address, uint64_t size)
{
	uint found = 0;
	dll_node_t *curr = arena->alloc_list->head;
	uint64_t start_block, end_block;
	while (curr) {
		start_block = ((block_t *)curr->data)->start_address;
		end_block = start_block + ((block_t *)curr->data)->size;
		if (start_block <= address && address < end_block) {
			found = 1;
			break;
		}
		curr = curr->next;
	}
	if (found == 0) {
		printf("Invalid address for read.\n");
		return;
	}

	uint64_t updated_size = size, reduced_size = size;
	list_t *mini_list = ((block_t *)curr->data)->miniblock_list;
	dll_node_t *curr_mini = mini_list->head;
	uint64_t curr_mini_start;
	while (curr_mini) {
		curr_mini_start = ((miniblock_t *)curr_mini->data)->start_address;
		if (((miniblock_t *)curr_mini->data)->start_address <= address &&
			address < curr_mini_start + ((miniblock_t *)curr_mini->data)->size)
			break;
		curr_mini = curr_mini->next;
	}

	if (verify_read_permission(curr_mini) == 0)
		return;

	if (end_block < address + size) {
		reduced_size = end_block - address;
		printf("Warning: size was bigger than the block size. ");
		printf("Reading %lu characters.\n", reduced_size);
	}

	int8_t *data = malloc(reduced_size * sizeof(int8_t));
	DIE(NULL == data, "malloc failed\n");

	uint offset = address - ((miniblock_t *)curr_mini->data)->start_address;
	uint64_t start_miniblock = ((miniblock_t *)curr_mini->data)->start_address;
	uint64_t end_miniblock;
	end_miniblock = start_miniblock + ((miniblock_t *)curr_mini->data)->size;
	void *buff = ((miniblock_t *)curr_mini->data)->rw_buffer;
	if (reduced_size <= end_miniblock - address) {
		// the size is short enough to fit in a miniblock rw_buffer
		memcpy(data, buff + offset, reduced_size);
		print_data(reduced_size, &data);
		return;
	}

	/* if the size exceeds the size of the miniblock, then we should continue
	reading from the following miniblocks' rw_buffers */
	memcpy(data, buff + offset, end_miniblock - address);
	updated_size = reduced_size - (end_miniblock - address);
	curr_mini = curr_mini->next;

	while (updated_size > 0) {
		buff = ((miniblock_t *)curr_mini->data)->rw_buffer;
		if (updated_size <= ((miniblock_t *)curr_mini->data)->size) {
			if (verify_read_permission(curr_mini) == 0) {
				free(data);
				return;
			}
			memcpy(data + (reduced_size - updated_size), buff, updated_size);
			break;
		}
		if (verify_read_permission(curr_mini) == 0) {
			free(data);
			return;
			}
		size_t mini_size = ((miniblock_t *)curr_mini->data)->size;
		memcpy(data + (reduced_size - updated_size), buff, mini_size);
		updated_size = updated_size - ((miniblock_t *)curr_mini->data)->size;
		curr_mini = curr_mini->next;
	}
	print_data(reduced_size, &data);
}

/* function that returns 1 if a miniblock has write permissions
or 0 if it doesn't */
int verify_write_permission(dll_node_t *curr_mini)
{
	if (((miniblock_t *)curr_mini->data)->perm != 2 &&
		((miniblock_t *)curr_mini->data)->perm != 6 &&
		((miniblock_t *)curr_mini->data)->perm != 7) {
		printf("Invalid permissions for write.\n");
		return 0;
	}
	return 1;
}

void
write(arena_t *arena, const uint64_t address, const uint64_t size, int8_t *data)
{
	if (address > arena->arena_size) {
		printf("Invalid address for write.\n");
		return;
	}

	uint found = 0;
	dll_node_t *curr = arena->alloc_list->head;
	uint64_t start_block, end_block;
	while (curr) {
		start_block = ((block_t *)curr->data)->start_address;
		end_block = start_block + ((block_t *)curr->data)->size;
		if (start_block <= address && address < end_block) {
			found = 1;
			break;
		}
		curr = curr->next;
	}

	if (found == 0) {
		printf("Invalid address for write.\n");
		return;
	}

	uint64_t updated_size = size;
	uint64_t reduced_size = size;
	list_t *mini_list = ((block_t *)curr->data)->miniblock_list;
	dll_node_t *curr_mini = mini_list->head;
	uint64_t start;
	while (curr_mini) {
		start = ((miniblock_t *)curr_mini->data)->start_address;
		if (start <= address &&
			address < start + ((miniblock_t *)curr_mini->data)->size)
			break;
		curr_mini = curr_mini->next;
	}

	if (verify_write_permission(curr_mini) == 0)
		return;

	uint offset = address - ((miniblock_t *)curr_mini->data)->start_address;
	if (end_block < address + size) {
		reduced_size = end_block - address;
		printf("Warning: size was bigger than the block size. ");
		printf("Writing %lu characters.\n", reduced_size);
	}

	void *buff = ((miniblock_t *)curr_mini->data)->rw_buffer;
	uint64_t end_miniblock = start + ((miniblock_t *)curr_mini->data)->size;
	if (reduced_size <= end_miniblock - address) {
		// the size is short enough to fit in a miniblock rw_buffer
		memcpy(buff + offset, data, reduced_size);
		return;
	}

	/* if the string is too long to fit in one miniblock buffer, then we are
	going to split it into successive miniblocks' rw_buffers */
	// firstly, we fill in the first miniblock rw_buffer

	memcpy(buff + offset, data, end_miniblock - address);
	updated_size = reduced_size - (end_miniblock - address);
	curr_mini = curr_mini->next;
	while (updated_size > 0) {
		buff = ((miniblock_t *)curr_mini->data)->rw_buffer;
		if (updated_size <= ((miniblock_t *)curr_mini->data)->size) {
			if (verify_write_permission(curr_mini) == 0)
				return;
			memcpy(buff, data + (reduced_size - updated_size), updated_size);
			break;
		}
		if (verify_write_permission(curr_mini) == 0)
			return;
		size_t min_size = ((miniblock_t *)curr_mini->data)->size;
		memcpy(buff, data + (reduced_size - updated_size), min_size);
		updated_size = updated_size - ((miniblock_t *)curr_mini->data)->size;
		curr_mini = curr_mini->next;
	}
}

void pmap(const arena_t *arena)
{
	printf("Total memory: 0x%lX bytes\n", arena->arena_size);

	uint64_t allocated_memory = 0;
	dll_node_t *current_node = arena->alloc_list->head;
	while (current_node) {
		allocated_memory += ((block_t *)current_node->data)->size;
		current_node = current_node->next;
	}
	printf("Free memory: 0x%lX bytes\n", arena->arena_size - allocated_memory);
	printf("Number of allocated blocks: %u\n", arena->alloc_list->size);

	unsigned int nr_miniblocks = 0;
	current_node = arena->alloc_list->head;
	while (current_node) {
		nr_miniblocks += ((block_t *)current_node->data)->miniblock_list->size;
		current_node = current_node->next;
	}
	printf("Number of allocated miniblocks: %u\n", nr_miniblocks);
	if (arena->alloc_list->size != 0)
		printf("\n");

	current_node = arena->alloc_list->head;
	for (unsigned int i = 0; i < arena->alloc_list->size; i++) {
		printf("Block %u begin\n", i + 1);
		uint64_t curr_start = ((block_t *)current_node->data)->start_address;
		printf("Zone: 0x%lX - ", curr_start);
		printf("0x%lX\n", curr_start + ((block_t *)current_node->data)->size);
		list_t *mini_list = ((block_t *)current_node->data)->miniblock_list;
		dll_node_t *curr_miniblock_node = mini_list->head;
		for (unsigned int j = 0; j < mini_list->size; j++) {
			printf("Miniblock %u:", j + 1);
			miniblock_t *curr_miniblock;
			curr_miniblock = (miniblock_t *)curr_miniblock_node->data;
			uint64_t curr_mini_start = curr_miniblock->start_address;
			printf("\t\t0x%lX\t\t-\t\t", curr_mini_start);
			printf("0x%lX\t\t", curr_mini_start + curr_miniblock->size);
			switch (curr_miniblock->perm) {
			case 6:
				printf("| RW-\n");
				break;

			case 7:
				printf("| RWX\n");
				break;

			case 5:
				printf("| R-X\n");
				break;

			case 3:
				printf("| -WX\n");
				break;

			case 0:
				printf("| ---\n");
				break;

			case 4:
				printf("| R--\n");
				break;

			case 2:
				printf("| -W-\n");
				break;

			case 1:
				printf("| --X\n");
				break;

			default:
				break;
			}
			curr_miniblock_node = curr_miniblock_node->next;
		}
		current_node = current_node->next;
		printf("Block %u end\n", i + 1);
		if (i != arena->alloc_list->size - 1)
			printf("\n");
	}
}

void mprotect(arena_t *arena, uint64_t address, char *permission)
{
	if (address > arena->arena_size) {
		printf("Invalid address for mprotect.\n");
		return;
	}

	// check if the address is inside an already allocated block
	uint found = 0;
	dll_node_t *curr = arena->alloc_list->head;
	uint64_t start_block, end_block;
	while (curr) {
		start_block = ((block_t *)curr->data)->start_address;
		end_block = start_block + ((block_t *)curr->data)->size;
		if (start_block <= address && address < end_block) {
			found = 1;
			break;
		}
		curr = curr->next;
	}

	if (found == 0) {
		printf("Invalid address for mprotect.\n");
		return;
	}

	// check if the address is at the beginning of a miniblock
	found = 0;
	list_t *mini_list = ((block_t *)curr->data)->miniblock_list;
	dll_node_t *curr_mini = mini_list->head;
	while (curr_mini) {
		if (((miniblock_t *)curr_mini->data)->start_address == address) {
			found = 1;
			break;
		}
		curr_mini = curr_mini->next;
	}

	if (found == 0) {
		printf("Invalid address for mprotect.\n");
		return;
	}
	if (strcmp(permission, "PROT_NONE\n") == 0) {
		((miniblock_t *)curr_mini->data)->perm = 0;
		return;
	}
	if (strcmp(permission, "PROT_READ\n") == 0) {
		((miniblock_t *)curr_mini->data)->perm = 4;
		return;
	}
	if (strcmp(permission, "PROT_WRITE\n") == 0) {
		((miniblock_t *)curr_mini->data)->perm = 2;
		return;
	}
	if (strcmp(permission, "PROT_EXEC\n") == 0) {
		((miniblock_t *)curr_mini->data)->perm = 1;
		return;
	}
	if (strcmp(permission, "PROT_READ | PROT_WRITE | PROT_EXEC\n") == 0) {
		((miniblock_t *)curr_mini->data)->perm = 7;
		return;
	}
	if (strcmp(permission, "PROT_READ | PROT_WRITE\n") == 0) {
		((miniblock_t *)curr_mini->data)->perm = 6;
		return;
	}
	if (strcmp(permission, "PROT_READ | PROT_EXEC\n") == 0) {
		((miniblock_t *)curr_mini->data)->perm = 5;
		return;
	}
	if (strcmp(permission, "PROT_WRITE | PROT_EXEC\n") == 0) {
		((miniblock_t *)curr_mini->data)->perm = 3;
		return;
	}
	if (strcmp(permission, "PROT_WRITE | PROT_READ\n") == 0) {
		((miniblock_t *)curr_mini->data)->perm = 6;
		return;
	}
	printf("Invalid command. Please try again.\n");
}
