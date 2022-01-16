// PRIVATE FUNCTIONS
void debug_bsp_start(void);
void debug_bsp_end(void);

/**
 * Make sure each PID has got the message and is waiting for the correct
 * amount of edges.
 *
 * Parameters:
 * - `edges`    The amount of edges this process is expecting.
 */
void debug_expected_amount_of_edges(nid_int edges) {
    debug_bsp_start();

    printf("[PID %u] Expecting %u edges.\n", bsp_pid(), edges);

    debug_bsp_end();
}

/**
 * Make sure each PID has received the correct edges.
 *
 * Parameters:
 * - `edges`        Array of received edges.
 * - `edges_length` Array length of parameter `edges`.
 */
void debug_expected_edges(nid_int (*edges)[2], nid_int edges_length) {
    debug_bsp_start();

    for (nid_int i=0; i<edges_length; i++) {
        printf("[PID %u] Received edge:\t%u %u\n", bsp_pid(), 
                edges[i][0] + 1, edges[i][1] + 1);
    }

    debug_bsp_end();
}

void debug_graph_setup(struct graph *g) {
    debug_bsp_start();

    uint p = bsp_pid();

    printf("[PID %u] ========================\n",  p);
    printf("[PID %u] GRAPH STRUCTURE OF PID %u\n", p, p);
    printf("[PID %u] Global degree = %3u\n",       p, g->global_degree);
    printf("[PID %u] Local degree  = %3u\n",       p, g->local_degree);
    printf("[PID %u] Max size      = %3u\n",       p, g->max_size);
    printf("[PID %u] Matches       = %3u\n",       p, g->m->length);
    printf("[PID %u]\n",                           p);
    printf("[PID %u] Nodes:\n",                    p);

    for (nid_int i=0; i<g->local_degree; i++) {
        struct node *nd = g->vertex[i];

        printf("[PID %u] - NODE %u\n",             p, nd->value);
        printf("[PID %u]     degree      = %u\n",  p, nd->degree);
        printf("[PID %u]     connections = [ ",    p);
        for (nid_int j=0; j<nd->degree; j++) {
            printf("%u ", nd->connections[j]);
        }
        printf("]\n[PID %u]\n", p);
    }

    printf("[PID %u]\n",                           p);
    printf("[PID %u] Matches:\n",                  p);
    
    for (nid_int i=0; i<g->m->length; i++) {
        printf("[PID %u] - MATCH  [ %3u %3u ]\n",  p, 
                g->m->matches[i][0], g->m->matches[i][1]);
    }

    printf("[PID %u]\n[PID %u]\n", p, p);

    debug_bsp_end();
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
 * Run this function at the start of a function. It makes sure all processes
 * send their debug output to stdout in order of PID.
 */
void debug_bsp_start(void) {
    uint p = bsp_pid();

    for (uint i=0; i<p; i++) {
        bsp_sync();
    }
}

/** PRIVATE
 * Run this function at the end of a function. It makes sure all processes
 * send their debug output to stdout in order of PID.
 */
void debug_bsp_end(void) {
    uint n = bsp_nprocs();
    uint p = bsp_pid();

    for (uint i=p; i<n; i++) {
        bsp_sync();
    }

    bsp_sync();
}