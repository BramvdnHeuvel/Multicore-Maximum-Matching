#include <bsp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

#include "debug.c"
#include "divide.c"
#include "graph.c"
#include "instructions.c"
#include "match.c"
#include "steps.c"
#include "utilities.c"


static unsigned int AMOUNT_OF_CORES;

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
    uint n = bsp_nprocs();
    uint p = bsp_pid();

    nid_int edges_in_pid;   // Amount of edges in this process
    
    nid_int amountOfNodes;  // Amount of vertices in the graph
    nid_int amountOfEdges;  // Amount of edges in the graph

    bsp_push_reg(&edges_in_pid,  sizeof(nid_int));
    bsp_push_reg(&amountOfNodes, sizeof(nid_int));

    bsp_sync();



    /* Step a)
    **
    ** Prompt how many vertices and edges the graph has,
    ** and tell each process how many vertices to expect.
    */
    if (p == 0) {
        prompt_graph_size(&amountOfNodes, &amountOfEdges);

        broadcast_total_node_amount(&amountOfNodes);
    }
    bsp_sync();



    /* Step b)
    **
    ** Prompt all the edges between vertices. Temporarily store them
    ** and calculate how many edges each process may expect.
    */
    nid_int (*edges)[2];

    if (p == 0) {
        edges = malloc(amountOfEdges * 2 * sizeof(nid_int));
        
        prompt_edges(edges, amountOfEdges);
        broadcast_edge_amount(edges, amountOfEdges, amountOfNodes, 
                              &edges_in_pid);
    }
    bsp_sync();

    // DEBUG
    // Check that every PID expects the right amount of edges.
    debug_expected_amount_of_edges(edges_in_pid);



    /* Step c)
    **
    ** Prepare to receive the edges.
    */

    // Set up the edge array
    nid_int (*local_edges)[2] = malloc(2 * edges_in_pid * sizeof(nid_int));
    bsp_push_reg(local_edges, 2 * edges_in_pid * sizeof(nid_int));
    bsp_sync();

    // Send the appropriate edges to the right processes
    if (p == 0) {
        send_edges(edges, amountOfEdges, amountOfNodes, local_edges);
        free(edges);
    }
    bsp_sync();

    // DEBUG
    // Check that every PID received the right edges.
    debug_expected_edges(local_edges, edges_in_pid);



    /* Step d)
    **
    ** Set up the structure in the process. No communication is required here.
    */

    struct graph *g = load_structure(amountOfNodes, edges_in_pid, local_edges);

    // DEBUG
    // Check that the node have been initialised properly.
    debug_graph_setup(g);

    // Clean up memory and BSP registers
    bsp_pop_reg(&edges_in_pid);
    bsp_pop_reg(&amountOfNodes);
    free(local_edges);

    bsp_sync();




    /***********************************
    *            SUPERSTEP 1           *
    ************************************
    * Get rid of all nodes of degree 1 *
    ************************************/
    
    struct todo_list *todo;
    struct instruction *response;

    while (true) {
        todo = get_todo_list();

        remove_singletons(todo, g);

        response = send_instructions(todo);

        // // DEBUG
        // // Check which responses were received
        // debug_instruction_response(response, todo->expected_responses);

        // Continue to the next phase when communication has stopped.
        if (todo->expected_responses == 0) {
            unallocate_todo_list(todo);
            free(response);

            break;
        }

        // Interpret the instructions
        //printf("Interpreting instructions!\n");
        for (nid_int i=0; i<todo->expected_responses; i++) {
            //show_instruction(response[i]);
            interpret_instruction(todo, g, &(response[i]));
        }

        // // DEBUG
        // // Check what the graph looks like now.
        // bsp_sync();
        // debug_graph_setup(g);

        // Clean up memory
        unallocate_todo_list(todo);
        free(response);

        // // Debug break
        // break;
    }

    debug_graph_setup(g);

    // // DEBUG
    // // Check the graph's state after superstep 1
    // unallocate_graph(g);




    /***********************************
    *            SUPERSTEP 2           *
    ************************************
    * Create large snakes that travel  *
    * through the graph as far as      *
    * possible.                        *
    ************************************/



    bsp_end();
}

/**
* Setup function
*/
int main(int argc, char** argv) {
    unsigned int j = 0;
    
    // Set up the processes
    printf("How many processes would you like to use? There are %u cores available.\n", bsp_nprocs());
    fflush(stdout);
    scanf("%u", &AMOUNT_OF_CORES);

    if (AMOUNT_OF_CORES == 0 || AMOUNT_OF_CORES > bsp_nprocs()) {
        fprintf(stderr, "Cannot start %u processes.\n", AMOUNT_OF_CORES);
        return EXIT_FAILURE;
    }

    // Start the process
    bsp_init(&spmd, argc, argv);
    spmd();

    return EXIT_SUCCESS;
}