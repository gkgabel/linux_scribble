#include <stdio.h>
#include <numa.h>

#define SIZE_IN_BYTES (1024 * 1024 * 1024) // 1 MB

int main() {
    // Initialize NUMA library
    numa_available();

    // Query NUMA information
    int num_nodes = numa_max_node() + 1;
    printf("Number of NUMA nodes: %d\n", num_nodes);

    // Select NUMA node for allocation and access
    int target_node = 0; // Node for allocation and initial access
    int other_node = 1;  // Another node for access

    // Allocate memory on the selected NUMA node
    void *ptr = numa_alloc_onnode(SIZE_IN_BYTES, target_node);
    if (ptr == NULL) {
        fprintf(stderr, "Failed to allocate memory on NUMA node %d\n", target_node);
        return 1;
    }

    // Access the allocated memory from the same NUMA node
    printf("Accessing memory from NUMA node %d:\n", target_node);
    for (int i = 0; i < SIZE_IN_BYTES; i += sizeof(int)) {
        *((int *)ptr + i) = i;
    }

    // Access the allocated memory from another NUMA node
    numa_run_on_node(other_node);
    printf("Accessing memory from NUMA node %d:\n", other_node);
    while(1)
    {
        for (int i = 0; i < SIZE_IN_BYTES; i += sizeof(int)) {
            printf("%d ", *((int *)ptr + i));
        }
        printf("\n");
    }
    // Free the allocated memory
    numa_free(ptr, SIZE_IN_BYTES);

    return 0;
}
