#pragma once
#include <unordered_map>
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
        uint8_t memory_data_acl_conf;
        // use percistent buffers to prevent memory leaks [[[
        uint8_t addr_x[4], addr_y[4]; 
        uint8_t color_block[64];
        uint8_t scroll_config[6]; // 3 params per 2 bytes
        uint8_t scroll_position[2];
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
        void setMemACL(uint8_t config);
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

        void setScrollArea(uint16_t top_fixed_area, uint16_t vertical_scroll_area, uint16_t bottom_fixed_area);
        void setScrollFullScreen() {setScrollArea(0, height, 0);};

        void setScrollPosition(uint16_t vertical_scroll_position);

};

class FontApi {
    private:
        uint16_t font_height;
        uint16_t font_width;
        std::unordered_map<unsigned char, uint32_t> table;
        uint8_t * _array_of_pixels;
    protected:
        uint16_t cursor[2];
        uint32_t char_size;
    public:
        FontApi(
            uint16_t font_height,
            uint16_t font_weght,
            unsigned char * alphabet,
            uint16_t alphabet_len,
            uint8_t * array_of_pixels
        );
        uint8_t * getSimbolImage(unsigned char ch);
        uint32_t getCharSize() { return char_size; }
        void setCursor(uint16_t x, uint16_t y) {cursor[0] = x; cursor[1] = y;};
        void writeChar(
            ST7789disp * display, 
            unsigned char ch
        );
        void writeBuff(ST7789disp * display, unsigned char * buff, uint16_t len);
};

#endif