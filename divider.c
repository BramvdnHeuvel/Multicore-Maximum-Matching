#include "divider.h"

/**
* Distribute a graph into subgraphs through cyclic distribution.
*
* Refer to file `divider.h` for more documentation.
*/
struct graph **divide_graph_cyclic(struct graph *g, uint processes) {
    struct graph **subs = create_subgraphs(processes, g->length/processes + 1);

    for (uint i=0; i<g->length; i++) {
        uint nodeDistribution = cyclic_distribution(i, g->length, processes);
        add_to_graph(g->nodes[i], subs[nodeDistribution]);
    }
    
    return subs;
}

/**
* Distribute a graph into subgraphs through block distribution.
*
* Refer to file `divider.h` for more documentation.
*/
struct graph **divide_graph_block(struct graph *g, uint processes) {
    struct graph **subs = create_subgraphs(processes, g->length/processes + 1);

    for (uint i=0; i<g->length; i++) {
        uint nodeDistribution = block_distribution(i, g->length, processes);
        add_to_graph(g->nodes[i], subs[nodeDistribution]);
    }
    
    return subs;
}

/**
* Distribute a graph into subgraphs through path distribution.
*
* Refer to file `divider.h` for more documentation.
*/
struct graph **divide_graph_path(struct graph *g, uint processes) {
    struct graph **subs = create_subgraphs(processes, g->length);
    uint p = 0;

    bool *used = malloc(g->length * sizeof(bool));
    for (uint i=0; i<g->length; i++) {
        used[i] = false;
    }

    for (uint i=0; i<g->length; i++) {
        while (!used[i]) {
            add_to_graph(g->nodes[i], subs[p]);
            printf("Added node %u to subgraph %u\n", i, p);
            used[i] = true;

            for (uint j=0; j<g->nodes[i]->connections; j++) {
                uint neighbour = g->nodes[i]->connectedTo[j];
                if (!used[neighbour]) {
                    i = neighbour;
                    break;
                }
            }

        }
        p++;
        p = p % processes;
    }

    return subs;
}