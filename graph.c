#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "snake.h"
#include "graph.h"
#include "utilities.h"

// Individual vertex in the graph
struct node {
    uint value;         // Node identifier

    // Which nodes are we connected to?
    uint *connectedTo;  // Which ones?
    uint  connections;  // How many?

    // Snake operations
    bool eaten;         // Whether the node has been eaten by a snake
    bool head;          // Whether the node is a head /\_ 
    bool tail;          // Whether the node is a tail [] 
    struct snake *eaten_by; // Snake that the node is part of
};

struct graph {
    struct node **nodes;
    int length;
};

// Create a node for the graph
//
// Parameters:
// n            Unique identifier to compare with other nodes
// connections  Other node identifiers the node is connected to
// length       
struct node *create_node(uint n, uint maxDegree) {
    struct node *vertex = malloc(sizeof(struct node));

    vertex->value       = n;
    
    vertex->connections = 0;
    vertex->connectedTo = malloc(maxDegree * sizeof(uint));

    vertex->eaten    = false;
    vertex->head     = false;
    vertex->tail     = false;
    vertex->eaten_by =  NULL;

    return vertex;
}

void connect_nodes(struct node *a, struct node *b) {
    a->connectedTo[a->connections] = b->value;
    a->connections++;

    b->connectedTo[b->connections] = a->value;
    b->connections++;
}

struct graph *initialize_graph() {
    uint amountOfNodes;
    uint amountOfEdges;

    printf("Let's initialize the graph! Give us the data on the graph, please.\n");
    fflush(stdout);
    scanf("%u %u", &amountOfNodes, &amountOfEdges);
    uint maxDegree = max(amountOfNodes, amountOfEdges);

    struct graph *g = malloc(sizeof(struct graph));
    g->nodes        = malloc(sizeof(struct node *) * amountOfNodes);
    g->length       = amountOfNodes;
    
    // Create the nodes
    for (uint i=0; i<amountOfNodes; i++) {
        g->nodes[i] = create_node(i, maxDegree);
    }

    // Connect the nodes
    for (uint i=0; i<amountOfEdges; i++) {
        uint node_a;
        uint node_b;
        scanf("%u %u", &node_a, &node_b);

        connect_nodes(g->nodes[node_a-1], g->nodes[node_b-1]);
    }

    return g;
}

struct graph **divide_graph(struct graph *g, uint processes) {
    struct graph **subs = malloc(processes * sizeof(struct graph *));

    for (uint i=0; i<processes; i++) {
        printf("Initializing graph %u...\n", i);
        subs[i] = malloc(sizeof(struct graph));
        subs[i]->length = 0;

        // Maximum amount of nodes per subgraph
        // If unsure, type the following:
        // subs[i]->nodes = malloc(g->length * sizeof(struct node *));
        subs[i]->nodes = malloc((g->length/processes+1) * sizeof(struct node *));
        printf("Subgraph %u can take up to %u nodes.\n", i, (g->length/processes+1));
    }

    for (uint i=0; i<g->length; i++) {
        uint nodeDistribution  = cyclic_distribution(i, g->length, processes);

        printf("Assigning node %u to subgraph %u...\n", i, nodeDistribution);
        struct graph *appointee = subs[nodeDistribution];

        appointee->nodes[appointee->length] = g->nodes[i];
        appointee->length++;
    }

    return subs;
}

void show_graph(struct graph *g) {
    printf("GRAPH OF LENGTH %u\n--------------\n", g->length);
    for (uint i=0; i<g->length; i++) {
        show_node(g->nodes[i]);
    }
}

void show_node(struct node *n) {
    printf("[\n");
    printf("    [%u]\n", n->value);

    if (n->eaten)     {printf("    eaten\n");}
    else if (n->head) {printf("    head\n");}
    else if (n->tail) {printf("    tail\n");} // Smashes skateboard
    else              {printf("    not eaten\n");}

    printf("    conn. with [ ");
    for (uint i=0; i<n->connections; i++) {
        printf("%u ", n->connectedTo[i]);
    }
    printf("]\n");
    printf("]\n");
}
