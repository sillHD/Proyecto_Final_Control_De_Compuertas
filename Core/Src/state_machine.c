#include "state_machine.h"
#include "main.h"

void system_state_machine(char *states)
{
  int state = system_events_handler(states);
  switch (state)
  {
    case 0: // Door closed
      break;
    case 1: // Door temporarily opened;
      break;
    case 2: // Door permanent opened
      break;
    default:
      break;
  }
}