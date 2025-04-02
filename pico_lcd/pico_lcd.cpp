#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/binary_info.h"
#include "ST7789_const.h"
#include "ST7789api.hpp"
#include "pico/rand.h"

#include "logo.hpp"
#include "font.hpp"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
// MOSI == master TX
// MISO == mastre RX
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19 
#define PIN_RES 20
#define PIN_DC 21

#define LCD_H 320
#define LCD_W 172

#define LOW 0
#define HIGH 1

#define delay sleep_ms

#define x_offset 34
#define y_offset 0

void draw_logo(ST7789disp * display, uint16_t x, uint16_t y) {
    display->setAddress(
        x,
        x + __YAMAHA96_WIDTH + -1,
        y, 
        y +  __YAMAHA96_HEIGHT + -1
    );
    display->prepareWrite();
    display->writeData(__yamaha96_array, __YAMAHA96_SIZE);
}

int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 200*1000*1000);
    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
        // Make the SPI pins available to picotool

    
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    bi_decl(bi_2pins_with_func(PIN_MOSI, PIN_SCK, GPIO_FUNC_SPI));

    ST7789disp * display = new ST7789disp(
        SPI_PORT, 
        LCD_H, 
        LCD_W, 
        PIN_CS, 
        PIN_DC, 
        PIN_RES
    );
    display->intit();
    display->setOffsetX(x_offset);
    display->setOffsetY(y_offset);

    FontApi * font = new FontApi(
        __CONTHRAX_SB_HEIGHT,
        __CONTHRAX_SB_WIDTH,
        (unsigned char *) __conthrax_sb_alphabet, 
        __CONTHRAX_SB_ALPHABE_LEN,
        __conthrax_sb_array
    );
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    uint16_t i;

    sleep_ms(3000);

    uint16_t x ,y, color=0x0000;
    unsigned char buff[32];

    while (true) {

        display->fill(0, LCD_W, 0, LCD_H, 0x0000);
        x = get_rand_32() % (LCD_W - __YAMAHA96_HEIGHT);
        y = get_rand_32() % (LCD_H - __YAMAHA96_WIDTH);

        draw_logo(display, x, y);

        sprintf((char *)buff, "X=%d", x);
        font->setCursor(10, 10);
        font->writeBuff(display, buff, 32);

        sprintf((char *)buff, "Y=%d", y);
        font->setCursor(10, 36);
        font->writeBuff(display, buff, 32);

        for (i=0; i < 128; i++) {
            x = get_rand_32() % (LCD_W - 5);
            y = get_rand_32() % (LCD_H - 5);
            color = get_rand_32() % 0xFFFF;
            display->fill(x, x+5, y, y+5, color);
            sleep_ms(10);
        }
        sleep_ms(100);
    }
}