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
* Create a new graph structure with default settings.
*
* Parameters:
* - `maxNodes`      The maximum amount of nodes you're expecting the graph
                        to contain.
*
* Returns:          Pointer to the newly created graph.
*/
struct graph *create_graph(uint maxNodes) {
    struct graph *g = malloc(sizeof(struct graph));

    g->nodes  = malloc(maxNodes * sizeof(struct node *));
    g->length = 0;

    return g;
}

/**
* Create multiple (sub)graphs.
*
* This function is generally used to split a graph up into multiple
* subgraphs so that the graph can be distributed across processes
* without any overlap of nodes.
*
* Parameters:
* - `n`                 Amount of (sub)graphs to create
* - `maxNodesPerGraph`  Maximum amount of nodes each graph may contain
*
* Returns:              Pointer to an array of pointers to graphs
*/
struct graph **create_subgraphs(uint n, uint maxNodesPerGraph) {
    struct graph **subs = malloc(n * sizeof(struct graph *));

    for (uint i=0; i<n; i++) {
        subs[i] = create_graph(maxNodesPerGraph);
    }

    return subs;
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
* Add an existing node to a (sub)graph.
*
* Parameters:
* - `n`         Node to add.
* - `g`         Graph to add the node to.
*/
void add_to_graph(struct node *n, struct graph *g) {
    g->nodes[g->length] = n;
    g->length++;
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

    struct graph *g = create_graph(amountOfNodes);
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
