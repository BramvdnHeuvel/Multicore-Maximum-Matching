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

/**
 * Create a node structure based on an array of edges, where some edges may not
 * even be relevant for the node.
 *
 * Parameters:
 * - `n`            Node identifier to be created.
 * - `edges`        Array of 2-integer arrays representing the graph's edges.
 * - `edges_length` Array length of parameter `edges`.
 *
 * Returns:         Pointer to the newly created node.
 */
struct node *create_node_from_array(nid_int n, nid_int (*edges)[2], nid_int edges_length) {
    nid_int *connections = malloc(edges_length * sizeof(nid_int));
    nid_int connections_length = 0;
    
    for (nid_int i=0; i<edges_length; i++) {
        nid_int node_one = edges[i][0];
        nid_int node_two = edges[i][1];

        if (node_one == n) {
            connections[connections_length] = node_two;
            connections_length++;
        } else if (node_two == n) {
            connections[connections_length] = node_one;
            connections_length++;
        }
    }

    struct node *answer = create_node(n, connections, connections_length);
    free(connections);

    return answer;
}

/**
 * Fill in an array of nodes based on the edges we received 
 * from the initialization phase.
 *
 * Parameters:
 * - `nodes`        Array of nodes to fill in.
 * - `nodes_length` Pointer to the integer that keeps track of the array length
 *                      of parameter `nodes`.
 * - `edges`        Array of edges received from initialization phase.
 * - `edges_length` Array length of parameter `edges`.
 * - `total_nodes`  Amount of nodes in the entire graph.
 */
void initialize_nodes(struct node **nodes, nid_int *nodes_length, 
                      nid_int (*edges)[2], nid_int  edges_length, 
                      nid_int total_nodes) {
    uint n = bsp_nprocs();
    uint p = bsp_pid();

    for (nid_int i=0; i<edges_length; i++) {
        for (short int j=0; j<2; j++) {
            nid_int new_value = edges[i][j];

            if (divide(new_value, total_nodes, n) == p) {
                // The node belongs to this process

                if (!already_exists(new_value, nodes, *nodes_length)) {
                    // The node hasn't been initialized yet.
                    nodes[*nodes_length] = create_node_from_array(new_value, 
                                            &(edges[i]), edges_length-i);
                    *nodes_length = *nodes_length + 1;
                }
            }
        }
    }
}

void remove_node(nid_int n, struct node **nodes, nid_int *nodes_length, nid_int total_nodes) {
    uint p = bsp_pid();

    struct node *nd;
    nid_int i;
    bool node_found = false;

    // Locate node
    for (i=0; i<*nodes_length; i++) {
        if (nodes[i]->value == n) {
            node_found = true;
            nd = nodes[i];

            break;
        }
    }

    // Finish if the node doesn't exist
    if (!node_found) {
        printf("[PID %u] WARNING: No node %u found to remove\n", p, n);
        return;
    }

    // Take the node out of the array
    *nodes_length = *nodes_length - 1;
    for (; i<*nodes_length; i++) {
        nodes[i] = nodes[i+1];
    }

    // Remove the connection from other nodes
    bool announced_publicly = false;
    uint np = bsp_nprocs();

    for (nid_int j=0; j<nd->degree; j++) {
        nid_int neighbour = nd->connections[j];

        if (divide(neighbour, total_nodes, np) == p) {
            // Locate node
            struct node *other;

            for (nid_int i=0; i<*nodes_length; i++) {
                other = nodes[i];

                if (nd->connections[j] == other->value) {
                    remove_edge(nd, other->value);
                    remove_edge(other, nd->value);
                    break;
                }
            }
        } else {
            // Node is in another process
            
        }

    }

    for (i=0; i<*nodes_length; i++) {
        struct node *other = nodes[i];


        if (nd->degree == 0) {
            break;
        }
    }

    unallocate_node(nd);
}

bool eliminate_irrelevant_nodes(struct node **nodes, nid_int *nodes_length, 
                                   struct instruction **tasks, nid_int *task_length, 
                                   nid_int total_nodes) {
    uint n = bsp_nprocs();
    uint p = bsp_pid();
    nid_int i = 0;

    for (; i<*nodes_length; i++) {
        struct node *nd = nodes[i];

        if (nd->degree == 0) {
            unallocate_node(nd);
            break;
        } else if (nd->degree == 1) {
            nid_int neighbour = nd->connections[0];
            uint neighbour_p  = divide(neighbour, total_nodes, n);

            if (neighbour == p) {
                instruction_delete_node(tasks, task_length, neighbour);
            } else {
                instruction_move_node(tasks, task_length, nd->value, neighbour);
            }

            unallocate_node(nd);
        }
    }

    *nodes_length = *nodes_length - 1;

    for (; i<*nodes_length; i++) {
        nodes[i] = nodes[i+1];
    }

    return true;
}

void debug_instruction_response(struct instruction *response) {
    uint n = bsp_nprocs();
    
    for (uint i=0; i<n; i++) {
        if (i==p) {
            printf("==================\nPID %u received these instructions:\n", p);

            for (nid_int j=0; j<todo->expected_responses; j++) {
                show_instruction(response[j]);
            }
        }
        bsp_sync();
    }
}
