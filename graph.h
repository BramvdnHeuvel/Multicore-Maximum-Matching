struct graph;
struct node;

#ifndef GRAPH_STRUCTURES
#define GRAPH_STRUCTURES

// #include "match.h"

struct graph {
    struct node **vertex;

    struct matching *m;

    nid_int max_size;
    nid_int local_degree;
    nid_int global_degree;
};

struct node {
    nid_int  value;

    nid_int *connections;
    nid_int  degree;
};

#endif

/**
 * Create a graph structure based on a given set of edges.
 *
 * Parameters:
 * - `total_nodes`  The total amount of nodes across the entire global graph.
 * - `local_edges`  The amount of edges that have at least one endpoint in this
 *                  (sub)graph.
 * - `edges`        Array of edges that have at least one endpoint in this
 *                  (sub)graph.
 *
 * Returns:         Pointer to a newly created graph structure.
 */
struct graph *load_structure(nid_int total_nodes, nid_int local_edges, 
                                                  nid_int (*edges)[2]);

/**
 * Create a node structure.
 *
 * Parameters:
 * - `n`            Unique node identifier.
 * - `degree`       Array length of parameter `connections`.
 */
struct node *create_node(nid_int n, nid_int degree);

/**
 * Remove an edge from a node.
 *
 * Parameters:
 * - `nd`       Pointer to the node structure.
 * - `n`        Unique node identifier that needs to be removed.
 */
void remove_edge(struct node *nd, nid_int n);

/**
 * Remove nodes from the graph that have a degree of 0.
 *
 * Parameters:
 * - `g`        Graph structure.
 */
void remove_empty_nodes(struct graph *g);

/**
 * Remove a node from the graph.
 *
 * Parameters:
 * - `todo`     Todo list structure that stores optional instructions.
 * - `g`        Graph structure.
 * - `n`        Unique node identifier of the node that needs to be removed.
 */
void remove_node(struct todo_list *todo, struct graph *g, nid_int n);

/**
 * Erase the graph structure from memory.
 *
 * This function also frees any nodes in the graph.
 *
 * Parameters:
 * - `g`        The graph structure that is to be freed from memory.
 */
void unallocate_graph(struct graph *g);

/**
 * Erase the node structure from memory.
 *
 * Parameters:
 * - `nd`       The node structure that is to be freed from memory.
 */
void unallocate_node(struct node *nd);
