uint divide(nid_int node, nid_int total_nodes, uint processes) {
    return cyclic_distribution(node, total_nodes, processes);
}

uint block_distribution(nid_int node, nid_int total_nodes, uint processes) {
    return node * processes / total_nodes;
}

uint cyclic_distribution(nid_int node, nid_int total_nodes, uint processes) {
    return node % processes;
}