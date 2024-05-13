#ifndef __WRITE_REGISTER_H__
#define __WRITE_REGISTER_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void write_register(uint32_t* addr, uint32_t value);
void write_uint_to_mem(unsigned int* addr, unsigned int value); // used in test_base_sw.c

#endif