#include <bsp.h>

struct instruction;
struct neighbour;
struct node;
struct snake;

typedef unsigned int uint;

/**
* Find the maximum value between two numbers.
*/
uint max(uint a, uint b);

/**
 * Calculate the sum of an array of numbers.
 *
 * Parameters:
 * - `nums`     Pointer to the array of numbers.
 * - `n`        Length of the array.
 */
uint sum(uint *nums, uint n);

/**
* Distribute nodes across processes in a block distribution.
*
*  1 2 3 4 5 6 7 8 9
*  ----- ----- -----
*    A     B     C
*/
uint  block_distribution(uint nodeValue, uint totalNodes, uint totalProcesses);

/**
* Distribute nodes across processes in a cyclic distribution.
*
*  1 2 3 4 5 6 7 8 9
*  - - - - - - - - -
*  A B C A B C A B C
*/
uint cyclic_distribution(uint nodeValue, uint totalNodes, uint totalProcesses); 
