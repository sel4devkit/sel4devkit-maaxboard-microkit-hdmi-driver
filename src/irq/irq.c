#include <stdio.h>
#include <stdlib.h>

#include <microkit.h>

// Included to stop error
char *__heap_start;
char *__heap_end;

void init(void) {
	
}

void
notified(microkit_channel ch) {

	printf("dcss interrupt\n");
    microkit_irq_ack(ch);
}