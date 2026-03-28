/***************************************************************
 * FILENAME: ILI9488.h
 * DESCRIPTION:
 *   4-wire SPI driver for ILI9488 LCD controller.
 * AUTHOR: Sam T
 * DATASHEET: https://www.hpinfotech.ro/ILI9488.pdf
 ***************************************************************/

#ifndef __ILI9488_H__
#define __ILI9488_H__

#include "main.h"
#include "fonts.h"
#include <stdbool.h>

/*** Redefine if necessary ***/
#define ILI9488_SPI_PORT hspi1
extern SPI_HandleTypeDef ILI9488_SPI_PORT;

// User-labelled pin definitions
#define ILI9488_RST_Pin		LCD_RST_Pin
#define	ILI9488_RST_Port	LCD_RST_GPIO_Port
#define	ILI9488_DC_Pin		LCD_DC_Pin
#define	ILI9488_DC_Port		LCD_DC_GPIO_Port
#define ILI9488_CS_Pin		LCD_CS_Pin
#define	ILI9488_CS_Port		LCD_CS_GPIO_Port

#define LANDSCAPE_ORIENTATION	1	// Change to 0 for portrait orientation

#if LANDSCAPE_ORIENTATION
// Landscape orientation (default)
#define ILI9488_WIDTH	480
#define ILI9488_HEIGHT	320
#else
// Portrait orientation
#define ILI9488_WIDTH	320
#define ILI9488_HEIGHT	480
#endif

// Color definitions (18-bit RGB666)
#define	RGB666_BLACK	0x00000
#define	RGB666_BLUE     0x0003F
#define	RGB666_RED		0x3F000
#define	RGB666_GREEN	0x00FC0
#define RGB666_CYAN     0x00FFF
#define RGB666_MAGENTA	0x3F03F
#define RGB666_YELLOW	0x3FFC0
#define RGB666_WHITE	0xFFFFF

// Number of burst buffer pixels
#define N_BURST_PIXELS	256

// Call before initializing any SPI devices
void ILI9488_ChipDeselect(void);

void ILI9488_Init(void);
void ILI9488_DrawPixel(uint16_t x, uint16_t y, uint32_t color);
void ILI9488_DrawHLine(uint16_t x, uint16_t y, uint16_t w, uint32_t colour);
void ILI9488_DrawVLine(uint16_t x, uint16_t y, uint16_t h, uint32_t colour);
void ILI9488_FillScreen(uint32_t colour);
void ILI9488_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint32_t colour, uint32_t bgcolor);
void ILI9488_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t colour);

#endif // __ILI9488_H__
