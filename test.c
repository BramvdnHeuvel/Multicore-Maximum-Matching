#include <stdio.h>
#include "instructions.c"
#include "utilities.c"

int main(void) {
    printf("%d\n", instruction_keep_alive() == instruction_keep_alive());
    printf("%d\n", same_instruction(instruction_keep_alive(), instruction_keep_alive()));

    struct instruction *ins = instruction_keep_alive();

    printf("%u bytes in content\n%u bytes per integer\n%u integers in content\n", sizeof(ins->content), sizeof(nid_int), sizeof(ins->content)/sizeof(nid_int));
}