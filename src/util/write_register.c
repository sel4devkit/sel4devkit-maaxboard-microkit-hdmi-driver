#include "write_register.h"
#include <string.h>
#include <stdio.h>

void write_register(uint32_t* addr, uint32_t value) {

    //(*(uint32_t *)(addr)) = value;
	*addr = value;
}


void write_uint_to_mem(unsigned int* addr, unsigned int value) {

    // (*(uint32_t *)(addr)) = value;
	// printf("Pointer address = %p\n", addr);
	// printf("Value before = %x \n", *(addr));
    // printf("Value to write = %x \n", value);
	*addr = value;
	//printf("Value after = %x \n", *(addr));
}

void write_register_debug(uint32_t* addr, uint32_t value) {
    printf("Pointer address = %p\n", addr);
	printf("Value before = %x \n", *(addr));
    printf("Value to write = %x \n", value);
    //(*(uint32_t *)(addr)) = value; 
	*addr = value; 
    printf("Value after = %x \n", *(addr));
}
