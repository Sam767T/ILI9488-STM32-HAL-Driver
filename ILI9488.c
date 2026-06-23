/***************************************************************
 * FILENAME: ILI9488.c
 * DESCRIPTION:
 *   4-wire SPI driver for ILI9488 LCD controller.
 * AUTHOR: Sam T
 * DATASHEET: https://www.hpinfotech.ro/ILI9488.pdf
 ***************************************************************/

#include "stm32u5xx_hal.h"  // Change if not using STM32U5 family
#include "ILI9488.h"

static void ILI9488_ChipSelect() {
    HAL_GPIO_WritePin(ILI9488_CS_Port, ILI9488_CS_Pin, GPIO_PIN_RESET);
}

void ILI9488_ChipDeselect() {
    HAL_GPIO_WritePin(ILI9488_CS_Port, ILI9488_CS_Pin, GPIO_PIN_SET);
}

static void ILI9488_Reset() {
    HAL_GPIO_WritePin(ILI9488_RST_Port, ILI9488_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(ILI9488_RST_Port, ILI9488_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(ILI9488_RST_Port, ILI9488_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(500);
}

static void ILI9488_WriteCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(ILI9488_DC_Port, ILI9488_DC_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&ILI9488_SPI_PORT, &cmd, sizeof(cmd), HAL_MAX_DELAY);
}

static void ILI9488_WriteData(uint8_t* buff, size_t buff_size) {
    HAL_GPIO_WritePin(ILI9488_DC_Port, ILI9488_DC_Pin, GPIO_PIN_SET);

    // split data in small chunks because HAL can't send more then 64K at once
    while(buff_size > 0) {
        uint16_t chunk_size = buff_size > 32768 ? 32768 : buff_size;
        HAL_SPI_Transmit(&ILI9488_SPI_PORT, buff, chunk_size, HAL_MAX_DELAY);
        buff += chunk_size;
        buff_size -= chunk_size;
    }
}

static void ILI9488_RGB666ToBytes(uint32_t colour, uint8_t* red, uint8_t* green, uint8_t* blue) {
    *red = (colour >> 10) & 0xFC;
    *green = (colour >> 4) & 0xFC;
    *blue = (colour & 0x3F) << 2;
}

void ILI9488_Init() {
    // 1.  Select and Reset the Chip
    ILI9488_ChipSelect();
    ILI9488_Reset();

    // 2. Software Reset
    ILI9488_WriteCommand(0x01);
    HAL_Delay(120); // Required 120ms delay

    // Positive Gamma Control
    // Parameter values are from: https://github.com/lvgl/lvgl_esp32_drivers/blob/master/lvgl_tft/ili9488.c
    ILI9488_WriteCommand(0xE0);
    {
        uint8_t data[] = { 0x00, 0x03, 0x09, 0x08, 0x16,  0x0A, 0x3F, 0x78,
                            0x4C, 0x09, 0x0A, 0x08, 0x16, 0x1A, 0x0F};
        ILI9488_WriteData(data, sizeof(data));
    }

    // Negative Gamma Control
    ILI9488_WriteCommand(0xE1);
    {
        uint8_t data[] = { 0x00, 0x16, 0x19, 0x03, 0x0F,  0x05, 0x32, 0x45,
                            0x46, 0x04, 0x0E, 0x0D, 0x35, 0x37, 0x0F};
        ILI9488_WriteData(data, sizeof(data));
    }

    // Power Control 1
    ILI9488_WriteCommand(0xC0);
    {
        uint8_t data[] = {0x17, 0x15};
        ILI9488_WriteData(data, sizeof(data));
    }

    // Power Control 2
    ILI9488_WriteCommand(0xC1);
    {
        uint8_t data[] = {0x41};
        ILI9488_WriteData(data, sizeof(data));
    }
    
    // VCOM Control
    ILI9488_WriteCommand(0xC5);
    {
        uint8_t data[] = {0x00, 0x12, 0x80};
        ILI9488_WriteData(data, sizeof(data));
    }

    // Memory Access Control
    ILI9488_WriteCommand(0x36);
    {
        #if LANDSCAPE_ORIENTATION
        // Landscape mode
        uint8_t data[] = {0xE8};
        #else
        // Portrait mode
        uint8_t data[] = {0x48};
        #endif
        
        ILI9488_WriteData(data, sizeof(data));
    }

    // Interface Pixel Format
    ILI9488_WriteCommand(0x3A);
    {
        // ILI9488 over SPI uses 18-bit RGB666 format (3 bytes per pixel)
        uint8_t data[] = {0x66};
        ILI9488_WriteData(data, sizeof(data));
    }

    // Interface Mode Control
    ILI9488_WriteCommand(0xB0);
    {
        uint8_t data[] = {0x00};
        ILI9488_WriteData(data, sizeof(data));
    }

    // Frame Rate Control
    ILI9488_WriteCommand(0xB1);
    {
        uint8_t data[] = {0xA0};
        ILI9488_WriteData(data, sizeof(data));
    }

    // Display Inversion Control
    ILI9488_WriteCommand(0xB4);
    {
        uint8_t data[] = {0x02};
        ILI9488_WriteData(data, sizeof(data));
    }

    // Display Function Control
    ILI9488_WriteCommand(0xB6);
    {
        uint8_t data[] = {0x02, 0x02, 0x3B};
        ILI9488_WriteData(data, sizeof(data));
    }

    // Entry Mode Set
    ILI9488_WriteCommand(0xB7);
    {
        uint8_t data[] = {0xC6};
        ILI9488_WriteData(data, sizeof(data));
    }

    // Sleep OUT
    ILI9488_WriteCommand(0x11);
    HAL_Delay(120); // Required 120ms delay

    // Display ON
    ILI9488_WriteCommand(0x29);
    HAL_Delay(120); // Delay after display ON

    ILI9488_ChipDeselect();
}

static void ILI9488_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    ILI9488_WriteCommand(0x2A); // Set Column Address
    uint8_t x_data[] = {x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF};
    ILI9488_WriteData(x_data, sizeof(x_data));

    ILI9488_WriteCommand(0x2B); // Set Page Address
    uint8_t y_data[] = {y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF};
    ILI9488_WriteData(y_data, sizeof(y_data));

    ILI9488_WriteCommand(0x2C); // Memory Write
}

void ILI9488_DrawPixel(uint16_t x, uint16_t y, uint32_t color) {
    if (x >= ILI9488_WIDTH || y >= ILI9488_HEIGHT) return;

    ILI9488_ChipSelect();
    ILI9488_SetAddressWindow(x, y, x, y);

    uint8_t red, green, blue;
    ILI9488_RGB666ToBytes(color, &red, &green, &blue);
    uint8_t data[] = {red, green, blue};
    ILI9488_WriteData(data, sizeof(data));

    ILI9488_ChipDeselect();
}

void ILI9488_FillScreen(uint32_t colour) {
    ILI9488_FillRectangle(0, 0, ILI9488_WIDTH, ILI9488_HEIGHT, colour);
}

void ILI9488_DrawHLine(uint16_t x, uint16_t y, uint16_t w, uint32_t colour) {
    if(w == 0) return;
    ILI9488_FillRectangle(x, y, w, 1, colour);
}

void ILI9488_DrawVLine(uint16_t x, uint16_t y, uint16_t h, uint32_t colour) {
    if(h == 0) return;
    ILI9488_FillRectangle(x, y, 1, h, colour);
}

static void ILI9488_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint32_t colour, uint32_t bgcolor) {
    uint32_t i, j;
    uint8_t fg[3], bg[3];
    uint8_t row_buf[3 * 32];

    if((x >= ILI9488_WIDTH) || (y >= ILI9488_HEIGHT)) return;
    if((x + font.width - 1) >= ILI9488_WIDTH) return;
    if((y + font.height - 1) >= ILI9488_HEIGHT) return;

    if((ch < 32) || (ch > 126)) ch = '?';

    ILI9488_RGB666ToBytes(colour, &fg[0], &fg[1], &fg[2]);
    ILI9488_RGB666ToBytes(bgcolor, &bg[0], &bg[1], &bg[2]);

    ILI9488_ChipSelect();
    ILI9488_SetAddressWindow(x, y, x + font.width - 1, y + font.height - 1);
    HAL_GPIO_WritePin(ILI9488_DC_Port, ILI9488_DC_Pin, GPIO_PIN_SET);

    if(font.width <= 32U) {
        for(i = 0; i < font.height; i++) {
            uint16_t row = font.data[(ch - 32) * font.height + i];
            for(j = 0; j < font.width; j++) {
                uint8_t* px = &row_buf[j * 3U];
                if((row << j) & 0x8000) {
                    px[0] = fg[0];
                    px[1] = fg[1];
                    px[2] = fg[2];
                } else {
                    px[0] = bg[0];
                    px[1] = bg[1];
                    px[2] = bg[2];
                }
            }
            HAL_SPI_Transmit(&ILI9488_SPI_PORT, row_buf, (uint16_t)(font.width * 3U), HAL_MAX_DELAY);
        }
    } else {
        for(i = 0; i < font.height; i++) {
            uint16_t row = font.data[(ch - 32) * font.height + i];
            for(j = 0; j < font.width; j++) {
                if((row << j) & 0x8000) {
                    HAL_SPI_Transmit(&ILI9488_SPI_PORT, fg, sizeof(fg), HAL_MAX_DELAY);
                } else {
                    HAL_SPI_Transmit(&ILI9488_SPI_PORT, bg, sizeof(bg), HAL_MAX_DELAY);
                }
            }
        }
    }
    ILI9488_ChipDeselect();
}

