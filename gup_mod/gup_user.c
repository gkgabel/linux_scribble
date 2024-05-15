#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define DEVICE_FILE "/dev/devnull"
#define PAGE_SIZE 4096UL
#define PAGES_PER_ALLOC 5
#define NR_ALLOC 100 //24UL*1000UL
int main() {
    int fd;
    ssize_t ret;
    char *buffer[NR_ALLOC];

    // Open the device file
    fd = open(DEVICE_FILE, O_WRONLY);
    if (fd == -1) {
        perror("Failed to open the device file");
        return EXIT_FAILURE;
    }

    // Allocate memory using mmap pagewise
    for(unsigned long i=0 ; i < NR_ALLOC ; i++)
    {
        buffer[i] = mmap(NULL, PAGE_SIZE*PAGES_PER_ALLOC, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (buffer[i] == MAP_FAILED) {
            perror("Failed to allocate memory using mmap");
            close(fd);
            return EXIT_FAILURE;
        }

        // Initialise each page allocated
        for(unsigned int j=0;j<PAGES_PER_ALLOC;j++)
            snprintf(buffer[i]+j*PAGE_SIZE,PAGE_SIZE,"%lu%u",i,j);

        //Only Pinning one in every 10 page
        if(i%10) continue;

        // Write for this device refers to pinning page using GUP
        // Write to the device file from the allocated memory
        ret = write(fd, buffer[i],PAGE_SIZE*PAGES_PER_ALLOC);

        if (ret == -1) {
            perror("Failed to write to the device file");
            munmap(buffer[i], PAGE_SIZE*PAGES_PER_ALLOC);
            close(fd);
            return EXIT_FAILURE;
        }
        printf("Pages GUPed: %ld \n",ret);
    }
    
    sleep(60);
    for(unsigned long i=0;i<NR_ALLOC;i++)
    {
            if(i%10)
                munmap(buffer, PAGE_SIZE*PAGES_PER_ALLOC);
    }

    // Close the device file
    close(fd);

    printf("Data written successfully!\n");

    return EXIT_SUCCESS;
}
