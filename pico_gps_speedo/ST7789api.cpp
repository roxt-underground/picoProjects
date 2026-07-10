#include <stdlib.h>
#include <unordered_map>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "ST7789_const.h"

#include "ST7789api.hpp"


void ST7789disp::hardwareReset() {

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(cs_pin);
    gpio_set_dir(cs_pin, GPIO_OUT);
    gpio_put(cs_pin, 1);
    bi_decl(bi_1pin_with_name(cs_pin, "SPI CS"));

    gpio_init(res_pin);
    gpio_set_dir(res_pin, GPIO_OUT);
    gpio_put(res_pin, 1);
    bi_decl(bi_1pin_with_name(res_pin, "DISPLAY RES"));
    

    gpio_init(dc_pin);
    gpio_set_dir(dc_pin, GPIO_OUT);
    gpio_put(dc_pin, 1);
    bi_decl(bi_1pin_with_name(dc_pin, "DISPLAY DC"));

    gpio_put(this->res_pin, 1);
    sleep_ms(5);
    gpio_put(this->res_pin, 0);
    sleep_ms(5);
    gpio_put(this->res_pin, 1);
    sleep_ms(5);
}

void ST7789disp::SPIConfig() {
    spi_set_format(this->spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}

void ST7789disp::softwareReset() {
        // SWRESET
        writeCommand(ST7789_CMD_SWRESET);
        sleep_ms(150);
    
        // SLPOUT
        writeCommand(ST7789_CMD_SLPOUT);
        sleep_ms(5);

        // COLMOD
        writeCommandData(ST7789_CMD_COLMOD, (uint8_t[]){ST7789_COLMOD_65K_16BIT}, 1);
        sleep_ms(5);

        // MADCTL
        writeCommandData(ST7789_CMD_MADCTL, &memory_data_acl_conf, 1);
        sleep_ms(5);
        setAddress(0, height, 0, width);

        // INVON
        writeCommand(ST7789_CMD_INVON);
        sleep_ms(5);
        
        // NORON partial mode off
        writeCommand(ST7789_CMD_NORON);
        sleep_ms(5);
        
        // DISPON
        writeCommand(ST7789_CMD_DISPON);
        sleep_ms(5); 
}
ST7789disp::ST7789disp(
    spi_inst_t *spi,
    uint16_t height,
    uint16_t width,
    uint8_t cs_pin,
    uint8_t dc_pin,
    uint8_t res_pin
) {
    this->spi = spi;
    this->height = height;
    this->cs_pin = cs_pin;
    this->dc_pin = dc_pin;
    this->res_pin = res_pin;
    memory_data_acl_conf = ST7789_MADCTL_DEFAULT;
}

void ST7789disp::intit() {
    SPIConfig();
    hardwareReset();
    softwareReset();
}

void ST7789disp::setMemACL(uint8_t config) {
    memory_data_acl_conf = config;
}

void ST7789disp::writeCommand(uint8_t cmd) {
    sleep_us(1);
    gpio_put(cs_pin, 0);
    gpio_put(dc_pin, 0);
    sleep_us(1);
    spi_write_blocking(spi, &cmd, sizeof(cmd));
    sleep_us(1);
    gpio_put(cs_pin, 1);
    sleep_us(1);
}

void ST7789disp::writeCommandData(uint8_t cmd, uint8_t * data, uint len){
    sleep_us(1);
    gpio_put(cs_pin, 0);
    gpio_put(dc_pin, 0);
    sleep_us(1);

    spi_write_blocking(spi, &cmd, sizeof(cmd));
    sleep_us(1);
    gpio_put(dc_pin, 1);
    sleep_us(1);
    // printf("%02x ", cmd);
    if (len) {
        // for (uint i=0; i<len; i++) {
        //     printf("%02x ",  data[i]);
        // }
        spi_write_blocking(spi, data, len);
    }
    // printf("\n");
    gpio_put(cs_pin, 1);
}

void _split(uint8_t *dest, uint16_t value){
    dest[0] = (uint8_t)(value >> 8);
    dest[1] = (uint8_t)(value);
}

void ST7789disp::setOffsetX(uint16_t offset){
    address_x_offset = offset;
}
void ST7789disp::setOffsetY(uint16_t offset){
    address_y_offset = offset;
}
void ST7789disp::setAddress(uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey) {
    uint8_t * buff;

    _split(&addr_x[0], sx + address_x_offset);
    _split(&addr_x[2], ex + address_x_offset);
    _split(&addr_y[0], sy + address_y_offset);
    _split(&addr_y[2], ey + address_y_offset);

    writeCommandData(ST7789_CMD_CASET, addr_x, 4);
    writeCommandData(ST7789_CMD_RASET, addr_y, 4);
}

void ST7789disp::putColorBuff(uint16_t * color, uint32_t len) {
    uint8_t * buff = convert16to8(color, len);
    writeCommandData(ST7789_CMD_RAMWR, buff, len * 2);
    free(buff);
}

void ST7789disp::writeData(uint8_t * data, uint len){
    gpio_put(cs_pin, 0);
    gpio_put(dc_pin, 1);
    sleep_us(1);
    spi_write_blocking(spi, data, len);
    sleep_us(1);
    gpio_put(cs_pin, 1);
    sleep_us(1);
}

void ST7789disp::prepareWrite() {
    writeCommand(ST7789_CMD_RAMWR);
}
void ST7789disp::fill(uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey, uint16_t color) {
    const uint32_t size = (abs(ex-sx)+1) * (abs(ey-sy)+1);
    uint i;
    for (i=0; i < 63; i+=2) { 
        _split(&color_block[i], color);
    }
    setAddress(sx, ex, sy, ey);
    prepareWrite();
    for (i=0; i<=size; i+=32) {
        writeData(color_block, 64);
    }
}

void ST7789disp::setScrollArea(uint16_t top_fixed_area, uint16_t vertical_scroll_area, uint16_t bottom_fixed_area)  {
    // Vertical scrolling arrea
    _split(&scroll_config[0], top_fixed_area);
    _split(&scroll_config[2], vertical_scroll_area);
    _split(&scroll_config[4], bottom_fixed_area);
    writeCommandData(ST7789_CMD_VSCRDEF, scroll_config, 6);
}

void ST7789disp::setScrollPosition(uint16_t vertical_scroll_position) {
    _split(scroll_position, vertical_scroll_position);
    writeCommandData(ST7789_CMD_VSCRSADD, scroll_position, 2);
}

uint8_t * convert16to8(uint16_t *buff, uint16_t len){
    uint8_t * data = (uint8_t *)malloc(sizeof(uint8_t) * len *2);
    uint i;
    for (i=0; i < len; i++) {
        data[i*2] = (uint8_t) (buff[i] >> 8);
        data[i*2 + 1] = (uint8_t) buff[i];
    }
    return data;
    free(buff);
}
