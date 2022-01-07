#include <bsp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "communicate.c"
#include "divider.c"
#include "graph.c"
#include "snake.c"
#include "utilities.c"

static unsigned int AMOUNT_OF_CORES;
static struct graph **subgraphs;

/**
* Function that runs synchronously on multiple cores.
*/
void spmd() {
    bsp_begin(AMOUNT_OF_CORES);
    
    /***********************************
    *       INITIALIZATION PHASE       *
    ************************************
    *  Load the data that was created  *
    *  for us during initialization.   *
    ************************************/

    struct graph *g = subgraphs[bsp_pid()];

    for (uint i=0; i<g->length; i++) {
        // printf("PID %u -> Node %u\n", bsp_pid(), g->nodes[i]->value);
        // printf("PID %u present!\n", bsp_pid());
    }


    /***********************************
    *********   SUPERSTEP 1    *********
    ************************************/

    uint *snake_nums = get_snake_numbers(bsp_pid());
    inspect_snake_numbers(snake_nums);
    
    bsp_end();
}

/**
* Setup function
*/
int main(int argc, char** argv) {
    
    // Ask for the initial graph
    struct graph *g = initialize_graph();
    printf("Graph built successfully.\n");


    // Set up the processes
    printf("How many processes would you like to use? There are %u cores available.\n", bsp_nprocs());
    fflush(stdout);
    scanf("%u", &AMOUNT_OF_CORES);

    if (AMOUNT_OF_CORES == 0 || AMOUNT_OF_CORES > bsp_nprocs()) {
        fprintf(stderr, "Cannot start %u processes.\n", AMOUNT_OF_CORES);
        return EXIT_FAILURE;
    }


    // Create a subgraph for each process
    subgraphs = divide_graph_cyclic(g, AMOUNT_OF_CORES);

    // Display the subgraphs
    for (uint i=0; i<AMOUNT_OF_CORES; i++) {
        show_graph(subgraphs[i]);
    }


    // Start the process
    bsp_init(&spmd, argc, argv);
    spmd();

    return EXIT_SUCCESS;
}