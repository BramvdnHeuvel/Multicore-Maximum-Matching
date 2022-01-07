#include "utilities.h"

#ifndef SNAKES_EVERYWHERE
#define SNAKES_EVERYWHERE

// Snake that forms M-augmenting paths
struct snake {
    struct node *local_head;
    struct node *local_belly;
    struct node *local_tail;

    uint next_node;
    uint previous_node;

    uint base;
};

#endif

bool is_snake_tail(struct neighbour *n);
