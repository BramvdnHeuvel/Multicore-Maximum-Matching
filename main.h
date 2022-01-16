
typedef unsigned int uint;
// This value should be at least as large as:
// - the number of PIDs.

typedef unsigned int nid_int;
// This value should be at least as large as:
// - the number of PIDs;
// - the number of nodes    in the input graph;
// - the number of vertices in the input graph.
//
// NOTE: Make sure that you also update the scanf function that looks for
// these values and casts them into variables.


struct graph;
struct instruction;
struct matching;
struct node;
struct todo_list;


struct graph {
    struct node **vertex;

    struct matching *m;

    nid_int max_size;
    nid_int local_degree;
    nid_int global_degree;
};

/**
 * Instruction structure that tells another process to perform an operation.
 *
 * Attributes:
 * - `value`    Unique identifier for a pre-defined instruction.
 * - `content`  Node identifiers that specify the operation.
 * - `next`     Pointer to another instruction, linked list style. This pointer
 *              is used in a todo-list and is ignored as soon as the instruction
 *              has been sent to another process.
 */
struct instruction {
    short int value;

    nid_int content[4];

    struct instruction *next;
};

struct matching {
    nid_int (*matches)[2];

    nid_int length;
    nid_int max_length;
};

struct node {
    nid_int  value;

    nid_int *connections;
    nid_int  degree;
};

/**
 * To do list that tracks all instructions that will be sent to neighbours.
 *
 * Attributes:
 * - `length`       Amount of instructions that will be sent to each process.
 * - `list_proc`    Linked list of all instructions that will be sent to the
 *                  respective processes.
 * - `latest`       Array of pointers to the instructions at the end of the
 *                  linked list. Speeds up insertion.
 */
struct todo_list {
    nid_int *length;

    struct instruction **p_list;
    struct instruction **latest;

    nid_int expected_responses;
};