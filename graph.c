#include <bsp.h>

#include "divide.h"
#include "graph.h"
#include "instructions.h"
#include "match.h"
#include "utilities.h"

// PRIVATE FUNCTIONS
bool belongs_here(struct graph *g, nid_int n);
nid_int count_unique_nodes(nid_int (*edges)[2], nid_int edges_length);
nid_int index_of_node(struct graph* g, nid_int n, nid_int *index);
struct graph *create_graph(nid_int max_size, nid_int global_size);
uint belongs_to_proc(struct graph *g, nid_int n);

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
                                                  nid_int (*edges)[2]) {
    nid_int maximum_nodes = count_unique_nodes(edges, local_edges);

    struct graph *g = create_graph(maximum_nodes, total_nodes);

    for (nid_int i=0; i<local_edges; i++) {
        for (unsigned short int j=0; j<2; j++) {
            nid_int node      = edges[i][j];
            nid_int neighbour = edges[i][1-j];

            // Let's take a look at this endpoint!
            // As long as it belongs to this process, of course.
            if (belongs_here(g, node)) {
                nid_int index = 0;

                if (index_of_node(g, node, &index) == 0) {
                    // Count how many edges connect to this node.
                    nid_int connections = 0;
                    for (nid_int j=i; j<local_edges; j++) {
                        if (edges[j][0] == node || edges[j][1] == node) {
                            connections++;
                        }
                    }

                    // Add node
                    index            = g->local_degree;
                    g->vertex[index] = create_node(node, connections);
                    g->local_degree  = index + 1;
                }

                // Insert connection
                struct node *nd = g->vertex[index];
                nid_int size    = nd->degree;

                nd->connections[size] = neighbour;
                nd->degree            = size + 1;
            }
        }
    }

    return g;
}

/**
 * Create a node structure.
 *
 * Parameters:
 * - `n`            Unique node identifier.
 * - `degree`       Array length of parameter `connections`.
 */
struct node *create_node(nid_int n, nid_int degree) {
    struct node *nd  = malloc(       1 * sizeof(struct node));
    nd->connections  = malloc(degree   * sizeof(nid_int)    );

    nd->degree = 0;
    nd->value  = n;

    return nd;
}

void interpret_instruction(struct todo_list *todo, struct graph *g, 
                           struct instruction *ins) {
    short int name = ins->value;

    /* KEEP_ALIVE
    ** Nothing is needed to be done.
    **
    ** This is an overhead instruction. It tells us that other processes are
    ** still communicating, and that their communication may eventually 
    ** influence this process too.
    */
    if (name == KEEP_ALIVE) {
        return;
    }

    /* DELETE
    ** Remove a node from the graph.
    **
    ** This node exists in another process and has already been deleted,
    ** but the process is notified so that their connection to that node
    ** can be removed.
    **
    ** [0] Deleted node                     | Not in this process
    ** [1] Neighbour of the deleted node    | In this process       (probably)
    */
    else if (name == DELETE) {
        nid_int index;
        if (index_of_node(g, ins->content[1], &index) == 1) {
            remove_edge(g->vertex[index], ins->content[0]);
        }

    }

    /* MOVE
    ** Move a node to this graph.
    **
    ** The concerning node has degree 1 and their only neighbour is in this
    ** process. It would like to match, but it can only match if its neighbour
    ** hasn't already been matched last round.
    ** Effectively, the node shouldn't be created, but instead it should take
    ** out its neighbour and become a match if that neighbour is still
    ** available for matching.
    **
    ** [0] Moved node                       | Not in this process
    ** [1] Neighbour of the moved node      | In this process       (probably)
    */
    else if (name == MOVE) {
        nid_int index;

        if (index_of_node(g, ins->content[1], &index) == 1) {
            insert_match(g->m, ins->content[0], ins->content[1]);
            remove_node(todo, g, ins->content[1]);
        }
    }

    /*
    **
    */
    else if (name == INHERIT) {

    }
    else if (name == REVERSE) {

    }
    else if (name == CONCATENATE) {

    }
    else {
        printf(
            "[PID %u] WARNING! Received unknown instruction %u!\n", 
            bsp_pid(), name
        );
    }
}

/**
 * Remove an edge from a node.
 *
 * Parameters:
 * - `nd`       Pointer to the node structure.
 * - `n`        Unique node identifier that needs to be removed.
 */
void remove_edge(struct node *nd, nid_int n) {
    nd->degree--;
    nid_int i = 0;

    for (; i<nd->degree; i++) {
        if (nd->connections[i] == n) {
            break;
        }
    }

    for (; i<nd->degree; i++) {
        nd->connections[i] = nd->connections[i+1];
    }
}

/**
 * Remove nodes from the graph that have a degree of 0.
 *
 * Parameters:
 * - `g`        Graph structure.
 */
void remove_empty_nodes(struct graph *g) {
    nid_int *empty_nodes = malloc(g->local_degree * sizeof(nid_int));
    nid_int empty_length = 0;

    for (nid_int i=0; i<g->local_degree; i++) {
        struct node *nd = g->vertex[i];

        if (nd->degree == 0) {
            empty_nodes[empty_length] = nd->value;
            empty_length++;
        }
    }

    for (nid_int i=0; i<empty_length; i++) {
        remove_node(NULL, g, empty_nodes[i]);
    }

    free(empty_nodes);
}

/**
 * Remove a node from the graph.
 *
 * Parameters:
 * - `todo`     Todo list structure that stores optional instructions.
 * - `g`        Graph structure.
 * - `n`        Unique node identifier of the node that needs to be removed.
 */
