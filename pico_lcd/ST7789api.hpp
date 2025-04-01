#pragma once
#include "pico/stdlib.h"
#include "hardware/spi.h"

#ifndef __ST7789API__
#define __ST7789API__

uint8_t * convert16to8(uint16_t *buff, uint16_t len);

class ST7789disp {
    private:   
        spi_inst_t *spi;
        uint8_t cs_pin, res_pin, dc_pin;
        uint16_t height, width, address_x_offset = 0, address_y_offset = 0;
        // use percistent buffers to prevent memory leaks [[[
        uint8_t addr_x[4], addr_y[4]; 
        uint8_t color_block[64];
        // ]]]
        void hardwareReset();
        void SPIConfig();
        void softwareReset();
    public:
        ST7789disp(
            spi_inst_t *spi,
            uint16_t height,
            uint16_t width,
            uint8_t cs_pin,
            uint8_t dc_pin,
            uint8_t res_pin
        );
        void intit();
        void setOffsetX(uint16_t offset);
        void setOffsetY(uint16_t offset);
        void writeCommand(uint8_t cmd);
        void writeCommandData(uint8_t cmd, uint8_t * data, uint len);
        void writeData(uint8_t * data, uint len);
        void setAddress(uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey);
        void putColorBuff(uint16_t * color, uint32_t len);
        void prepareWrite();
        void fill(uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey, uint16_t color);
};
#endif