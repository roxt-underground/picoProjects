#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "pico/binary_info.h"
#include "ST7789_const.h"
#include "ST7789api.hpp"
#include "pico/rand.h"
#include "tiny_gps/TinyGPS.h"
#include "hardware/irq.h"

// icons and fonts
#include "logo.hpp"
#include "font.hpp"
#include "fontMoby.hpp"
#include "aw.hpp"

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

// GPS-UART
#define UART_ID uart1
#define UART_IRQ UART1_IRQ
#define BAUD_RATE 9600
#define BAUD_RATE_NEXT 115200
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define MAX_NMEA_LENGTH 256
#define BUFF_LEN 1024

// Drag-time
#define DRAG_READY 0
#define DRAG_IN_PROCESS 1
#define DRAG_DONE 2
#define DRAG_FAIL 3

#define DRAG_SPEED_DRIFT 0.5

// Display settings
#define __swap
#ifdef __swap
#define LCD_W 320
#define LCD_H 172
#define x_offset 0
#define y_offset 34
#else
#define LCD_H 320
#define LCD_W 172
#define x_offset 34
#define y_offset 0
#endif

#define LOW 0
#define HIGH 1

#define timezone 3

#define delay sleep_ms

TinyGPS gps;
static int chars_rxed = 0;
static bool valid_sentence = false, speed_valid=false, valid_time=false;


// info
static float speed = 0;
static int sats = 0;
static uint8_t month, day, hour, minute, second, hundredths;

static uint64_t 
    last_valid_sent = 0, 
    last_second_update = 0;

// drag 
struct speedLog
{
    uint64_t moment;
    float speed;/* data */
};

speedLog speed_log[1000];
static uint16_t speed_log_cursor = 0;
static uint8_t drag_state = DRAG_FAIL;

void drag_speed_recived();

// void draw_logo(ST7789disp * display, uint16_t x, uint16_t y) {
//     display->setAddress(
//         x,
//         x + __YAMAHA96_WIDTH + -1,
//         y, 
//         y +  __YAMAHA96_HEIGHT + -1
//     );
//     display->prepareWrite();
//     display->writeData(__yamaha96_array, __YAMAHA96_SIZE);
// }


/**
 * @brief Initializes the UART interface for GPS communication.
 *
 * This function sets up the UART interface with the specified baud rate,
 * configures the TX and RX pins, and enables the UART FIFO.
 */
void on_uart_rx();

uint8_t tz_hour(uint8_t hr);

void gps_puts(char * sent) {
    uart_puts(UART_ID, sent);
    uart_putc(UART_ID, '\r');
    uart_putc(UART_ID, '\n');
}

void uart_gps_init()
{
    uart_init(UART_ID, BAUD_RATE);
    gps_puts("$PCAS01,5*19");
    sleep_ms(20);
    uart_init(UART_ID, BAUD_RATE_NEXT);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);

    uart_set_fifo_enabled(UART_ID, false);

    irq_set_exclusive_handler(UART1_IRQ, on_uart_rx);
    irq_set_enabled(UART1_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);

    gps_puts("$PCAS02,150*1B");
    gps_puts("$PCAS04,7*1E");
}


void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        if (ch == 0) {
            continue;
        }
        // printf("%c", ch);
        if(gps.encode(ch)) {
            valid_sentence = true;
            speed = gps.f_speed_kmph();
            speed_valid = true;
            last_valid_sent = time_us_64();
            drag_speed_recived();
        }
        chars_rxed++;
    }
}

