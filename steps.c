void prompt_graph_size(nid_int *nodes, nid_int *edges) {
    printf("Let's initialize the graph! Give me the graph data, please.\n");
    fflush(stdout);
    scanf("%u %u", nodes, edges);
}

void prompt_edge(nid_int *node_one, nid_int *node_two) {
    fflush(stdout);
    scanf("%u %u", node_one, node_two);
}

void prompt_edges(nid_int (*edges)[2], nid_int total_edges) {
    for (nid_int i=0; i<total_edges; i++) {
        nid_int node_one;
        nid_int node_two;

        prompt_edge(&node_one, &node_two);

        edges[i][0] = node_one - 1;
        edges[i][1] = node_two - 1;
    }
}

void *broadcast_node_amount(nid_int total_nodes, nid_int *sync_number) {
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
