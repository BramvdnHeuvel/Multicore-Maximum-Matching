
/**
* Vertex structure in a (sub)graph.
*
* Attributes:
* - `value`         A unique identifier that distinguishes the node.
* - `connections`   An array of integers that are identifiers to the nodes that
*                       this node is connected to.
* - `degree`        Array length of the `connections` attribute.
* - `eaten`         Boolean whether the node has been eaten by a snake yet.
* - `head`          Boolean whether the node is the head of a snake looking for
                        a tail to bite on.
* - `tail`          Boolean whether the node is a tail that can be bitten.
* - `eaten_by`      Pointer to the local snake that this node is part of.
*/
struct node {
    nid_int value;          // Node identifier

    // Which nodes are we connected to?
    nid_int *connections;   // Which ones?
    nid_int  degree;   // How many?

    // Snake operations
    bool eaten;         // Whether the node has been eaten by a snake
    bool head;          // Whether the node is a head /\_ 
    bool tail;          // Whether the node is a tail [] 
    struct snake *eaten_by; // Snake that the node is part of
};

/**
 * Snake structure in the graph.
 *
 * The snake structure slithers through the graph, looking for other
 * snakes to eat. It is the main component of the 2nd part of the
 * algorithm.
 *
 * Attributes:
 * - `local_head`    Node at the head of the local snake.
 * - `local_belly`   Nodes between the head and the tail of the local snake.
 * - `local_tail`    Node at the tail of the local snake.
 * - `next_node`     Unique node identifier of the tail that this snake bites.
 * - `previous_node` Unique node identifier of the head that bites this snake.
 * - `base`          Unique node identifier of the global snake's tail.
 */
struct snake {
    struct node *local_head;
    struct node *local_belly;
    struct node *local_tail;

    nid_int next_node;
    nid_int previous_node;

    nid_int base;
};



struct node *create_node(nid_int value, nid_int *endpoints,
                         nid_int edges_length) {
    struct node *nd = malloc(sizeof(struct node));

    nd->value = value; nd->degree = edges_length;
    nd->eaten = false; nd->head = false; nd->tail = false;
    nd->eaten_by = NULL;

    nd->connections = malloc(edges_length * sizeof(nid_int));

    for (nid_int i=0; i<edges_length; i++) {
        nd->connections[i] = endpoints[i];
    }

    return nd;
}

void remove_edge(struct node *nd, nid_int e) {
    nid_int i;

    for (i=0; i<nd->degree; i++) {
        if (nd->connections[i] == e) {
            break;
        }
    }

    for (; i<nd->degree-1; i++) {
        nd->connections[i] = nd->connections[i+1];
    }

    nd->degree--;
}

void unallocate_node(struct node *nd) {
    free(nd->connections);
    free(nd);
}

void show_node(struct node *nd) {
    printf("[ Node\n    value       -> %u\n    degree      -> %u\n    connections -> [ ", nd->value, nd->degree);
    for (nid_int i=0; i<nd->degree; i++) {
        printf("%u ", nd->connections[i]);
    }
    printf("]\n]\n");
}
