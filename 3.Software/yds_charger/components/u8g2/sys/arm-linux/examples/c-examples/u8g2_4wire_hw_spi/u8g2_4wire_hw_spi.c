#include "u8g2port.h"

// GPIO chip number for character device
#define GPIO_CHIP_NUM 0
// SPI bus uses upper 4 bits and lower 4 bits, so 0x10 will be /dev/spidev1.0
#define SPI_BUS 0x10
#define OLED_SPI_PIN_RES            199
#define OLED_SPI_PIN_DC             198

// CS pin is controlled by linux spi driver, thus not defined here, but need to be wired
#define OLED_SPI_PIN_CS             U8X8_PIN_NONE


int main(void) {
	u8g2_t u8g2;

	// Initialization
	u8g2_Setup_ssd1306_128x64_noname_f(&u8g2, U8G2_R0,
			u8x8_byte_arm_linux_hw_spi, u8x8_arm_linux_gpio_and_delay);
	init_spi_hw(&u8g2, GPIO_CHIP_NUM, SPI_BUS, OLED_SPI_PIN_DC,
			OLED_SPI_PIN_RES, OLED_SPI_PIN_CS);

	u8g2_InitDisplay(&u8g2);
	u8g2_ClearBuffer(&u8g2);
	u8g2_SetPowerSave(&u8g2, 0);

	u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
	u8g2_DrawStr(&u8g2, 1, 18, "U8g2 HW SPI");

	u8g2_SetFont(&u8g2, u8g2_font_unifont_t_symbols);
	u8g2_DrawGlyph(&u8g2, 112, 56, 0x2603);

	u8g2_SendBuffer(&u8g2);

	printf("Initialized ...\n");
	sleep_ms(5000);
	u8g2_SetPowerSave(&u8g2, 1);
	// Close and deallocate SPI resources
	done_spi();
	// Close and deallocate GPIO resources
	done_user_data(&u8g2);
	printf("Done\n");
	return 0;
}
