#include <stdlib.h>  
#include "pico/stdlib.h"

uint32_t hslToRGB(uint16_t hue, uint8_t s, uint8_t l) {
    uint8_t _c = ((255 - abs(2 * l - 255)) * s) / 255;
    uint16_t h_step = (hue * 100) / 60;
    uint8_t _x =  _c * (100 - abs(h_step % 200 - 100)) / 100;

    uint8_t _r = 0, _g = 0, _b = 0;
    
    switch ((int) h_step / 100)
    {
    case 0:
        _r = _c;
        _g = _x;
        break;
    case 1:
        _r = _x;
        _g = _c;
        break;
    case 2:
        _g = _c;
        _b = _x;
        break;
    case 3:
        _g = _x;
        _b = _c;
        break;
    case 4:
        _r = _x;
        _b = _c;
        break;
    default:
        _r = _c;
        _b = _x;
        break;
    }

    uint8_t _m = l - _c / 2;
    _r += _m;
    _g += _m;
    _b += _m;

    uint32_t ressult = ((uint32_t) (_r) << 8) | ((uint32_t) (_g) << 16) | (uint32_t) (_b);
    return ressult;
}