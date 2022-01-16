#include <bsp.h>
#include <stdlib.h>
#include <stdbool.h>

#include "instructions.h"

// PRIVATE FUNCTIONS
bool same_instruction(struct instruction *a, struct instruction *b);
nid_int *exchange_instruction_offsets(struct todo_list *todo);
struct instruction *instruction_keep_alive(void);
void keep_todo_list_alive(struct todo_list *todo);
void unallocate_instruction(struct instruction *ins);

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
                                                  nid_int prey_base) {
    struct instruction *ins = malloc(sizeof(struct instruction));

    ins->value = CONCATENATE;
    ins->next  = NULL;
    ins->content[0] = prey_base;
    ins->content[1] = hunter_base;
    ins->content[2] = hunter_head;
    ins->content[3] = 0;

    return ins;
}

/**
 * Create the instruction to delete a node from the graph.
 *
 * Parameter:
 * - `nid`          Node identifier that needs to be forgotten.
 * - `neighbour`    Node identifier that is connected to the node.
 *
 * Returns:         Instruction to remove the node.
 */
struct instruction *instruction_delete_node(nid_int nid, nid_int neighbour) {
    struct instruction *ins = malloc(sizeof(struct instruction));

    ins->value = DELETE;
    ins->next  = NULL;
    ins->content[0] = nid;
    ins->content[1] = neighbour;
    ins->content[2] = 0;
    ins->content[3] = 0;
    
    return ins;
}

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
                                             nid_int snake_node_two) {
    struct instruction *ins = malloc(sizeof(struct instruction));

    ins->value = INHERIT;
    ins->next  = NULL;
    ins->content[0] = snek_base;
    ins->content[1] = snek_head;
    ins->content[2] = snake_node_one;
    ins->content[3] = snake_node_two;

    return ins;
}

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
struct instruction *instruction_move_node(nid_int nid, nid_int neighbour) {
    struct instruction *ins = malloc(sizeof(struct instruction));

    ins->value = MOVE;
    ins->next  = NULL;
    ins->content[0] = nid;
    ins->content[1] = neighbour;
    ins->content[2] = 0;
    ins->content[3] = 0;

    return ins;
}

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
                                              nid_int snake_head) {
    struct instruction *ins = malloc(sizeof(struct instruction));

    ins->value = REVERSE;
    ins->next  = NULL;
    ins->content[0] = snake_base;
    ins->content[1] = snake_head;
    ins->content[2] = 0;
    ins->content[3] = 0;

    return ins;
}

/**
 * Send all registered instructions to the respective processes.
 *
 * Parameters:
 * - `todo`     todo_list structure.
 *
 * Returns:     Array of instructions of length `todo->expected_responses`.
 */
struct instruction *send_instructions(struct todo_list *todo) {
    uint n = bsp_nprocs();

    keep_todo_list_alive(todo);

    nid_int *offsets = exchange_instruction_offsets(todo);

    bsp_sync();
    unsigned int mem_size = todo->expected_responses * sizeof(struct instruction);
    struct instruction *inss = malloc(mem_size);
    bsp_push_reg(inss, mem_size);
    bsp_sync();
    
    for (uint i=0; i<n; i++) {
        nid_int ins_length      = todo->length[i];
        nid_int offset          = offsets[i];
        struct instruction *ins = todo->p_list[i];

        for (nid_int j=0; j<ins_length; j++) {
            bsp_put(i, ins, inss, (offset+j)*sizeof(struct instruction), 
                    sizeof(struct instruction));
            ins = ins->next;
        }
    }
    free(offsets);

    bsp_sync();
    bsp_pop_reg(inss);

    return inss;
}

/**
 * Create a new todo list structure for a process.
 *
 * The structure does not need to be directly edited, there are functions to
 * take care of everything. There is no need to clean the todo list either,
 * it can simply be unallocated, after which you can create a new one.
 *
 * Returns:     Pointer to the newly created todo list.
 */
struct todo_list *get_todo_list(void) {
    uint n = bsp_nprocs();
    struct todo_list *todo = malloc(sizeof(struct todo_list));

    todo->length = malloc(n * sizeof(nid_int));
    todo->p_list = malloc(n * sizeof(struct instruction *));
    todo->latest = malloc(n * sizeof(struct instruction *));

    for (uint i=0; i<n; i++) {
        todo->length[i] = 0;
        todo->p_list[i] = NULL;
        todo->latest[i] = NULL;
    }

    return todo;
}

