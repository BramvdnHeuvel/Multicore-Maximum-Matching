#include "utilities.h"

// Individual vertex in the graph
struct node;

struct graph;

struct node *create_node(uint n, uint maxDegree);

void connect_nodes(struct node *a, struct node *b);

struct graph *initialize_graph();

void show_graph(struct graph *g);

void show_node(struct node *n);