/* structures.h
 *
 * This file contains the base structures and integer types that are used
 * throughout the program.
 **/

// uint
//
// This value should be at least as large as:
// - the number of PIDs.
typedef unsigned int uint;

// nid_int
//
// This value should be at least as large as:
// - the number of nodes    in the input graph;
// - the number of vertices in the input graph.
//
// NOTE: Make sure that you also update the scanf function that looks for
// these values and casts them into variables.
typedef unsigned int nid_int;

struct instructions;

struct neighbour;

struct node;

/**
 * Snake
 *
 * The snake structure slithers through the graph, looking for other
 * snakes to eat. It is the main component of the 2nd part of the
 * algorithm.
 *
 * Attributes:
 * - `local_head`    Node at the head of the local snake.
 * - `local_belly`   Nodes between the head and the tail of the local snake.
 * - `local_tail`    Node at the tail of the local snake.
 * - `next_node`     Unique node identifier of the tail that this snake bites.
 * - `previous_node` Unique node identifier of the head that bites this snake.
 * - `base`          Unique node identifier of the global snake's tail.
 */
struct snake;
