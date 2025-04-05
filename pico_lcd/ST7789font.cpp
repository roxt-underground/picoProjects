#define __ST7789_FONT

#include "ST7789api.hpp"


FontApi::FontApi(
    uint16_t _font_height,
    uint16_t _font_width,
    unsigned char * alphabet,
    uint16_t alphabet_len,
    uint8_t * array_of_pixels
){
    uint16_t i;

    font_width = _font_width;
    font_height = _font_height;
    char_size = _font_height * _font_width * 2;
    _array_of_pixels = array_of_pixels;

    for (i=0; i < alphabet_len; i++) {
        table[alphabet[i]] = char_size * i;
    }
}

uint8_t * FontApi::getSimbolImage(unsigned char ch) {
    if (auto search = table.find(ch); search == table.end()) {
        return &_array_of_pixels[table['.']];
    }
    return &_array_of_pixels[table[ch]];
}

void FontApi::writeChar(
    ST7789disp * display, 
    unsigned char ch
){
    display->setAddress(
        cursor[0],
        cursor[0] + font_width - 1,
        cursor[1],
        cursor[1] + font_height - 1
    );
    display->prepareWrite();
    display->writeData(getSimbolImage(ch), char_size);
    cursor[0] += font_width;
}

void FontApi::writeBuff(
    ST7789disp * display,
    unsigned char * buff, 
    uint16_t len
) {
    uint16_t i;
    for (i=0; i< len; i++) {
        if (buff[i] == 0) return; // end of string
        writeChar(display, buff[i]);
    }
}