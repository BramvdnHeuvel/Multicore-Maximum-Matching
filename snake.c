#include <stdio.h>
#include <stdlib.h>

#include "graph.h"

bool is_snake_tail(struct neighbour *n) {
    return n->base == n->value;
}
