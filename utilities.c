#include "utilities.h"

/**
 * Exchange with other processes how many instructions they're supposed to
 * expect.
 */
void exchange_numbers_all_to_all(nid_int *numbers) {
    uint n = bsp_nprocs();
    uint p = bsp_pid();

    bsp_push_reg(numbers, n*sizeof(nid_int));
    bsp_sync();

    for (uint i=0; i<n; i++) {
        bsp_put(i, &(numbers[i]), numbers, p*sizeof(nid_int), sizeof(nid_int));
    }
    bsp_sync();
    bsp_pop_reg(numbers);
}

nid_int max(nid_int *nums, uint n) {
    nid_int maximum = nums[0];

    for (uint i=0; i<n; i++) {
        if (nums[i] > maximum) {
            maximum = nums[i];
        }
    }

    return maximum;
}

nid_int unique(nid_int *nums, nid_int n) {
    nid_int *unique_values = malloc(n * sizeof(nid_int));
    nid_int unique_length  = 0;

    for (nid_int i=0; i<n; i++) {
        nid_int value = nums[i];
        bool seen_before = false;

        for (nid_int j=0; j<unique_length; j++) {
            if (unique_values[j] == value) {
                seen_before = true;
                break;
            }
        }

        if (!seen_before) {
            unique_values[unique_length] = value;
            unique_length++;
        }
    }

    free(unique_values);
    return unique_length;
}

nid_int sum(nid_int *nums, uint n) {
    nid_int total = 0;

    for (uint i=0; i<n; i++) {
        total = total + nums[i];
    }

    return total;
}