/**
 * Add the same instruction to the waiting list of every process.
 *
 * For each process, the function makes a shallow copy of the instruction.
 *
 * Parameters:
 * - `ins`      Pointer to the instruction that needs to be sent.
 * - `todo`     todo_list structure that will send the next instructions.
 */
void add_global_instruction(struct instruction *ins, struct todo_list *todo) {
    uint n = bsp_nprocs();
    struct instruction *new_ins;

    add_instruction(ins, todo, 0);

    // Add shallow copies to other processes
    for (uint i=1; i<n; i++) {
        new_ins  = malloc(sizeof(struct instruction));
        *new_ins = *ins;

        add_instruction(new_ins, todo, i);
    }
}

/**
 * Add an instruction to the waiting list for a specific process.
 *
 * Parameters:
 * - `ins`      Pointer to the instruction that needs to be sent.
 * - `todo`     todo_list structure that will send the next instructions.
 * - `p`        Process that the instruction is sent to.
 */
void add_instruction(struct instruction *ins, struct todo_list *todo, uint p) {
    if (todo->length[p] > 0 && same_instruction(todo->latest[p], ins)) {
        // Soft-prevent the same instruction from appearing multiple times
        return;
    }
    
    if (todo->length[p] == 0) {
        todo->p_list[p] = ins;
    } else {
        todo->latest[p]->next = ins;
    }

    todo->latest[p] = ins;
    todo->length[p]++;
}

/**
 * Send an instruction to stdout.
 *
 * This function is mostly used for debugging.
 *
 * Parameters:
 * - `ins`      Instruction to send to stdout.
 */
void show_instruction(struct instruction ins) {
    printf("< Instruction ");

    short int value = ins.value;
         if (value == 0) {printf("KEEP_ALIVE  ");}
    else if (value == 1) {printf("DELETE      ");}
    else if (value == 2) {printf("MOVE        ");}
    else if (value == 3) {printf("INHERIT     ");}
    else if (value == 4) {printf("REVERSE     ");}
    else if (value == 5) {printf("CONCATENATE ");}
    else                 {printf("UNKNOWN     ");}

    printf("[ ");
    unsigned short int content_length = sizeof(ins.content) / sizeof(nid_int);
    for (unsigned short int i=0; i<content_length; i++) {
        printf("%u ", ins.content[i]);
    }

    printf("] >\n");
}

/**
 * Deregister a todo list and its content from memory.
 *
 * Parameters:
 * - `todo`     todo_list structure to free from memory.
 */
void unallocate_todo_list(struct todo_list *todo) {
    uint n = bsp_nprocs();
    
    for (uint i=0; i<n; i++) {
        unallocate_instruction(todo->p_list[i]);
    }

    free(todo->length);
    free(todo->p_list);
    free(todo->latest);
    free(todo);
}

// |----------------------------------------------|
// |----------------------------------------------|
// |----------------------------------------------|
// |                 P R I V A T E                |
// |----------------------------------------------|
// |----------------------------------------------|
// |               F U N C T I O N S              |
// |----------------------------------------------|
// |----------------------------------------------|
// |----------------------------------------------|

/** PRIVATE
 * Compares two instructions to see if they're shallow copies of each other.
 *
 * Parameters:
 * - `a`        Pointer to the first instruction.
 * - `b`        Pointer to the second instruction.
 *
 * Returns:     boolean whether the two instructions are shallow copies.
 */
bool same_instruction(struct instruction *a, struct instruction *b) {
    if (a == NULL || b == NULL) {
        return (a == NULL && b == NULL);
    }
    if (a->value != b->value) {
        return false;
    }

    unsigned short int content_length = sizeof(a->content) / sizeof(nid_int);

    for (unsigned short int i=0; i<content_length; i++) {
        if (a->content[i] != b->content[i]) {
            return false;
        }
    }

    // Ignore the next pointer.

    return true;
}

/** PRIVATE
 * Exchange at which offsets all instructions may be given to other processes.
 *
 * The process also updates the todo structure on how many instructions may be
 * expected from other processes.
 *
 * Parameters:
 * - `todo`     todo_list structure.
 */
