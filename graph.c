#include "graph.h"

/**
* Vertex structure in a (sub)graph.
*
* Attributes:
* - `value`         A unique identifier that distinguishes the node.
* - `connectedTo`   An array of integers that are identifiers to the nodes that
*                       this node is connected to.
* - `connections`   Array length of the `connectedTo` attribute.
* - `eaten`         Boolean whether the node has been eaten by a snake yet.
* - `head`          Boolean whether the node is the head of a snake looking for
                        a tail to bite on.
* - `tail`          Boolean whether the node is a tail that can be bitten.
* - `eaten_by`      Pointer to the local snake that this node is part of.
*/
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

/**
* Graph structure to contain a collection of nodes
*
* Attributes:
* - `nodes`         Array of pointers to node structures that are part
                        of the graph.
* - `length`        Array length of the `nodes` attribute.
*/
struct graph {
    struct node **nodes;
    int length;
};

/**
* Create a new node structure with default settings.
*
* Parameters:
* - `n`             Unique identifier of the node.
* - `maxDegree`     The maximum number of connections this node may have.
*
* Returns:          Pointer to the newly created node.
*/
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

/**
* Connect two nodes to each other. The operation is symmetrical, 
* so the order of the two nodes does not matter.
*
* Parameters:
* - `a`             Node structure 1
* - `b`             Node structure 2
*/
void connect_nodes(struct node *a, struct node *b) {
    a->connectedTo[a->connections] = b->value;
    a->connections++;

    b->connectedTo[b->connections] = a->value;
    b->connections++;
}

/**
* Create a new graph structure based on stdin.
*
* The function first expects two numbers: one for the amount of vertices V(G),
* and one for the amount of vertices E(G).
* Then, the following E(G) lines contain two numbers between 1 and V(G),
* which indicate one of the edges between two nodes.
*
* Take a look at the following input, for example:
*
*       > 6 7
*       > 1 2
*       > 2 3
*       > 3 4
*       > 3 5
*       > 4 5
*       > 4 6
*       > 5 6
*
* This input would create a graph that looks like the following visual example:
*
*       (1)   (3)---(5)
*         \   / \   / \
*          \ /   \ /   \
*          (2)   (4)---(6)
*
* Returns:          Pointer to the newly created graph
*/
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

/**
* Divide a graph into multiple subgraphs.
* 
* Dividing the graph into subgraphs allows multicore processes to each
* claim their own segment of the graph. This function determines how
* the nodes are distributed, so rewrite this function if you wish to
* alter the way the nodes are distributed.
*
* Parameters:
* - `g`         Graph structure that needs to be divided
* - `processes` Amount of subgraphs that need to be created
*
* Returns:      Pointer to an array of pointers to the newly created subgraphs
*/
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

/**
* Print a graph's representation to stdout.
*
* This function has no production use, but it helps debug the structure
* and makes it easier to traverse the (sub)graph.
*
* Parameters:
* - `g`         Graph that needs to be shown
*/
void show_graph(struct graph *g) {
    printf("GRAPH OF LENGTH %u\n--------------\n", g->length);
    for (uint i=0; i<g->length; i++) {
        show_node(g->nodes[i]);
    }
}

/**
* Print a node's representation to stdout.
*
* This function has no production use, but it helps debug the structure
* and makes it easier to inspect any vertex.
*
* Parameters:
* - `n`         Node that needs to be shown
*/
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
