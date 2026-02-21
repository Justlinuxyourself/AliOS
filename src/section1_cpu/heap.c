/* LINKED TO NOTEBOOK: SECTION VI - Memory Management */
#include "heap.h"

// Start heap at 2MB mark to avoid clobbering the kernel
unsigned char* heap_ptr = (unsigned char*)0x200000;
unsigned int bytes_allocated = 0; // New counter

void* kmalloc(unsigned int size) {
    void* res = (void*)heap_ptr;
    heap_ptr += size;
    bytes_allocated += size; // Track every allocation
    return res;
}

unsigned int get_heap_usage() {
    return bytes_allocated;
}

