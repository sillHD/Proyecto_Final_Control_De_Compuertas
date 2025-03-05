
#include "prints_display.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include <stdio.h>
#include "locked.h"
#include "unlocked.h"



void clean_case(void) {
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("Buffer:Clean", Font_7x10, White);
    ssd1306_SetCursor(0, 30);
    ssd1306_WriteString("Door:Cl", Font_7x10, White);
    ssd1306_UpdateScreen();
    HAL_Delay(2000); 
    ssd1306_Fill(Black);
    show_closed();
}

void already_open(void) {
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("Door already open", Font_7x10, White);
    ssd1306_UpdateScreen();
    HAL_Delay(2000); 
    ssd1306_Fill(Black);
    show_opened();
}

void open_case(void) {
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("Door: Op", Font_7x10, White);
    ssd1306_UpdateScreen();
    HAL_Delay(2000); 
    ssd1306_Fill(Black);
    show_opened();
}

void already_closed(void) {
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("Door already closed", Font_7x10, White);
    ssd1306_UpdateScreen();
    HAL_Delay(2000); 
    ssd1306_Fill(Black);
    show_closed();
}

void close_case(void) {
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("Door: Cl", Font_7x10, White);
    ssd1306_UpdateScreen();
    HAL_Delay(2000); 
    ssd1306_Fill(Black);
    show_closed();
}

void show_opened(void) {
    ssd1306_Fill(Black);
    ssd1306_DrawBitmap(0, 0, unlocked, 128, 64, White);
    ssd1306_UpdateScreen();
}

void show_closed(void) {
    ssd1306_Fill(Black);
    ssd1306_DrawBitmap(0, 0, locked, 128, 64, White);
    ssd1306_UpdateScreen();
}