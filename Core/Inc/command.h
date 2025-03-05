#ifndef __COMMAND_H_
#define __COMMAND_H_


#include "ring_buffer.h"

void process_command(ring_buffer_t *rb, uint8_t *buffer, char *state);


#endif // __COMMAND_H_