void remove_node(struct todo_list *todo, struct graph *g, nid_int n) {
    nid_int index = 0;
    printf("[PID %u] Let's remove node %u!\n", bsp_pid(), n);

    nid_int outcome = index_of_node(g, n, &index);

    if (outcome != 1) {
        printf("[PID %u] Huh, strange. I can't find node %u.\n", bsp_pid(), n);
        return;
    }

    struct node *nd = g->vertex[index];
            
    // Make all neighbours forget you ever existed.
    for (nid_int i=0; i<nd->degree; i++) {
        nid_int neighbour = nd->connections[i];
        nid_int nindex;
        outcome = index_of_node(g, neighbour, &nindex);

        if (outcome == 1) {         // Neighbour was found

            remove_edge(g->vertex[nindex], n);

        } else if (outcome == 2) {  // Neighbour is in another process

            add_instruction(instruction_delete_node(n, neighbour), 
                todo, belongs_to_proc(g, neighbour)
            );
        }
    }

    g->local_degree--;
    // Remove the node itself.
    for (nid_int i=index; i<g->local_degree; i++) {
        g->vertex[i] = g->vertex[i+1];
    }
    unallocate_node(nd);
}

/**
 * Remove singletons from the graph by matching them to their only neighbour.
 *
 * Parameters:
 * - `todo`     Todo structure that tracks optional instructions.
 * - `g`        Graph structure.
 */
void remove_singletons(struct todo_list *todo, struct graph *g) {
    bool singleton_found = true;

    while (singleton_found) {
        singleton_found = false;

        for (nid_int i=0; i<g->local_degree; i++) {
            struct node *nd = g->vertex[i];

            if (nd->degree == 1) {  // We found a singleton!
                nid_int neighbour = nd->connections[0];

                if (belongs_here(g, neighbour)) {
                    remove_node(todo, g, neighbour);
                    insert_match(g->m, nd->value, neighbour);
                } else {
                    add_instruction(instruction_move_node(nd->value, neighbour),
                        todo, belongs_to_proc(g, neighbour)
                    );
                }
                remove_node(todo, g, nd->value);

                singleton_found = true;
                break;
            }
        }
    }

    remove_empty_nodes(g);
}

/**
 * Erase the graph structure from memory.
 *
 * This function also frees any nodes in the graph.
 *
 * Parameters:
 * - `g`        The graph structure that is to be freed from memory.
 */
void unallocate_graph(struct graph *g) {
    // Free the nodes
    for (nid_int i=0; i<g->local_degree; i++) {
        unallocate_node(g->vertex[i]);
    }

    unallocate_matching(g->m);
    free(g->vertex);
    free(g);
}

/**
 * Erase the node structure from memory.
 *
 * Parameters:
 * - `nd`       The node structure that is to be freed from memory.
 */
void unallocate_node(struct node *nd) {
    free(nd->connections);
    free(nd);
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
 * Check whether a given node belongs in this process.
 * 
 * Parameters:
 * - `g`        Graph structure.
 * - `n`        Unique node identifier.
 *
 * Returns:     boolean whether the node identifier should be in this process.
 */
bool belongs_here(struct graph *g, nid_int n) {
    return (belongs_to_proc(g, n) == bsp_pid());
}

/** PRIVATE
 * Count the amount of unique endpoints in an array of edges.
 *
 * Parameters:
 * - `edges`        Array of edges.
 * - `edges_length` Array length of parameter `edges`.
 *
 * Returns:         Amount of unique endpoints.
 */
nid_int count_unique_nodes(nid_int (*edges)[2], nid_int edges_length) {
    nid_int *endpoints = malloc(2 * edges_length * sizeof(nid_int));

    for (nid_int i=0; i<edges_length; i++) {
        endpoints[i]              = edges[i][0];
        endpoints[edges_length+i] = edges[i][1];
    }

    nid_int answer = unique(endpoints, 2*edges_length);
    free(endpoints);
    return answer;
}

/** PRIVATE
 * Determine the index of a node in the graph.
 *
 * Parameters:
 * - `g`        Graph structure.
 * - `n`        Unique node identifier.
 * - `index`    Pointer to the integer that stores the index.
 *
 * Returns:      /  0    if the node doesn't exist in the graph (yet) \
 *               |  1    if the node was found                        |
 *               \  2    if the node should be in a different process /
 */
nid_int index_of_node(struct graph* g, nid_int n, nid_int *index) {
    if (!belongs_here(g, n)) {
        return 2;
    }

    for (nid_int i=0; i<g->local_degree; i++) {
        if (g->vertex[i]->value == n) {
            *index = i;
            return 1;
        }
    }

    return 0;
}

/** PRIVATE
 * Create a graph structure that this process may manage.
 *
 * Parameters:
 * - `max_size`     The theoretical limit of how many vertices may be expected
 *                  in this process as a result of moving nodes around.
 * - `global_size`  The total amount of nodes across the entire global graph.
 *
 * Returns:         Pointer to a newly created graph structure.
 */
struct graph *create_graph(nid_int max_size, nid_int global_size) {
    struct graph *g  = malloc(       1 * sizeof(struct graph ));
    g->vertex        = malloc(max_size * sizeof(struct node *));
    g->m             = create_matching(max_size / 2);

    g->local_degree  = 0;
    g->max_size      = max_size;
    g->global_degree = global_size;

    return g;
}

/** PRIVATE
 * Determine in to which process a given node ID belongs.
 * 
 * Parameters:
 * - `g`        Pointer to this process' graph structure.
 * - `n`        Unique node identifier.
 *
 * Returns:     The process that node `n` belongs to.
 */
uint belongs_to_proc(struct graph *g, nid_int n) {
    return divide(n, g->global_degree, bsp_nprocs());
}