nid_int *exchange_instruction_offsets(struct todo_list *todo) {
    uint n = bsp_nprocs();

    nid_int *numbers = malloc(n * sizeof(nid_int));
    nid_int *offsets = malloc(n * sizeof(nid_int));

    for (uint i=0; i<n; i++) {
        numbers[i] = todo->length[i];
    }

    // DEBUG: Get a message matrix
    bsp_sync();
    for (uint i=0; i<n; i++) {
        if (i == bsp_pid()) {
            if (i == 0) {
                printf("This is how many messages each process sends to each other process:\n");
            }
            printf("[ ");
            for (uint i=0; i<n; i++) {
                numbers[i] = todo->length[i];
                printf("%u ", numbers[i]);
            }
            printf("]\n");
        }
        bsp_sync();
    }

    exchange_numbers_all_to_all(numbers);

    nid_int total = 0;
    for (uint i=0; i<n; i++) {
        offsets[i] = total;
        total      = total + numbers[i];
    }
    todo->expected_responses = total;

    exchange_numbers_all_to_all(offsets);

    free(numbers);
    return offsets;
}

/** PRIVATE
 * Create the instruction to keep watching.
 *
 * This instruction does nothing by design, but it is telling other processes,
 * "hey, we aren't finished yet!" and keeps them waiting for eventual updates
 * in the future.
 *
 * Returns:     Instruction to keep watching for updates.
 */
struct instruction *instruction_keep_alive(void) {
    struct instruction *ins = malloc(sizeof(struct instruction));

    ins->value = KEEP_ALIVE;
    ins->next  = NULL;
    ins->content[0] = 0;
    ins->content[1] = 0;
    ins->content[2] = 0;
    ins->content[3] = 0;

    return ins;
}

/** PRIVATE
 * Keep the running process alive as long as instructions are still relevant.
 *
 * This function is used before sending messages to other processes. To prevent
 * other processes from thinking that communication has stopped if they don't
 * receive any, a process sends a "I'm not done yet!" instruction that lets
 * other processes know they shouldn't stop listening for instructions.
 *
 * Parameters:
 * - `todo`     todo_list structure.
 */
void keep_todo_list_alive(struct todo_list *todo) {
    uint n = bsp_nprocs();

    // Check if any instruction is sent anywhere.
    for (uint i=0; i<n; i++) {
        if (todo->length[i] > 0) {

            // If so, send an empty instruction to all processes that otherwise
            // wouldn't get any. That way, they know that they're not getting
            // any instructions now, but other processes are.
            for (i=0; i<n; i++) {
                if (todo->length[i] == 0) {
                    add_instruction(instruction_keep_alive(), todo, i);
                }
            }
            break;
        }
    }
}

/** PRIVATE
 * Deregister an instruction and its content from memory.
 *
 * Parameters:
 * - `ins`      instruction structure to free from memory.
 */
void unallocate_instruction(struct instruction *ins) {
    if (ins == NULL) {
        return;
    }

    unallocate_instruction(ins->next);
    free(ins);
}


// ------------------------------------------
// ------------------------------------------
// ------------------------------------------
// ------------------------------------------
// ------------------------------------------

void exchange_instruction_sizes(nid_int *numbers, nid_int value) {
    uint n = bsp_nprocs();
    uint p = bsp_pid();

    bsp_push_reg(numbers, n*sizeof(nid_int));
    bsp_sync();

    for (uint i=0; i<n; i++) {
        bsp_put(i, &value, numbers, p*sizeof(nid_int), sizeof(nid_int));
    }
    bsp_sync();

    bsp_pop_reg(numbers);
}


void exchange_instructions(struct instruction **tasks, nid_int *task_length) {
    uint n = bsp_nprocs();
    uint p = bsp_pid();

    // Broadcast and listen how many instructions each process has
    nid_int *numbers = malloc(n * sizeof(nid_int));
    exchange_instruction_sizes(numbers, *task_length);

    // Prepare receiving new values
    nid_int total_size = sum(numbers, n);
    nid_int offset     = sum(numbers, p);
    free(numbers);

    struct instruction *new_instructions = malloc(total_size * sizeof(struct instruction));
    bsp_push_reg(new_instructions, total_size*sizeof(struct instruction));
    bsp_sync();

    // Exchange instructions
    for (uint i=0; i<n; i++) {
        for (nid_int j=0; j<*task_length; j++) {
            bsp_put(i, tasks[j], new_instructions, offset+j*sizeof(struct instruction), sizeof(struct instruction));
        }
    }
    bsp_sync();
    bsp_pop_reg(new_instructions);


    // Clear memory and replace task list
    for (nid_int i=0; i<*task_length; i++) {
        free(tasks[i]);
    }

    //tasks        = malloc(sizeof(struct instruction *));
    tasks[0]     = new_instructions;
    *task_length = total_size;
}
