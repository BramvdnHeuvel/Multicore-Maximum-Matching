#include "divide.h"

// PRIVATE FUNCTIONS
uint block_distribution(nid_int node, nid_int total_nodes, uint processes);
uint cyclic_distribution(nid_int node, nid_int total_nodes, uint processes);

/**
 * Divide nodes across processes by a certain independent algorithm.
 *
 * This algorithm may be altered for experimental purposes.
 *
 * Parameters:
 * - `node`         Unique node identifier.
 * - `total_nodes`  Amount of vertices in the graph.
 * - `processes`    Amount of processes in the program.
 *
 * Returns:         The process to which the node with ID `node` belongs.
 */
uint divide(nid_int node, nid_int total_nodes, uint processes) {
    return block_distribution(node, total_nodes, processes);
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
 * Distribute a graph into subgraphs through block distribution.
 *
 * A graph with nodes 1-9 would be distributed across 3 processes (A,B,C)
 * as follows:
 *
 *   1 2 3 4 5 6 7 8 9
 *   ----- ----- -----
 *     A     B     C  
 *
 * Parameters:
 * - `node`         Unique node identifier.
 * - `total_nodes`  Amount of vertices in the graph.
 * - `processes`    Amount of processes in the program.
 *
 * Returns:         The process to which the node with ID `node` belong
 */
uint block_distribution(nid_int node, nid_int total_nodes, uint processes) {
    return node * processes / total_nodes;
}

/** PRIVATE
 * Distribute a graph into subgraphs through cyclic distribution.
 *
 * A graph with nodes 1-9 would be distributed across 3 processes (A,B,C)
 * as follows:
 *
 *   1 2 3 4 5 6 7 8 9
 *   - - - - - - - - -
 *   A B C A B C A B C
 *
 * Parameters:
 * - `node`         Unique node identifier.
 * - `total_nodes`  Amount of vertices in the graph.
 * - `processes`    Amount of processes in the program.
 *
 * Returns:         The process to which the node with ID `node` belong
 */
uint cyclic_distribution(nid_int node, nid_int total_nodes, uint processes) {
    return node % processes;
}

