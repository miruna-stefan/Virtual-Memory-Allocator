# Virtual-Memory-Allocator
a program that simulates the traditional malloc and calloc functions by marking as used specific memory blocks from a pre-allocated pool of bytes, called arena; The virtual memory allocator also has a functionality that frees the reserved memory blocks.

STEFAN MIRUNA ANDREEA 314CA

VIRTUAL MEMORY ALLOCATOR

In order to build a virtual memory allocator, we are going to implement a
doubly linked list, in which each element points to another doubly linked list,
as the arena contains a list of blocks and each block contains a list of
miniblocks. Therefore, the specific functions that help when operating with
doubly linked lists are going to be very useful:

- "dll_create": creates a new doubly linked list;

- "create_node": allocates memory for a new node and also puts the new data in
it (new_data being a generic pointer);

- "dll_get_nth_node": browses the list received as parameter in order to reach
the node on the nth position;

- "dll_add_nth_node": creates a new node by calling the "create_node_function"
and inserts the new node in the list on a specific postion (received as 
parameter);

- "dll_remove_nth_node": eliminates from the list the node that is situated on
a specific position, without freeing the memory. This function only breaks the
links to/from the node (it kind of "isolates" the node) and returns a pointer 
to the removed node;

- "dll_free": it frees the data in each node, each node structure and then
the whole list. This function has been changed a little, in order to work 
properly not only for lists of blocks, but also for lists of miniblocks, so as
to correctly free each field of the structure.

One of the most interesting, but also challenging parts of the homework was the
fact that the programmer needs to make a clear difference between the node in 
itself and the block or miniblock structure associated with it, as in every 
node->data there is a pointer to a block or miniblock structure. Therefore, we 
need to be very careful when freeing the memory, as both the node->data and the
block/miniblock structure have been allocated dynamically, so the number of 
mallocs or callocs needs to be equal to the number of frees.

So now that we have all the necessary tools when it comes to working with 
doubly linked lists, we may proceed in implementing the virtual memory 
allocator. For this, we are going to use the following functions:

- "create_miniblock": allocates memory and initializes all the fields of the 
miniblock structure;

-"create_block": allocates memory and initializes all the fields of the block 
structure; it also calls the previous function, as for every new block, one 
miniblock should be created in its miniblock list;

-"alloc_arena": although the name of the function might be a little 
missleading, the arena is a generic vector that must not be allocated. This 
function only creates the arena structure, which stores the size of the arena 
and a pointer to the list of blocks, also created within this function;

-"dealloc_arena": browses through the list of blocks in order to free each 
block's miniblock list and then it browses it again, this time in order to free
every block. Then, the arena is freed.

