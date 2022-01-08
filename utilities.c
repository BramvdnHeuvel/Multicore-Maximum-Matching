nid_int max(nid_int *nums, uint n) {
    nid_int maximum = nums[0];

    for (uint i=0; i<n; i++) {
        if (nums[i] > maximum) {
            maximum = nums[i];
        }
    }
}