#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "bmp280.h"
#include "ssd1306.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

int main()
{
    stdio_init_all();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    // Enable UART so we can print status output
    stdio_init_all();

    // init barometer
    bmp280_init();
    struct bmp280_calib_param params;
    bmp280_get_calib_params(&params);

    int32_t raw_pressure, raw_temperature;
    sleep_ms(250);

    // init display
    SSD1306_init();

    struct render_area frame_area = {
             start_col: 0,
             end_col : SSD1306_WIDTH - 1,
             start_page : 0,
             end_page : SSD1306_NUM_PAGES - 1
    };
    calc_render_area_buflen(&frame_area);
        // zero the entire display
    uint8_t buf[SSD1306_BUF_LEN];
    memset(buf, 0, SSD1306_BUF_LEN);
    render(buf, &frame_area);
    WriteString(buf, 5, 0, "this is baro");
    render(buf, &frame_area);
    char row[32];
    while (true)
    {
        bmp280_read_raw(&raw_temperature, &raw_pressure);
        int32_t temperature = bmp280_convert_temp(raw_temperature, &params);
        int32_t pressure = bmp280_convert_pressure_qs(raw_pressure, raw_temperature, &params);
        sprintf(row, "Pressure %.3f mmHg\n", pressure / 1000.f);
        printf(row);
        WriteString(buf, 5, 0, row);
        sprintf(row, "Temp. = %.2f C\n", temperature / 100.f);
        printf(row);
        WriteString(buf, 5, 16, row);
        render(buf, &frame_area);
        sleep_ms(1000);
    }
    return 0;
}
