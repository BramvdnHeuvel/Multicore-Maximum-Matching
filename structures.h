/* structures.h
 *
 * This file contains the base structures and integer types that are used
 * throughout the program.
 **/



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

struct instruction;

struct node *create_node(nid_int value, nid_int *endpoints,
                         nid_int edges_length);

void remove_edge(struct node *nd, nid_int e);

void unallocate_node(struct node *nd);

void show_node(struct node *nd);
