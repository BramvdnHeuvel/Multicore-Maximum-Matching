#include "graph.h"


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
 * Deepcopy a graph. 
 *
 * If you are unsure whether the processes are waiting for each other to
 * read data outside of their SPMDs, you can create a copy of a graph
 * with this function.
 *
 * This way, all relevant data for a process remains within its thread.
 *
 * Parameters:
 * - `g`        A graph structure that needs to be copied
 *
 * Returns:     An exact replica of graph `g`, yet all pointers point to
 *                  newly allocated memory.
 */
struct graph *deepcopy_graph(struct graph *g) {
    struct graph *new_g = create_graph(g->length);

    for (uint i=0; i<g->length; i++) {
        new_g->nodes[i] = deepcopy_node(g->nodes[i]);
    }
}

/**
 * Deepcopy a node. 
 *
 * If you are unsure whether the processes are waiting for each other to
 * read data outside of their SPMDs, you can create a copy of a node
 * with this function.
 *
 * This way, all relevant data for a process remains within its thread.
 *
 * Parameters:
 * - `n`        A node structure that needs to be copied
 *
 * Returns:     An exact replica of node `n`, yet all pointers point to
 *                  newly allocated memory.
 */
struct node *deepcopy_node(struct node *n) {
    struct node *new = create_node(n->value, n->connections);

    new->value          = n->value;
    new->connections    = n->connections;
    new->eaten          = n->eaten;
    new->head           = n->head;
    new->tail           = n->tail;

    for (uint i=0; i<n->connections; i++) {
        new->connectedTo[i] = n->connectedTo[i];
    }
}

/**
 * Unallocate a graph to free memory.
 *
 * The function also unallocates any nodes that are in the graph.
 *
 * Parameters:
 * - `g`        Graph that is freed from memory.
 */
void unallocate_graph(struct graph *g) {
    for (uint i=0; i<g->length; i++) {
        unallocate_node(g->nodes[i]);
    }

    free(g->nodes);
    free(g);
}

/**
 * Unallocate a node to free memory.
 *
 * The function also unallocates any nodes that are in the node.
 *
 * Parameters:
 * - `n`        Node that is freed from memory.
 */
void unallocate_node(struct node *n) {
    free(n->connectedTo);
    free(n);
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
