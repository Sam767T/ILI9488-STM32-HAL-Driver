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

## Notes
* Contrary to what the datasheet states, only RGB666 is supported via 3/4-wire SPI. RBG565 does not work unfortunately.
* Through experimentation, I found the maximum achievable clock speed to be 30Mhz using an STM32U5 and DFR0669.

## References
Inspired by: https://github.com/afiskon/stm32-ssd1351

LCD initialisation parameters: https://github.com/lvgl/lvgl_esp32_drivers/blob/master/lvgl_tft/ili9488.c 

## Example Code
```c
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "icache.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ILI9488.h"
#include "stm32u5xx_hal.h"
#include "fonts.h"
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemPower_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void init(void) {
	ILI9488_ChipDeselect();
	ILI9488_Init();
}

void showColourPalette(void) {
	// This function is intended for use in landscape mode
	uint32_t colours[] = {RGB666_BLACK, RGB666_WHITE, RGB666_BLUE, RGB666_RED, RGB666_GREEN, 
							RGB666_CYAN, RGB666_MAGENTA, RGB666_YELLOW};
	for (int i = 0; i < 8; i++) {
		ILI9488_FillRectangle(0 + (60 * i), 0, 60, ILI9488_HEIGHT, colours[i]);
	}
}

void updateCountdown(char* counter) {
	ILI9488_WriteString(232, 210, counter, Font_16x26, RGB666_BLUE, RGB666_WHITE);
}

void runCountdown(int N) {
	char str[11];
	while (N >= 0) {
		sprintf(str, "%d", N);
		updateCountdown(str);
		N--;
		HAL_Delay(1000);
	}
}

void loop(void) {
	ILI9488_FillScreen(RGB666_WHITE);
	ILI9488_WriteString(20, 20, "ILI9488 Driver by SAM767T", Font_16x26, RGB666_BLACK, RGB666_WHITE);
	ILI9488_WriteString(20, 60, "This is an average font.", Font_11x18, RGB666_BLACK, RGB666_WHITE);
	ILI9488_WriteString(20, 85, "This is a barely readable font. Terms and conditions apply.", Font_7x10, RGB666_BLACK, RGB666_WHITE);
	ILI9488_FillRectangle(200, 110, 80, 20, RGB666_YELLOW);
	ILI9488_DrawHLine(100, 140, 280, RGB666_BLUE);
	ILI9488_DrawHLine(100, 150, 280, RGB666_MAGENTA);

	HAL_Delay(1000);
	ILI9488_WriteString(85, 170, "Displaying Colour Palette In", Font_11x18, RGB666_BLACK, RGB666_WHITE);
	ILI9488_WriteString(160, 250, "Indeterminate Time Units", Font_7x10, RGB666_BLACK, RGB666_WHITE);
	runCountdown(9);

	showColourPalette();
	HAL_Delay(5000);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the System Power */
  SystemPower_Config();

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ICACHE_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	loop();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

```