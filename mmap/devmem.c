#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096
#define MAP_MASK (PAGE_SIZE-1)

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "\nUsage:\t%s <address> [data]\n"
                "\taddress: the device physical memory address\n"
                "\tdata   : data to written\n"
                "\nNote   : only supported byte data read and write\n",
            argv[0]);
        return -1;
    }

    int fd;
    unsigned int addr;
    void *map_ptr, *virt_ptr;
    unsigned char result, data;

    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        fprintf(stderr, "Failed to open '/dev/mem': %s\n", strerror(errno));
        return -1;
    }

    addr = strtoul(argv[1], NULL, 0);
    map_ptr = mmap(0, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, addr & ~MAP_MASK);
    if (map_ptr == MAP_FAILED) {
        close(fd);
        fprintf(stderr, "Failed to mmap: %s\n", strerror(errno));
        return -1;
    }

    virt_ptr = map_ptr + (addr & MAP_MASK);
    result = *(unsigned char*)virt_ptr;
    printf("Data in memory(%s): 0x%X\n", argv[1], result);

    if (argc == 2) {
        goto out;
    }

    // write the memory
    data = (unsigned char)strtoul(argv[2], NULL, 0);
    *(unsigned char*)virt_ptr = data;
    result = *(unsigned char*)virt_ptr;
    printf("Write to data: 0x%X, read again: 0x%X\n", data, result);
out:
    if (munmap(map_ptr, PAGE_SIZE) == -1) {
        fprintf(stderr, "Failed to munmap: %s\n", strerror(errno));
    }
    close(fd);

    return 0;
}
