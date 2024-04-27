#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#define MEMORY_SIZE 1024UL*1024UL*1024UL*18 // Define the size of memory to allocate

void initializeMemory(void *ptr, size_t size, uint8_t value) {
    for (size_t i = 0; i < size; i++) {
        *((uint8_t *)ptr + i) = value;
    }
}

int main() {
    const size_t size = MEMORY_SIZE; // Use the defined size
    uint8_t value = 0xFF; // Define the value to initialize memory with

    // Allocate memory
    void *memory = malloc(size);
    if (memory == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        return EXIT_FAILURE;
    }

    // Initialize memory
    initializeMemory(memory, size, value);

    // Print memory content
    /*printf("Memory content after initialization:\n");
    for (size_t i = 0; i < size; i++) {
        //printf("%02X ", *((uint8_t *)memory + i));
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
    */
    sleep(120);
    // Free allocated memory
    free(memory);

    return EXIT_SUCCESS;
}
