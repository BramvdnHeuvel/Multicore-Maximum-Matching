#include <bsp.h>

// Calculate the highest value
uint max(uint a, uint b) {
    if (a <= b) {
        return b;
    }
    return a;
}

/**
* Calculate something.
*/
uint block_distribution(uint nodeValue, uint totalNodes, uint totalProcesses) {
    return nodeValue * totalProcesses / totalNodes;
}
uint cyclic_distribution(uint nodeValue, uint totalNodes, uint totalProcesses) {
    return nodeValue % totalProcesses;
}
