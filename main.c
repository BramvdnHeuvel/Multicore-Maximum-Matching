#include <bsp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structures.h"

#include "divide.h"
#include "instructions.h"
#include "utilities.h"

#include "structures.c"

#include "divide.c"
#include "instructions.c"
//#include "steps.c"
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
    uint n = bsp_nprocs();
    uint p = bsp_pid();

    nid_int       nodes_in_pid;
    bsp_push_reg(&nodes_in_pid, sizeof(nid_int));
    nid_int       edges_in_pid;
    bsp_push_reg(&edges_in_pid, sizeof(nid_int));
    
    nid_int amountOfNodes;
    nid_int amountOfEdges;
    bsp_push_reg(&amountOfNodes, sizeof(nid_int));

    bsp_sync();

    // Step a)
    //
    // Prompt how many vertices and edges the graph has,
    // and tell each process how many vertices to expect.
    nid_int *node_distr;

    if (p == 0) {

        prompt_graph_size(&amountOfNodes, &amountOfEdges);

        broadcast_node_amount(amountOfNodes, &nodes_in_pid);

        broadcast_total_node_amount(&amountOfNodes);

    }
    bsp_sync();

    // Step b)
    //
    // Prompt all the edges between vertices. Temporarily store them
    // and calculate how many edges each process may expect.
    nid_int (*edges)[2];

    if (p == 0) {
        edges = malloc(amountOfEdges * 2 * sizeof(nid_int));
        prompt_edges(edges, amountOfEdges);
        broadcast_edge_amount(edges, amountOfEdges, amountOfNodes, &edges_in_pid);
    }
    bsp_sync();

    // Step c)
    //
    // Prepare to receive the edges.

    // // DEBUG: See whether all PIDs got the right amount of vertices and edges.
    // for (uint i=0; i<n; i++) {
    //     if (i == p) {
    //         printf("PID %u is expecting %u nodes and %u edges in total.\n", 
    //                 p, nodes_in_pid, edges_in_pid);
    //     }
    //     bsp_sync();
    // }

    // Make room for the given amount of edges
    nid_int (*local_edges)[2] = malloc(edges_in_pid * 2 * sizeof(nid_int));
    bsp_push_reg(local_edges, edges_in_pid * 2 * sizeof(nid_int));
    bsp_sync();

    // Send the appropriate edges to the right processes
    if (p == 0) {
        send_edges(edges, amountOfEdges, amountOfNodes, local_edges);
        free(edges);
    }
    bsp_sync();

    // // DEBUG: See if the PIDs received the right edges
    // for (uint i=0; i<n; i++) {
    //     if (i == p) {
    //         for (nid_int j=0; j<edges_in_pid; j++) {
    //             printf("PID %u received edge [%u, %u]\n", p, local_edges[j][0], local_edges[j][1]);
    //         }
    //     }
    //     bsp_sync();
    // }

    // Step d)
    //
    // Set up the structure in the process. No communication is required here.

    nid_int maximum_nodes = calculate_maximum_nodes_in_process(local_edges,
                            nodes_in_pid, edges_in_pid, amountOfNodes);

    // // DEBUG: Check the theoretical limit that each node may receive.
    // for (uint i=0; i<n; i++) {
    //     if (i == p) {
    //         printf("In theory, PID %u process can receive up to %u nodes.\n", p, maximum_nodes);
    //     }
    //     bsp_sync();
    // }

    // Create the node structures
    struct node **nodes = malloc(maximum_nodes * sizeof(struct node *));
    nodes_in_pid = 0;
    initialize_nodes(      nodes, &nodes_in_pid, 
                     local_edges,  edges_in_pid, amountOfNodes);

    // // DEBUG: Check if the nodes have initialised successfully.
    // for (uint i=0; i<n; i++) {
    //     if (i == p) {
    //         printf("PID %u: %u nodes found\n", p, nodes_in_pid);
    //         // for (nid_int j=0; j<nodes_in_pid; j++) {
    //         //     show_node(nodes[j]);
    //         // }
    //     }
    //     bsp_sync();
    // }

    free(local_edges);
    bsp_sync();


    nid_int (*matches)[2] = malloc((maximum_nodes)/2 * 2 * sizeof(nid_int));
    nid_int matches_found = 0;


    /***********************************
    *           SUPERSTEP 1            *
    ************************************
    * Get rid of all nodes of degree 1 *
    ************************************/
    
    struct todo_list *todo;
    struct instruction *response;

    while (true) {
        todo = get_todo_list();



        response = send_instructions(todo);

        // DEBUG: Check which responses were received
        debug_instruction_response(response);

        // Continue to the next phase when communication has stopped.
        if (todo->expected_responses == 0) {
            break;
        }

        // Clean up memory
        unallocate_todo_list(todo);
        free(response);
    }


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