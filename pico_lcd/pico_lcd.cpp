#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/binary_info.h"
#include "ST7789_const.h"
#include "ST7789api.hpp"

#include "logo.hpp"

// #include "ST7789.hpp"

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
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    uint16_t i;

    sleep_ms(3000);

    uint16_t x ,y, color=0x0000;

    while (true) {

        display->fill(0, LCD_W, 0, LCD_H, 0x0000);
        for (i=0; i < 128; i++) {
            x = rand() % (LCD_W - 5);//__YAMAHA96_WIDTH);
            y = rand() % (LCD_H - 5);//__YAMAHA96_HEIGHT);
            color = rand() % 0xFFFF;

            display->fill(x, x+5, y, y+5, color);
            // display->setAddress(
            //     x + _x,
            //     x + __YAMAHA96_WIDTH + _x -1,
            //     y + _y, 
            //     y +  __YAMAHA96_HEIGHT + _y -1
            // );
            // display->prepareWrite();
            // display->writeData(__yamaha96_array, __YAMAHA96_SIZE);
            sleep_ms(10);
        }
        x = rand() % (LCD_W - __YAMAHA96_WIDTH);
        y = rand() % (LCD_H - __YAMAHA96_HEIGHT);
        display->setAddress(
            x,
            x + __YAMAHA96_WIDTH + -1,
            y, 
            y +  __YAMAHA96_HEIGHT + -1
        );
        display->prepareWrite();
        display->writeData(__yamaha96_array, __YAMAHA96_SIZE);

        sleep_ms(100);
    }
}