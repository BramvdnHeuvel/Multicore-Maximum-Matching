#include <bsp.h>
#include <stdlib.h>
#include <stdio.h>

#include "graph.c"
#include "snake.c"
#include "utilities.c"

static unsigned int AMOUNT_OF_CORES;
static struct graph **subgraphs;

void spmd() {
    bsp_begin(AMOUNT_OF_CORES);
    
    /***********************************
    *****   INITIALIZATION PHASE   *****
    ************************************/

    struct graph *g = subgraphs[bsp_pid()];


    /***********************************
    *********   SUPERSTEP 1    *********
    ************************************/



    
    bsp_end();
}

int main(int argc, char** argv) {
    struct graph *g = initialize_graph();
    printf("Graph initialized successfully.\n");


    printf("How many threads do you want started? There are %u cores available.\n", bsp_nprocs());
    fflush(stdout);
    scanf("%u", &AMOUNT_OF_CORES);


    if (AMOUNT_OF_CORES == 0 || AMOUNT_OF_CORES > bsp_nprocs()) {
        fprintf(stderr, "Cannot start %u threads.\n", AMOUNT_OF_CORES);
        return EXIT_FAILURE;
    }

    // Create subgraphs before starting the program
    subgraphs = divide_graph(g, AMOUNT_OF_CORES);

    for (uint i=0; i<AMOUNT_OF_CORES; i++) {
        show_graph(subgraphs[i]);
        printf("----------------------\n\n");
    }

    bsp_init(&spmd, argc, argv);
    spmd();
    
    return EXIT_SUCCESS;
}