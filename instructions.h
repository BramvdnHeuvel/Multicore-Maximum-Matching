

/**
 * Create the instruction to concatenate a snake.
 *
 * Parameters:
 * - `prey_base`    The tail of the snake that will be concatenated.
 * - `hunter_head`  The head of the eating snake.
 * - `hunter_base`  The tail of the eating snake.
 */
struct instruction *instruction_concatenate_snake(nid_int hunter_base,
                                                  nid_int hunter_head,
                                                  nid_int prey_base);

/**
 * Create the instruction to delete a node from the graph.
 *
 * Parameter:
 * - `nid`          Node identifier that needs to be forgotten.
 * - `neighbour`    Node identifier that is connected to the node.
 *
 * Returns:         Instruction to remove the node.
 */
struct instruction *instruction_delete_node(nid_int nid, nid_int neighbour);

/**
 * Create the instruction to inherit a small snake (snek) by a bigger snake.
 *
 * Parameters:
 * - `snek_base`        The tail of the snek.
 * - `snek_head`        The head of the snek.
 * - `snake_node_one`   The large snake's node that attaches to the snek's tail.
 * - `snake_node_two`   The large snake's node that attaches to the snek's head.
 *
 * Returns:             Instruction to inherit the snek.
 */
struct instruction *instruction_inherit_node(nid_int snek_base,
                                             nid_int snek_head,
                                             nid_int snake_node_one,
                                             nid_int snake_node_two);

/**
 * Create the instruction to move a node to another graph.
 *
 * This instruction is exclusively used for vertices of degree 1 that will
 * either match to their neighbour or be deleted from the graph.
 *
 * Parameter:
 * - `nid`          Node identifier that needs to be moved.
 * - `neighbour`    Node identifier of the node's neighbour.
 *
 * Returns:         Instruction to move the node.
 */
struct instruction *instruction_move_node(nid_int nid, nid_int neighbour);

/**
 * Create the instruction to reverse a multi-process snake.
 *
 * Parameters:
 * - `snake_base`   The tail of the snake.
 * - `snake_head`   The head of the snake, which will become its new tail.
 *
 * Returns:         Instruction to reverse the snake.
 */
struct instruction *instruction_reverse_snake(nid_int snake_base, 
                                              nid_int snake_head);

/**
 * Send all registered instructions to the respective processes.
 *
 * Parameters:
 * - `todo`     todo_list structure.
 *
 * Returns:     Array of instructions of length `todo->expected_responses`.
 */
struct instruction *send_instructions(struct todo_list *todo);

/**
 * Create a new todo list structure for a process.
 *
 * The structure does not need to be directly edited, there are functions to
 * take care of everything. There is no need to clean the todo list either,
 * it can simply be unallocated, after which you can create a new one.
 *
 * Returns:     Pointer to the newly created todo list.
 */
struct todo_list *get_todo_list(void);

/**
 * Add the same instruction to the waiting list of every process.
 *
 * For each process, the function makes a shallow copy of the instruction.
 *
 * Parameters:
 * - `ins`      Pointer to the instruction that needs to be sent.
 * - `todo`     todo_list structure that will send the next instructions.
 */
void add_global_instruction(struct instruction *ins, struct todo_list *todo);

/**
 * Add an instruction to the waiting list for a specific process.
 *
 * Parameters:
 * - `ins`      Pointer to the instruction that needs to be sent.
 * - `todo`     todo_list structure that will send the next instructions.
 * - `p`        Process that the instruction is sent to.
 */
void add_instruction(struct instruction *ins, struct todo_list *todo, uint p);

/**
 * Send an instruction to stdout.
 *
 * This function is mostly used for debugging.
 *
 * Parameters:
 * - `ins`      Instruction to send to stdout.
 */
void show_instruction(struct instruction ins);

/**
 * Deregister a todo list and its content from memory.
 *
 * Parameters:
 * - `todo`     todo_list structure to free from memory.
 */
void unallocate_todo_list(struct todo_list *todo);

