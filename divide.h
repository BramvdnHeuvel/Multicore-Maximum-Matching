
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
uint divide(nid_int node, nid_int total_nodes, uint processes);