void ILI9488_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint32_t colour, uint32_t bgcolor) {
    uint16_t x_start = x;

    while(str && *str) {
        if(*str == '\r') {
            str++;
            continue;
        }

        if(*str == '\n') {
            x = x_start;
            y += font.height;
            str++;
            if((y + font.height - 1) >= ILI9488_HEIGHT) break;
            continue;
        }

        if((x + font.width - 1) >= ILI9488_WIDTH) {
            x = x_start;
            y += font.height;
        }

        if((y + font.height - 1) >= ILI9488_HEIGHT) break;

        ILI9488_WriteChar(x, y, *str, font, colour, bgcolor);
        x += font.width;
        str++;
    }
}

void ILI9488_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t colour) {
    uint32_t i, remaining_pixels = w * h;
    uint8_t burst_buf[3 * N_BURST_PIXELS];

    if ((x >= ILI9488_WIDTH) || (y >= ILI9488_HEIGHT)) return;
    if ((w == 0U) || (h == 0U)) return;
    if ((x + w - 1U) >= ILI9488_WIDTH) w = ILI9488_WIDTH - x;
    if ((y + h - 1U) >= ILI9488_HEIGHT) h = ILI9488_HEIGHT - y;

    {
        uint8_t red, green, blue;
        ILI9488_RGB666ToBytes(colour, &red, &green, &blue);
        for(i = 0; i < N_BURST_PIXELS; i++) {
            burst_buf[i * 3U + 0U] = red;
            burst_buf[i * 3U + 1U] = green;
            burst_buf[i * 3U + 2U] = blue;
        }
    }

    ILI9488_ChipSelect();
    ILI9488_SetAddressWindow(x, y, (x + w - 1U), (y + h - 1U));
    HAL_GPIO_WritePin(ILI9488_DC_Port, ILI9488_DC_Pin, GPIO_PIN_SET);

    while(remaining_pixels > 0U) {
        uint16_t this_burst = (remaining_pixels > N_BURST_PIXELS) ? N_BURST_PIXELS : remaining_pixels;
        HAL_SPI_Transmit(&ILI9488_SPI_PORT, burst_buf, (this_burst * 3U), HAL_MAX_DELAY);
        remaining_pixels -= this_burst;
    }

    ILI9488_ChipDeselect();
}