void log_gps_stats() {
    unsigned long chars;
    unsigned short sentences, failed;
    uint8_t _second;

    uint8_t _sats = gps.satellites();

    if (_sats > 40) {
        _sats = 0;
    }

    if (_sats < 3) {
        speed_valid = false;
        drag_state = DRAG_FAIL;
    }
    if (speed_valid) {
        if (time_us_64() - last_valid_sent > 3000000) {
            speed_valid = false;
        }
    }
    sats = _sats;

    int year;
    unsigned long age;
    if (valid_sentence) {
        printf("found valid sentence!\n");
        valid_sentence = false;
    }
    gps.stats(&chars, &sentences, &failed);
    // printf("Stats: chars=%d sents=%d fail=%d\n", chars, sentences, failed);
    // printf("Visible sats: %d\n", sats);

    gps.crack_datetime(&year, &month, &day, &hour, &minute, &_second, &hundredths, &age);

    if (age != TinyGPS::GPS_INVALID_AGE)
    {
        valid_time = true;
        // printf("%02d/%02d/%02d %02d:%02d:%02d\n", day, month, year, hour, minute, _second);
        if (_second != second) {
            second = _second;
            last_second_update = time_us_64();
        }
    }
    // else printf("******* invalid date *******\n");
    if (time_us_64() - last_second_update > 3000000) valid_time = false;

    // detect chip code
    // printf("*** ask info");
    // uart_puts(UART_ID, "$PCAS06,0*1B\r\n");  // received: $GPTXT,01,01,02,SW=URANUS4,V4.3.0.5*18 
    
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

    display->setMemACL(ST7789_MADCTL_ROTATE_270);

    display->intit();
    display->setOffsetX(x_offset);
    display->setOffsetY(y_offset);

    uart_gps_init();

    FontApi * font = new FontApi(
        __CONTHRAX_SB_HEIGHT,
        __CONTHRAX_SB_WIDTH,
        (unsigned char *) __conthrax_sb_alphabet,
        __CONTHRAX_SB_ALPHABE_LEN,
        __conthrax_sb_array
    );

    FontApi * fontLarge = new FontApi(
        __MOBY_MONOSPACE_HEIGHT,
        __MOBY_MONOSPACE_WIDTH,
        (unsigned char *) __Moby_Monospace_alphabet,
        __MOBY_MONOSPACE_ALPHABE_LEN,
        __Moby_Monospace_array
    );

    FontApi * satFont = new FontApi(
        __SAT_ICONS_HEIGHT,
        __SAT_ICONS_WIDTH,
        (unsigned char *) __sat_icons_alphabet,
        __SAT_ICONS_ALPHABE_LEN,
        __sat_icons_array
    );
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    uint16_t i;

    sleep_ms(3000);

    uint16_t x ,y, color=0x0000;
    unsigned char buff[32];

    display->fill(0, LCD_W, 0, LCD_H, 0x0000);
    while (true) {

        // draw_logo(display, x, y);
        // Draw speed
        if (speed_valid) {
            sprintf((char *)buff, "%.1f   ", speed);
        }
        else {
            sprintf((char *)buff, "--.-  ");
        }
        if (speed < 100) {
            fontLarge->setCursor(10 + __MOBY_MONOSPACE_WIDTH, 5);
        }
        else {
            fontLarge->setCursor(10, 5);
        }
        fontLarge->writeBuff(display, buff, 32);

        // Draw drag
        font->setCursor(10, 100);
        sprintf((char *)buff, "%d", drag_state);
        font->writeBuff(display, buff, 32);

        if (drag_state == DRAG_IN_PROCESS) {
            sprintf((char *)buff, "%.1fs", 
                double(time_us_64() - speed_log[0].moment) / 1000000.0
            );
        }
        else if (drag_state == DRAG_DONE) {
            sprintf((char *)buff, "%.1fs", 
                double(speed_log[speed_log_cursor].moment - speed_log[0].moment) / 1000000.0
            );
        }
        else sprintf((char *)buff, "     ");
        font->setCursor(60, 100);
        font->writeBuff(display, buff, 32);

        // Draw sats
        if (sats < __SAT_ICONS_ALPHABE_LEN) buff[0] = sats+1;
        else buff[0] = __SAT_ICONS_ALPHABE_LEN;
        buff[1] = '\0';
        satFont->setCursor(240, 136);
        satFont->writeBuff(display, buff, 32);

        // Draw time
        if (valid_time) {
            sprintf((char *)buff, "%02d:%02d", tz_hour(hour), minute);
        }
        else {
            sprintf((char *)buff, "--:--");
        }
        font->setCursor(10, 137);
        font->writeBuff(display, buff, 32);

        display->setScrollPosition(0);

        log_gps_stats();
        sleep_ms(100);
    }
}


uint8_t tz_hour(uint8_t hr) {
    uint8_t new_hr = hr + timezone;
    new_hr = new_hr % 24;
    return new_hr;
}

void write_speed_log() {
        if (speed_log_cursor > 1000) {
            printf("Drag: fail too long race\n");
            drag_state = DRAG_FAIL;
            return;
        }
        speed_log[speed_log_cursor].moment = time_us_64();
        speed_log[speed_log_cursor].speed = speed;
        speed_log_cursor++;
}

void drag_speed_recived() {
    if (sats < 7) {
        printf("Drag: Fail on sats: %d\n", sats);
        drag_state = DRAG_FAIL;
        return;
    }
    if (speed_valid && (int(speed * 100) == 0)) {
        drag_state = DRAG_READY;
        speed_log_cursor = 0;
        write_speed_log();
        return;
    }
    if (drag_state == DRAG_READY) {
        drag_state = DRAG_IN_PROCESS;
        write_speed_log();
        return;
    }
    if (drag_state != DRAG_IN_PROCESS) return;
    printf("Drag: Running cursor:%d last log:%.4f last:%.4f time: %.3f\n", 
        speed_log_cursor, speed_log[speed_log_cursor-1].speed, speed, float(time_us_64() - speed_log[0].moment) / 1000000.0);
    if ((speed_log[speed_log_cursor - 1].speed - DRAG_SPEED_DRIFT) > speed) {
        printf("Drag: fail on sped_low: %.4f > %.4f\n", speed_log[speed_log_cursor-1].speed, speed);
        drag_state = DRAG_FAIL;
        return;
    }
    write_speed_log();
    if (int(speed * 100) > 10000) {
        drag_state = DRAG_DONE;
        return;
    }
}