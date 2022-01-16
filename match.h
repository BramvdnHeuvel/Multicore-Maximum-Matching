
/**
 * Create a new matching object.
 *
 * Parameters:
 * - `max_size`     The maximum amount of matches the object should expect.
 *
 * Returns:         The newly created matching structure.
 */
struct matching *create_matching(nid_int max_size);

/**
 * Add a new match to the the matching structure.
 *
 * Parameters:
 * - `a`        Unique node identifier.
 * - `b`        Unique node identifier.
 */
void insert_match(struct matching *m, nid_int a, nid_int b);

/**
 * Erase the matching object from memory.
 *
 * Parameters:
 * - `m`        The matching structure that is to be freed from memory.
 */
void unallocate_matching(struct matching *m);

