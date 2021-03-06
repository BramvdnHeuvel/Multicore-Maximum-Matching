#include "divide.h"
#include "graph.h"
#include "instructions.h"

/**
 * Ask for the graph's size on stdin.
 *
 * Parameters:
 * - `nodes`    Integer that will store the amount of vertices in the graph.
 * - `edges`    Integer that will store the amount of edges in the graph.
 */
void prompt_graph_size(nid_int *nodes, nid_int *edges) {
    printf("Let's initialize the graph! Give me the graph data, please.\n");
    fflush(stdout);
    scanf("%u %u", nodes, edges);
}

/**
 * Ask for an edge on stdin.
 *
 * Parameters:
 * - `node_one` Integer that will store the first node.
 * - `node_two` Integer that will store the second node.
 */
void prompt_edge(nid_int *node_one, nid_int *node_two) {
    fflush(stdout);
    scanf("%u %u", node_one, node_two);
}

/**
 * Ask for all edges in the graph.
 *
 * Parameters:
 * - `edges`        Array that will store all the edges.
 * - `total_edges`  How many edges are expected.
 */
void prompt_edges(nid_int (*edges)[2], nid_int total_edges) {
    for (nid_int i=0; i<total_edges; i++) {
        nid_int node_one;
        nid_int node_two;

        prompt_edge(&node_one, &node_two);

        edges[i][0] = node_one - 1;
        edges[i][1] = node_two - 1;
    }
}

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
void broadcast_node_amount(nid_int total_nodes, nid_int *sync_number) {
    uint n = bsp_nprocs();

    // Initialize table
    nid_int *node_distr = malloc(n * sizeof(nid_int));
    for (uint i=0; i<n; i++) {
        node_distr[i] = 0;
    }

    // Count how many vertices each process receives
    for (nid_int i=0; i<total_nodes; i++) {
        node_distr[divide(i, total_nodes, n)]++;
    }

    // Send the amounts
    for (uint i=0; i<n; i++) {
        bsp_put(i, &(node_distr[i]), sync_number, 0, sizeof(nid_int));
    }

    free(node_distr);
}

/**
 * Broadcast the total amount of numbers that the graph has.
 *
 * Parameters:
 * - `sync_number`  The pushed BSP register where the number may be stored.
 *                  This value is expected to store the value already in the
 *                  process where you're executing this function.
 */
void broadcast_total_node_amount(nid_int *sync_number) {
    uint n = bsp_nprocs();

    for (uint i=0; i<n; i++) {
        bsp_put(i, sync_number, sync_number, 0, sizeof(nid_int));
    }
}

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
                           nid_int total_nodes, nid_int *sync_number) {
    uint n = bsp_nprocs();

    // Initialize edge table
    nid_int *edge_distr = malloc(n * sizeof(nid_int));
    for (uint i=0; i<n; i++) {
        edge_distr[i] = 0;
    }

    // Count processes for which the edge is relevant
    for (nid_int i=0; i<total_edges; i++) {
        nid_int node_one = edges[i][0];
        nid_int node_two = edges[i][1];

        uint p1 = divide(node_one, total_nodes, n);
        uint p2 = divide(node_two, total_nodes, n);

        edge_distr[p1]++;
        if (p2 != p1) {
            edge_distr[p2]++;
        }
    }

    // Send the amounts to the respective processes
    for (uint i=0; i<n; i++) {
        bsp_put(i, &(edge_distr[i]), sync_number, 0, sizeof(nid_int));
    }

    free(edge_distr);
}

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
                nid_int total_nodes, nid_int (*sync_array)[2]) {
    uint n = bsp_nprocs();

    // Keep track of how many edges have been broadcast yet.
    nid_int *edges_found = malloc(n * sizeof(nid_int));
    for (uint i=0; i<n; i++) {
        edges_found[i] = 0;
    }

    for (nid_int i=0; i<total_edges; i++) {
        nid_int node_one = edges[i][0];
        nid_int node_two = edges[i][1];

        uint p1 = divide(node_one, total_nodes, n);
        uint p2 = divide(node_two, total_nodes, n);

        bsp_put(p1, &(edges[i]), sync_array, edges_found[p1]*2*sizeof(nid_int),
                2*sizeof(nid_int));
        edges_found[p1]++;

        if (p2 != p1) {
            bsp_put(p2, &(edges[i]), sync_array, edges_found[p2]*2*sizeof(nid_int),
                2*sizeof(nid_int));
            edges_found[p2]++;
        }
    }

    free(edges_found);
}

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
                                           nid_int total_nodes) {
    uint n = bsp_nprocs();
    uint p = bsp_pid();

    nid_int *neighbours = malloc(local_edges * sizeof(nid_int));
    nid_int neighbours_length = 0;

    for (nid_int i=0; i<local_edges; i++) {
        nid_int node_one = edges[i][0];
        nid_int node_two = edges[i][1];

        uint p1 = divide(node_one, total_nodes, n);
        uint p2 = divide(node_two, total_nodes, n);

        for (short int j=0; j<2; j++) {
            bool    already_exists = false;
            nid_int node           = edges[i][j];
            uint    p              = divide(node, total_nodes, n);

            for (nid_int k=0; k<neighbours_length; k++) {
                if (neighbours[k] == node_one) {
                    already_exists = true;
                    break;
                }
            }

            if (!already_exists) {
                neighbours[neighbours_length] = node_one;
                neighbours_length++;
            }
        }
    }

    free(neighbours);
    return neighbours_length + local_nodes;
}

/**
 * Check whether a node of a certain ID already exists in the array of nodes.
 *
 * Parameters:
 * - `n`                Unique node identifier.
 * - `nodes`            Array of pointers to node structures.
 * - `nodes_in_length`  Array length of parameter `nodes`.
 *
 * Returns:             Boolean whether the identifier `n` was found 
 *                      in the array `nodes`
 */
bool already_exists(nid_int n, struct node **nodes, nid_int nodes_length) {
    if (nodes_length == 0) {
        return false;
    } else if (nodes[0]->value == n) {
        return true;
    } else {
        for (nid_int i=1; i<nodes_length; i++) {
            if (nodes[i]->value == n) {
                // Heuristic optimization: move vertices of high degree
                //                         to the front of the array.
                struct node *temp = nodes[i];
                nodes[i]   = nodes[i-1];
                nodes[i-1] = temp;

                return true;
            }
        }
        return false;
    }
}

void debug_instruction_response(struct instruction *response, 
                                nid_int response_length) {
    uint n = bsp_nprocs();
    uint p = bsp_pid();
    
    for (uint i=0; i<n; i++) {
        if (i==p) {
            printf("==================\nPID %u received these instructions:\n", p);

            for (nid_int j=0; j<response_length; j++) {
                show_instruction(response[j]);
            }
        }
        bsp_sync();
    }
}
