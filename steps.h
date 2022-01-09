/**
 * Ask for the graph's size on stdin.
 *
 * Parameters:
 * - `nodes`    Integer that will store the amount of vertices in the graph.
 * - `edges`    Integer that will store the amount of edges in the graph.
 */
void prompt_graph_size(nid_int *nodes, nid_int *edges);

/**
 * Ask for an edge on stdin.
 *
 * Parameters:
 * - `node_one` Integer that will store the first node.
 * - `node_two` Integer that will store the second node.
 */
void prompt_edge(nid_int *node_one, nid_int *node_two);

/**
 * Ask for all edges in the graph.
 *
 * Parameters:
 * - `edges`        Array that will store all the edges.
 * - `total_edges`  How many edges are expected.
 */
void prompt_edges(nid_int (*edges)[2], nid_int total_edges);

/**
 * Broadcast to each process how many nodes they should expect.
 *
 * This functions helps gain an accurate estimate about how much memory should
 * be allocated by each process.
 *
 * Parameters:
 * - `total_nodes`  How many nodes the graph has in total.
 * - `sync_number`  The pushed BSP register where each number may be stored.
 */
void broadcast_node_amount(nid_int total_nodes, nid_int *sync_number);

/**
 * Broadcast the total amount of numbers that the graph has.
 *
 * Parameters:
 * - `sync_number`  The pushed BSP register where the number may be stored.
 *                  This value is expected to store the value already in the
 *                  process where you're executing this function.
 */
void broadcast_total_node_amount(nid_int *sync_number);

/**
 * Broadcast to each process how many edges they should expect.
 *
 * This functions helps gain an accurate estimate about how much memory should
 * be allocated by each process.
 *
 * Parameters:
 * - `edges`        Array that stores every edge in the graph.
 * - `total_edges`  How many edges the graph has in total.
 * - `total_nodes`  How many nodes the graph has in total.
 * - `sync_number`  The pushed BSP register where each number may be stored.
 */
void broadcast_edge_amount(nid_int (*edges)[2], nid_int total_edges,
                           nid_int total_nodes, nid_int *sync_number);

/**
 * Broadcast all edges to the relevant processes.
 *
 * Parameters:
 * - `edges`        Array that stores every edge in the graph.
 * - `total_edges`  How many edges the graph has in total.
 * - `total_nodes`  How many nodes the graph has in total.
 * - `sync_number`  The pushed BSP register where every edge may be stored.
 */
void send_edges(nid_int (*edges)[2], nid_int total_edges, 
                nid_int total_nodes, nid_int (*sync_array)[2]);

/**
 * Calculate the maximum amount of nodes that may be expected in the process
 * based on the edges we have available.
 *
 * This number may differ from the amount of nodes that is initially given to
 * the process. It may be lower as the process ignores vertices of degree 0.
 * (Such vertices by definition do not match.) It may also be higher as the
 * process receives vertices of degree 1 from other processes if their only
 * neighbour is in the concerning the process.
 *
 * Parameters:
 * - `edges`        Array that stores every edge in the graph.
 * - `local_nodes`  How many nodes this process has.
 * - `local_edges`  How many edges this process has.
 * - `total_nodes`  How many nodes the graph has in total.
 */
nid_int calculate_maximum_nodes_in_process(nid_int (*edges)[2], 
                                           nid_int local_nodes, 
                                           nid_int local_edges, 
                                           nid_int total_nodes);