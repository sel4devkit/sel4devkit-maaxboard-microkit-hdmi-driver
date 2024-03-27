/******************************************************************************
 *
 * Copyright (C) 2016-2017 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 *
 * Copyright 2017-2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 ******************************************************************************
 *
 * test_base_sw.c
 *
 ******************************************************************************
 */

#include <microkit.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include <externs.h>


uintptr_t hdmi_base;

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


int cdn_apb_read(unsigned int addr, unsigned int *value)
{
	//unsigned int temp;
	//u64 tmp_addr = addr + HDMI_BASE;
	//printf("%s() ...\n", __func__);

	*value  = *((uint32_t*)(hdmi_base + addr));
	//printf("Pointer address HDMI = %p\n",(void*)(hdmi_ptr));
	
	//printf("Pointer address HDMI = %p\n",(void*)((uint32_t*)(hdmi_base + addr)));
	return 0;
}

int cdn_apb_write(unsigned int addr, unsigned int value)
{
	//printf("%s() ...\n", __func__);
	//printf("ADDR = %x, value = %x\n", addr, value);
	write_uint_to_mem((unsigned int*)((hdmi_base + addr)), value); // check casts
	return 0;
}

int cdn_sapb_read(unsigned int addr, unsigned int *value)
{
	// unsigned int temp;
	// u64 tmp_addr = addr + HDMI_SEC_BASE;

	// temp = __raw_readl(tmp_addr);
	// *value = temp;
	printf("%s() ...\n", __func__);
	return 0;
}

int cdn_sapb_write(unsigned int addr, unsigned int value)
{
	// u64 tmp_addr = addr + HDMI_SEC_BASE;
	//(*(uint32_t *)(addr + HDMI_SEC_BASE)) = value;
	// __raw_writel(value, tmp_addr);
	printf("%s() ...\n", __func__);
	return 0;
}

void cdn_sleep(uint32_t ms)
{
	printf("%s() ...\n", __func__);
	//mdelay(ms);
}

void cdn_usleep(uint32_t us)
{
	printf("%s() ...\n", __func__);
	//udelay(us);
}


