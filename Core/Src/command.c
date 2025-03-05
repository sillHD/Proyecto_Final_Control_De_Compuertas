#include "command.h"
#include "prints_display.h"
#include "main.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include <string.h>
#include <stdio.h>
#include "ring_buffer.h"
#include "display_main.h"

extern UART_HandleTypeDef huart2;


void process_command(ring_buffer_t *rb, uint8_t *buffer, char *state) {
    if (ring_buffer_size(rb) == 5) {  // Verifica si el comando es de longitud 5
        // Lee el comando completo del buffer
        for (int i = 0; i < 5; i++) {
            ring_buffer_read(rb, &buffer[i]);
        }
        buffer[5] = '\0';  // Asegura el término del string
  
        if (strcmp((char *)buffer, "#*D*#") == 0) {
          OLED_Printer(rb, buffer, state);   
        } 
  
        // Procesa el comando basado en su contenido
        else if (strcmp((char *)buffer, "#*A*#") == 0) {
            if (strcmp(state, "Op") == 0) {
                HAL_UART_Transmit(&huart2, (uint8_t *)"Puerta ya esta abierta\r\n", 24, 100);
                OLED_Printer(rb, buffer, state);
            } else {
                HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);  // Cambia el estado del LED
                HAL_UART_Transmit(&huart2, (uint8_t *)"Puerta Abierta\r\n", 16, 100);
                OLED_Printer(rb, buffer, state);
                strcpy(state, "Op");
            }
  
        } 
  
        else if (strcmp((char *)buffer, "#*C*#") == 0) {
            if (strcmp(state, "Cl") == 0) {
                HAL_UART_Transmit(&huart2, (uint8_t *)"Puerta ya esta cerrada\r\n", 24, 100);
                OLED_Printer(rb, buffer, state);
            } else {
                HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);  // Cambia el estado del LED
                HAL_UART_Transmit(&huart2, (uint8_t *)"Puerta Cerrada\r\n", 16, 100);
                OLED_Printer(rb, buffer, state);
                strcpy(state, "Cl");
            }
        } 
  
        else if (strcmp((char *)buffer, "#*1*#") == 0) {
            if (strcmp(state, "Cl") == 0) {
                HAL_UART_Transmit(&huart2, (uint8_t *)"Estado de la puerta:Cerrada\r\n", 28, 100);
                ssd1306_Fill(Black);
                ssd1306_SetCursor(0, 0);
                ssd1306_WriteString((char *)"Door state:Cl", Font_7x10, White);
                ssd1306_UpdateScreen();
                HAL_Delay(5000); 
                ssd1306_Fill(Black);
                show_closed();
            } else {
                HAL_UART_Transmit(&huart2, (uint8_t *)"Estado de la puerta:Abierta\r\n", 28, 100);
                ssd1306_Fill(Black);
                ssd1306_SetCursor(0, 0);
                ssd1306_WriteString((char *)"Door state:Op", Font_7x10, White);
                ssd1306_UpdateScreen();
                HAL_Delay(5000); 
                ssd1306_Fill(Black);
                show_opened();
            }
        } 
  
        else if (strcmp((char *)buffer, "#*0*#") == 0) {
            HAL_UART_Transmit(&huart2, (uint8_t *)"Buffer limpiado y puerta cerrada\r\n", 34, 100);
            ring_buffer_reset(rb);
            OLED_Printer(rb, buffer, state);
            strcpy(state, "Cl");
            HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
            HAL_Delay(500);
            HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
            HAL_Delay(500);
            HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
            HAL_Delay(500);
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
        } 
  
        else {
            HAL_UART_Transmit(&huart2, (uint8_t *)"Comando no reconocido\r\n", 23, 100);
            if (strcmp(state, "Cl") == 0) {
                ssd1306_Fill(Black);
                ssd1306_SetCursor(0, 0);
                ssd1306_WriteString((char *)"Comand unknown", Font_7x10, White);
                ssd1306_UpdateScreen();
                HAL_Delay(2000); 
                ssd1306_Fill(Black);
                show_closed();
            } else {
                ssd1306_Fill(Black);
                ssd1306_SetCursor(0, 0);
                ssd1306_WriteString((char *)"Comand unknown", Font_7x10, White);
                ssd1306_UpdateScreen();
                HAL_Delay(2000); 
                ssd1306_Fill(Black);
                show_opened();
            }
        }
  
        // Reinicia el buffer de comando
        for (int i = 0; i < 5; i++) {
            buffer[i] = '_';
        }
    }
  }