#include <bsp.h>
#include "utilities.h"
#include "graph.h"

#ifndef COMMUNICATION_INSTRUCTIONS
#define COMMUNICATION_INSTRUCTIONS

// The defined instructions.
// The instructions are evaluated in an ascending order.
#define MAX_INSTR_VAL 3
//------------------- :
#define CONCATENATE   2
#define REVERSE       1
#define INHERIT       0

/**
 * Instruction structure that processes use to share updates.
 *
 * Attributes:
 * - `name`             One of the instructions names listed above.
 * - `relevant_nodes`   Array of node values that concern the instruction.
 */
struct instruction {
    uint name;
    uint relevant_nodes[4];
};

/**
 * View upon a neighbouring vertex in a different process. This is the
 * minimum necessary amount to share sufficient information about nodes.
 *
 * Attributes:
 * - `is_head`      Whether the node is the head of a snake.
 * - `value`        The unique identifier of the node.
 * - `previous`     Identifier of the node that precedes this node in its
 *                      own snake. Used to spot opportunities of inheritance.
 * - `base`         Indentifier of the node's snake's base value.
 */
struct neighbour {
    bool is_head;
    uint value;
    uint previous;
    uint base;
};

#endif

/**
 * Create a new instruction for the entire graph to alter.
 *
 * Parameters:
 * - `name`     Task name. This is usually a predefined enumerator in
 *              the header file `communiate.h` like CONCATENATE, REVERSE
 *              or INHERIT.
 */
struct instruction *create_instruction(uint name);

/**
 * Unallocate an instruction to free memory.
 *
 * Parameters:
 * - `task`     Instruction that is freed from memory.
 */
void unallocate_instruction(struct instruction *task);

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
                                          struct neighbour *prey);

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
                                              struct neighbour *prey);

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
                                              struct neighbour *belly_two);
