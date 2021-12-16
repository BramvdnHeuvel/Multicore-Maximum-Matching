/**
* divider.h
* ---
* Dividing the graph into subgraphs allows multicore processes to each
* claim their own segment of the graph. This file declares how nodes
* can be distributed.
*/

#include "graph.h"


/**
* Distribute a graph into subgraphs through cyclic distribution.
*
* A graph with nodes 1-9 would be distributed across 3 processes (A,B,C)
* as follows:
*
*   1 2 3 4 5 6 7 8 9
*   - - - - - - - - -
*   A B C A B C A B C
*
*
* Parameters:
* - `g`         Graph structure that needs to be divided
* - `processes` Amount of subgraphs that need to be created
*
* Returns:      Pointer to an array of pointers to the newly created subgraphs
*/
struct graph **divide_graph_cyclic(struct graph *g, uint processes);

/**
* Distribute a graph into subgraphs through block distribution.
*
* A graph with nodes 1-9 would be distributed across 3 processes (A,B,C)
* as follows:
*
*   1 2 3 4 5 6 7 8 9
*   ----- ----- -----
*     A     B     C  
*
*
* Parameters:
* - `g`         Graph structure that needs to be divided
* - `processes` Amount of subgraphs that need to be created
*
* Returns:      Pointer to an array of pointers to the newly created subgraphs
*/
struct graph **divide_graph_block(struct graph *g, uint processes);

/**
* Distribute a graph into subgraphs through path distribution.
*
* Path distribution means that a greedy algorithm distributed the
* graph by greedily looking for paths on the graph.
* 
* Pros:
*   - The nodes are already distributed conveniently across processes;
*   - Each process ends up with a relatively low amount of snakes;
*   - Snake are less likely to be "at the border" between several processes
*       where the snake slithers back and forth between processes.
*
* Cons:
*   - Processes may have radically different amount of nodes;
*   - Some processes may not receive any nodes at all;
*
* Suppose a graph with 9 nodes that looks like the following:
*
*       (1)   (4)---(7)   (9)
*         \   / \         /
*          \ /   \       /
*          (2)   (5)   (8)
*            \     \   /
*             \     \ /
*             (3)---(6)
*
* This graph would be distributed by starting at the lowest unpicked value
* and then adding its neighbour with the lowest (arbitrary) value. When all
* neighbours are picked, we move on to another node and the next process.
*
*   Process A : (1)---(2)---(3)---(6)---(5)---(4)---(7) 
*                       \           \
*                        \           \
*                        (4)--       (8)--
*                          \
*
*   Process B : (8)---(9)
*
*   Process C : empty
*
* Note that the distribution is arbitrary because the numbers are arbitrary.
* If the graph is the same but the numbers are arranged differently, let's say
* like the following:
*
*       (1)   (2)---(3)   (4)
*         \   / \         /
*          \ /   \       /
*          (5)   (6)   (7)
*            \     \   /
*             \     \ /
*             (8)---(9)
*
* Then the distribution would look like this:
*
*   Process A : (1)---(5)---(2)---(3)
*   Process B : (6)---(9)---(7)---(4)
*   Process C : (8)
*
*
* Parameters:
* - `g`         Graph structure that needs to be divided
* - `processes` Amount of subgraphs that need to be created
*
* Returns:      Pointer to an array of pointers to the newly created subgraphs
*/
struct graph **divide_graph_path(struct graph *g, uint processes);
