#include "communicate.h"

/**
 * Create a new instruction for the entire graph to alter.
 *
 * Parameters:
 * - `name`     Task name. This is usually a predefined enumerator in
 *              the header file `communiate.h` like CONCATENATE, REVERSE
 *              or INHERIT.
 */
struct instruction *create_instruction(uint name) {
    struct instruction *task = malloc(sizeof(struct instruction));             
    task->name = name;
    return task;
}

/**
 * Unallocate an instruction to free memory.
 *
 * Parameters:
 * - `task`     Instruction that is freed from memory.
 */
void unallocate_instruction(struct instruction *task) {
    free(task);
}

/**
 * Create the instruction to eat another snake.
 *
 * Parameters:
 * - `hunter`   Snake that eats the other snake.
 * - `prey`     Snake that is getting eaten.
 *
 * Returns:     Instruction to eat the `prey` snake.
 */
struct instruction *instruction_eat_snake(struct snake *hunter, 
                                          struct neighbour *prey) {
    struct instruction *task = create_instruction(CONCATENATE);

    task->relevant_nodes[0] = hunter->local_head->value; // This node
    task->relevant_nodes[1] = prey->value;   //  claims that this base
    task->relevant_nodes[2] = hunter->base;  // should become this base.

    return task;
}

/**
 * Create the request to reverse and eat a snake.
 *
 * Parameters:
 * - `hunter`   Snake that demands the reverse.
 * - `prey`     Snake that is requested to reverse.
 * 
 * Returns:     Instruction to reverse the `prey` snake.
 */
struct instruction *instruction_reverse_snake(struct snake *hunter,
                                              struct neighbour *prey) {
    struct instruction *task = create_instruction(REVERSE);

    task->relevant_nodes[0] = hunter->local_head->value; // This node
    task->relevant_nodes[1] = prey->base;   // wants this snake
    task->relevant_nodes[2] = hunter->base; // to reverse and make this node
                                            // the new base.
    
    return task;
}

/**
 * Create the instruction to inherit a snek (relatively small snake) into a
 * larger snake. This process incorporates one-process snakes into larger
 * snakes by cutting the larger snake in half and attaching the snek.
 *
 * Parameters:
 * - `snek`         Small snake that injects itself into a snake's belly.
 * - `belly_two`    The belly node that the snek jumps behind. This node also
 *                      holds the knowledge of which node the snek jumps
 *                      in front of.
 *
 * Returns:         Instruction to inherit the snek.
 */
struct instruction *instruction_inherit_snake(struct snake *snek,
                                              struct neighbour *belly_two) {
    struct instruction *task = create_instruction(INHERIT);

    task->relevant_nodes[0] = snek->local_tail->value;    // Snek head
    task->relevant_nodes[1] = snek->local_head->value;    // Snek tail
    task->relevant_nodes[2] = belly_two->value;    // Attach snek head here
    task->relevant_nodes[3] = belly_two->previous; // Attach snek tail here
    
    // In theory, this base value is not necessary.
    // However, it speeds up the process of finding the appropriate snake
    // because it prevents every snake from checking whether the concerning
    // nodes are hidden somewhere in its belly.
    task->relevant_nodes[4] = snek->base;

    return task;
}
