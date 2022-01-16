#include "match.h"

/**
 * Create a new matching object.
 *
 * Parameters:
 * - `max_size`     The maximum amount of matches the object should expect.
 *
 * Returns:         The newly created matching structure.
 */
struct matching *create_matching(nid_int max_size) {
    struct matching *m = malloc(sizeof(struct matching));
    m->matches         = malloc(2 * max_size * sizeof(nid_int));

    m->length     = 0;
    m->max_length = max_size;

    return m;
}

/**
 * Add a new match to the the matching structure.
 *
 * Parameters:
 * - `a`        Unique node identifier.
 * - `b`        Unique node identifier.
 */
void insert_match(struct matching *m, nid_int a, nid_int b) {
    nid_int size = m->length;

    m->matches[size][0] = a;
    m->matches[size][1] = b;
    m->length = size + 1;
}

/**
 * Erase the matching object from memory.
 *
 * Parameters:
 * - `m`        The matching structure that is to be freed from memory.
 */
void unallocate_matching(struct matching *m) {
    free(m->matches);
    free(m);
}

