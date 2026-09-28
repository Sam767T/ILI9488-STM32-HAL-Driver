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
// Pixel data is sent by DMA, so the SPI handle needs a TX DMA channel linked to it, with the
// DMA channel's and the SPI's interrupts enabled.
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

// The panel's gate lines run along its long side in both orientations. Hardware scrolling moves
// the image along these lines, which is horizontal in landscape and vertical in portrait.
#define ILI9488_SCROLL_LINES	480
#define ILI9488_LINE_PIXELS		320		// Along each of those lines

// Color definitions (18-bit RGB666)
#define	RGB666_BLACK	0x00000
#define	RGB666_BLUE     0x0003F
#define	RGB666_RED		0x3F000
#define	RGB666_GREEN	0x00FC0
#define RGB666_CYAN     0x00FFF
#define RGB666_MAGENTA	0x3F03F
#define RGB666_YELLOW	0x3FFC0
#define RGB666_WHITE	0xFFFFF

// Number of pixels sent per DMA transfer. Larger bursts mean fewer restarts between them.
#define N_BURST_PIXELS	1024

// Call before initializing any SPI devices
void ILI9488_ChipDeselect(void);

void ILI9488_Init(void);
void ILI9488_DrawPixel(uint16_t x, uint16_t y, uint32_t color);
void ILI9488_DrawHLine(uint16_t x, uint16_t y, uint16_t w, uint32_t colour);
void ILI9488_DrawVLine(uint16_t x, uint16_t y, uint16_t h, uint32_t colour);
void ILI9488_FillScreen(uint32_t colour);
void ILI9488_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint32_t colour, uint32_t bgcolor);
void ILI9488_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t colour);

// Panel refresh rate. frs is the FRS[3:0] code from the datasheet's Frame Rate Control (B1h) table:
// 0x0 = 28.78 Hz ... 0xA = 60.76 Hz (the default set by ILI9488_Init) ... 0xD = 91.15 Hz (the fastest).
// On the DFR0669 in landscape, 0xD leaves the top rows dark, so 0xC (78.13 Hz) is its fastest.
void ILI9488_SetFrameRate(uint8_t frs);

// Hardware scrolling along ILI9488_SCROLL_LINES. The fixed areas stay put and the lines between them
// scroll. ILI9488_ScrollTo sets which frame memory line is shown first in the scrolling area, and the
// panel applies it at the start of its next refresh, so scrolling never tears. The datasheet requires
// row/column exchange to be off for memory writes while scrolled, so in landscape the other drawing
// functions only work before scrolling, or after ILI9488_StopScrolling. ILI9488_DrawScrollLine works
// either way.
void ILI9488_SetScrollArea(uint16_t top_fixed, uint16_t bottom_fixed);
void ILI9488_ScrollTo(uint16_t line);
void ILI9488_StopScrolling(void);   // Shows the frame memory unscrolled again

// Draws a whole frame memory line from ILI9488_LINE_PIXELS colours, in either orientation, scrolled or
// not. Before any scrolling, the line is at y = line in portrait, with the colours running left to right,
// and at x = ILI9488_SCROLL_LINES - 1 - line in landscape, with the colours running top to bottom.
void ILI9488_DrawScrollLine(uint16_t line, const uint32_t* colours);

#endif // __ILI9488_H__