-"alloc_block": this function is very wide and complex, so it needs to be very 
well organized and split into numerous functions, such as:

	- "alloc_validity": this function has been created so as to verify the 
	validity of the data (address and size) received from the keyboard.
	Hence, it checks if the block that the user wishes to allocate memory 
	for would fit into the arena (would be situated within the boarders of 
	the arena). If so, it browses the list of blocks so as to make sure 
	that the zone hasn't already been allocated. In conclusion, if this 
	function returns 1, then we are certain that the address and the size
	received from the user are valid.

	Now we only have one more important detail to take into consideration: 
	the case in which the list is empty. In this case, the problem is
	simple: we only have to create our first block and a node for it, which
	we are going to place on the first position of the block list. This is 
	going to be the head of the arena.

	Next step: we need to decide whether we should create a block or a 
	miniblock. Therefore, we have to find out if the zone that we want to 
	allocate already has any neighbours. 
	
	- "no_neighbours_alloc": if the zone does not have any neighbour, then 
	we should create a block and add a new node in the block list, which is
	exactly what this function does;
	
	- "left_neighbour_only_alloc": this function treats the case in which 
	the zone that we have to allocate is adjacent with another zone on the 
	left side, so we say that it has a left neighbour. Consequently, we are
	not allowed to create a block, but a miniblock, that we should insert 
	on the last position of the left neighbour's miniblock list. We must 
	not forget to increase the size of the left neighbour block 
	accordingly!
	
	-"right_neighbour_only_alloc": treats a similar case to the one tackled
	by the previous function, only that here the only neighbour is on the 
	right side. So, we have to create a miniblock that should be added on 
	the first position of the right neighbour's miniblock list. In this 
	case, besides updating right neighbour's block size, we need to change
	its start address into the new miniblock's start address.
	
	- "two_neighbours_alloc": if the zone has both a left and a right 
	neighbour, we must basically merge the left neighbour, the zone, and 
	the right neighbour, so that they will become a much bigger block 
	together. For this, we should create a miniblock, add it at the end of
	left neighbour's miniblock list and then move the miniblocks from right
	neighbour's miniblock list one by one at the end of left neighbour's
	miniblock list (the process is as follows: we remove the first 
	miniblock from the right neighbour's miniblock list and add it on the 
	last position of the left neighbour's miniblock list, then repeat the 
	process until the right neighbour's miniblock list becomes empty). 
	After that, we should update the size of the left_neighbour's block and
	free the right neighbour block.
	
-"free_block": the purpose of this function is to free the memory of a zone 
that was previously allocated, so the first step would be to check if the 
address received as parameter is valid (is within the borders of the arena). If
so, we have to browse the block list until we find the block which contains 
this address. Then, we browse through its miniblock list until we find the 
miniblock that starts with that address and store its position in the miniblock
list. Now, we need to tackle three different cases, by calling three auxiliary
functions:
	- "remove_block_with_only_one_miniblock": if the block only contains
	one miniblock (so its miniblock list has only one node), we call this 
	function in order to remove and free not only the miniblock and the 
	miniblock list, but also the block in itself.
	
	-"free_fisrt_or_last": if the miniblock is on the first or on the last 
	position in the miniblock list, then we call this function to free it.
	
	-"free_middle_miniblock": this function is used to free a miniblock 
	that is situated somewhere in the middle of the miniblock list. It also
	splits the miniblock list into two separate lists: the first one 
	remains the miniblock list of the old block and the second part becomes
	the miniblock list of a new block, that we also create within this 
	function. Something that we need to pay attention to is the fact that
	the "create_block" function automatically creates a miniblock 
	associated with the new block. Therefore, after moving all the 
	miniblocks (that were situated after the removed miniblock in the list)
	to the miniblock list of the new block, we need to free the first 
	miniblock in the new block's miniblock list, as it would be a duplicate.
	
-"read": this function is used in order to read data (starting from a specific 
address) that is situated in the rw_buffers of the miniblocks. Firstly, we need
to scroll through the block list until finding the block that matches to the 
address received as parameter and then we do the exact same thing in order to 
find the miniblock which contains the address. Note that we can start reading 
from the middle of the miniblock, not only from the start address, so we need
to memorize the offset (the distance between the address where we start reading
and the start address of the miniblock). Now we need to tackle different cases:
if the size (the number of characters that we need to display on screen) is 
short enough to fit in a miniblock, then we would have to read the exact size 
from the miniblock without any problems. If the size exceeds the length of the 
miniblock, we need to read the whole buffer and move on to the next miniblock 
until the remaining size fits into a miniblock.

	-when we are done with copying the information from the buffers into a
	string (created and dynamically allocated by us), we need to print each
	character of this string and then free the memory allocated for this 
	string. For that, we are going to use the function "print_data".

	- another auxiliary function for the "read" function is 
	"verify_read_permission" which checks whether a miniblock has read 
	permissions, according to the specific bit mask.
	
-"write": this function is very similar to the "read" function, as it follows
the same steps and concepts. The only difference is that instead of reading
information from one or more miniblocks, it writes info. Analogically, instead
of "verify_read_permission", we call "verify_write_permission", which checks 
whether a miniblock has write permissions.

-"pmap": this function prints the information regarding the arena, the block 
list and each block's miniblock list in a specific format

-"mprotect": this function has the role to change the permissions of a 
miniblock. After scrolling through the block list, finding the block that 
contains the address received as parameter, scrolling through its miniblock 
list and finding the miniblock starting with that address, it compares the 
string introduced by the user with the permission specific format for the 
mprotect command. In accordance with the permissions that need to be modified, 
it changes the bit mask associated with the miniblock.

INT MAIN
- we read strings from keyboard and identify the command until we receive the 
command "DEALLOC_ARENA", which marks the end of the program. For each command, 
we verify if it is followed by the correct number of parameters (if the command
is valid). If so, we read the parameters and call the associated function from
vma.c. If the command is not valid, we print a specific message.

