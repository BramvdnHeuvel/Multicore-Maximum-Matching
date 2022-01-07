#include "utilities.h"

/**
* Find the maximum value between two numbers.
*/
uint max(uint a, uint b) {
    if (a <= b) {
        return b;
    }
    return a;
}

/**
 * Calculate the sum of an array of numbers.
 *
 * Parameters:
 * - `nums`     Pointer to the array of numbers.
 * - `n`        Length of the array.
 */
uint sum(uint *nums, uint n) {
    uint total = 0;

    for (uint i=0; i<n; i++) {
        total = total + nums[i];
    }

    return total;
}

/**
* Distribute nodes across processes in a block distribution.
*
*  1 2 3 4 5 6 7 8 9
*  ----- ----- -----
*    A     B     C
*/
uint block_distribution(uint nodeValue, uint totalNodes, uint totalProcesses) {
    return nodeValue * totalProcesses / totalNodes;
}

/**
* Distribute nodes across processes in a cyclic distribution.
*
*  1 2 3 4 5 6 7 8 9
*  - - - - - - - - -
*  A B C A B C A B C
*/
uint cyclic_distribution(uint nodeValue, uint totalNodes, uint totalProcesses) {
    return nodeValue % totalProcesses;
}
