#include <bsp.h>
#include <stdlib.h>
#include <stdio.h>

#include "graph.c"
#include "snake.c"
#include "utilities.c"

static unsigned int AMOUNT_OF_CORES;
static struct graph *g;

void spmd() {
    bsp_begin(AMOUNT_OF_CORES);
    
    // Run the algorithm
    
    bsp_end();
}

int main(int argc, char** argv) {
    g = initialize_graph();
    printf("Graph initialized successfully.\n");

    show_graph(g);

    printf("How many threads do you want started? There are %u cores available.\n", bsp_nprocs());
    fflush(stdout);
    scanf("%u", &AMOUNT_OF_CORES);


    if (AMOUNT_OF_CORES == 0 || AMOUNT_OF_CORES > bsp_nprocs()) {
        fprintf(stderr, "Cannot start %u threads.\n", AMOUNT_OF_CORES);
        return EXIT_FAILURE;
    }
    bsp_init(&spmd, argc, argv);
    spmd();
    
    return EXIT_SUCCESS;
}