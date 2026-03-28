# ILI9488 STM32 HAL Driver
This driver was written to drive the ILI9488 IC via 4-wire SPI.
Three GPIO outputs must be configured in addition to SPI.

## Setup
Simply copy the `Lib` folder into the project `Core` directory. Make sure to `#include` the library in your main file and add it to `CMakeLists.txt` (if using CMake).

For boards outside the STM32U5 family, this line must be modified:
```c
#include "stm32u5xx_hal.h"  // line 9 of ILI9488.c
```

Pin definitions must also be corrected in `ILI9488.h` (line 20). Alternatively, the following labels can be applied to arbitrary output GPIOs in CubeMX:
* `LCD_RST`: Reset
* `LCD_DC`: Data/Command
* `LCD_CS`: Chip Select

If CubeMX is used to configure SPI, set `mode` to `Transmit Only Master` and `Data Size` to `8 bits`.

By default, the display is in landscape mode, but portrait can be selected by setting `LANDSCAPE_ORIENTATION` to `0` in the header file.

Finally, this library relies on the STM32 HAL library.

## How to Use
An example `main.c` file is provided, it contains a demonstration program for the LCD. To use it in your own main file, simply copy the contents of all user code sections.

## Notes
* Contrary to what the datasheet states, only RGB666 is supported via 3/4-wire SPI. RBG565 does not work unfortunately (I tried it).
* Through experimentation, I found the maximum achievable clock speed to be 30Mhz using an STM32U5 and DFR0669.

## References
Inspired by: https://github.com/afiskon/stm32-ssd1351

LCD initialisation parameters: https://github.com/lvgl/lvgl_esp32_drivers/blob/master/lvgl_tft/ili9488.c 
