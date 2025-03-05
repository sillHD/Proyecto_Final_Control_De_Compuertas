#include "display_main.h"
#include "prints_display.h"
#include "main.h"
#include "ring_buffer.h" // Include the header file for ring_buffer_t
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include <string.h>
#include <stdio.h>

// Declare the UART handle
extern UART_HandleTypeDef huart2;

// Declare the temp_open variable

 uint32_t door_open_time = 0 ; // Almacena el tiempo en el que se abrió la puerta temporalmente
 uint8_t temp_open = 0; // Flag para indicar si la puerta está temporalmente abierta

void OLED_Printer(ring_buffer_t *rb, uint8_t *buffer, char *state)
{
  (void)rb;  // Evita la advertencia de compilador
  if(strcmp((char *)buffer, "#*D*#") == 0 )
  {
    strcpy(state, "To"); // Temporalmente abierta
    HAL_UART_Transmit(&huart2, (uint8_t *)"Puerta Abierta Temporalmente\r\n", 30, 100);
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
    ssd1306_Fill(Black);
    show_opened();
    
    // Iniciar temporizador en el loop principal
    initialize_temp();
  }
  if(strcmp((char *)buffer, "#*0*#") == 0 )
  {
    clean_case();
    return;
  }

  if (strcmp((char *)buffer, "#*A*#") == 0)
  {
    if (strcmp(state, "Op") == 0)
    {
      already_open();
    }
    else
    {
      open_case();
   }
  }

 else if (strcmp((char *)buffer, "#*C*#") == 0)
 {
  if (strcmp(state, "Cl") == 0)
  {
    already_closed();
  }
  else
  {
    close_case();
  } 
 }
}

void initialize_temp(void)
{
    temp_open = 1;
    door_open_time = HAL_GetTick(); // Guarda el tiempo actual en milisegundos
}