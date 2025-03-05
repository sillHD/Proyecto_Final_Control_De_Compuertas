#ifndef __DISPLAY_MAIN_H_
#define __DISPLAY_MAIN_H_


#include "ring_buffer.h" // Include the header file where ring_buffer_t is defined

void OLED_Printer(ring_buffer_t *rb, uint8_t *buffer, char *state);
void initialize_temp(void);

#endif // __DISPLAY_MAIN_H_