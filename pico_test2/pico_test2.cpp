#include <stdio.h>
#include "colors.hpp"
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "generated/ws2812.pio.h"

#define IS_RGBW false

#define SIMPLE_LED_PIN 25
#define RGB_LED_PIN 23
#define LED_LENGTH 5

#define NUM_PIXELS 31

#define CLICK_PIN 24

#define fill_end if(btn->pressed()){printf("pressed\n");btn->tic();return;}btn->tic();sleep_ms(50);

const char color_names[3] = {'R', 'G', 'B'};


static inline void put_pixel(PIO pio, uint sm, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

static inline uint32_t urgbw_u32(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            ((uint32_t) (w) << 24) |
            (uint32_t) (b);
}


void get_random_color(uint8_t * _color, uint8_t max_value) {
    _color[0] = rand() % max_value;
    _color[1] = rand() % max_value;
    _color[2] = rand() % max_value; 
}

void get_random_color(uint8_t * _color) {
    get_random_color(_color, 255);
}

uint32_t get_random_dominant_color() {
    uint8_t hue = rand() % 360;
    return hslToRGB(hue, 255, 127);
}


void write_byte(unsigned char _value) {
    uint8_t up_time, down_time;
    for (uint8_t i=0; i<8; i++) {
        bool n = _value & 0x01;
        _value = _value >> 1;
        if (n) {
            up_time = 8;
            down_time = 5;
        }
        else {
            up_time = 4;
            down_time = 9;
        }
       gpio_put(RGB_LED_PIN, true);
       sleep_us(up_time);
       gpio_put(RGB_LED_PIN, false);
       sleep_us(down_time);
    }
}

class Button {
    public:
        Button(uint8_t pin) {
            this->pin = pin;
            gpio_set_dir(this->pin, GPIO_IN);
            gpio_pull_up(this->pin);
            this->last_state = gpio_get(this->pin);
        };

        bool pressed() {
            // printf("State: %d\n", this->last_state && !(gpio_get(this->pin)));
            return this->last_state && !(gpio_get(this->pin));
            // return !(this->last_state) && gpio_get(this->pin);
        };

        void tic() {
            this->last_state = gpio_get(this->pin);
        };
    
    private:
        uint8_t pin;
        bool last_state;

};

uint16_t hue = 0;
void base_rainbow_fill(uint8_t brightnes, Button *btn, PIO pio, uint sm) {
    uint8_t i;

    while (true)
    {   
        for (i=0; i < NUM_PIXELS; i++) 
        {
            put_pixel(pio, sm, hslToRGB((hue + (i * 4)) % 360, 255, brightnes));
        }
        hue = (hue + 1) % 360;
        fill_end
    }
}
void rainbow_fill(Button *btn, PIO pio, uint sm) {
    static uint8_t br_choices[5] = {127, 80, 50, 20, 10};
    for (uint8_t i=1; i < 5; i++) {
        base_rainbow_fill(br_choices[i], btn, pio, sm);
    }
}
// void rainbow_fill_1(Button *btn, PIO pio, uint sm) {
//     base_rainbow_fill(127, btn, pio, sm);
// }
// void rainbow_fill_2(Button *btn, PIO pio, uint sm) {
//     base_rainbow_fill(80, btn, pio, sm);
// }
// void rainbow_fill_3(Button *btn, PIO pio, uint sm) {
//     base_rainbow_fill(50, btn, pio, sm);
// }
// void rainbow_fill_4(Button *btn, PIO pio, uint sm) {
//     base_rainbow_fill(20, btn, pio, sm);
// }
// void rainbow_fill_5(Button *btn, PIO pio, uint sm) {
//     base_rainbow_fill(10, btn, pio, sm);
// }

void base_fill(uint32_t color, Button *btn, PIO pio, uint sm){
    uint8_t i;
    for (i=0; i < NUM_PIXELS; i++) put_pixel(pio, sm, color);
    while (true)
    {
        fill_end
    } 
}

void warm_fill(Button *btn, PIO pio, uint sm){
    base_fill(hslToRGB(40, 255, 150), btn, pio, sm);
}

void cold_fill(Button *btn, PIO pio, uint sm){
    base_fill(hslToRGB(200, 255, 200), btn, pio, sm);
}

void fill_colors(Button *btn, PIO pio, uint sm) {
    for (uint16_t i=0; i < 360; i+=40) {
        base_fill(hslToRGB(i, 255, 127), btn, pio, sm);
    }
}

#define fire_nums 4
#define br_dif 24
int16_t fires[fire_nums] = {0, 5 * br_dif, 14 * br_dif, 20 * br_dif};
void running_fire(Button *btn, PIO pio, uint sm) {
    int16_t br = 127;
    uint32_t pixels[NUM_PIXELS];
    while (true)
    {
        for (uint8_t i=0; i < NUM_PIXELS; i++) pixels[i] = 0;
        for (int8_t n = 0; n < fire_nums; n++){
            br = 127 * (fires[n] % br_dif) / br_dif;
            pixels[int16_t (fires[n] / br_dif)] = hslToRGB((hue + n * 16) % 360, 255, 127 - br);
            pixels[(int16_t (fires[n] / br_dif) + 1) % NUM_PIXELS] = hslToRGB((hue + n * 16) % 360, 255, br);
        }
        for (uint8_t i=0; i < NUM_PIXELS; i++) {
            put_pixel(pio, sm, pixels[i]);         
        }
        hue = (hue + 2) % 360; 
        for (int8_t n = 0; n < fire_nums; n++) fires[n] = (fires[n] + 1) % (NUM_PIXELS * br_dif);
        fill_end
    }
    
}

// void red_fill(Button *btn, PIO pio, uint sm){
//     base_fill(hslToRGB(0, 255, 127), btn, pio, sm);
// }

// void purpur_fill(Button *btn, PIO pio, uint sm){
//     base_fill(hslToRGB(320, 255, 127), btn, pio, sm);
// }

int main()
{
    stdio_init_all();

    // led
    gpio_init(SIMPLE_LED_PIN);
    gpio_set_dir(SIMPLE_LED_PIN, GPIO_OUT);
    Button btn = Button(CLICK_PIN);

    bool led_state = true;
    uint8_t color[3];

    #define color_ch_len 5
    typedef void (*fillProgrPointer)(Button *, PIO, uint);
    static fillProgrPointer fill_programs[color_ch_len] = {
        &rainbow_fill,
        &running_fire,
        &warm_fill,
        &cold_fill,
        &fill_colors,
    };

    PIO pio;
    uint sm;
    uint offset;

    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, RGB_LED_PIN, 1, true);
    hard_assert(success);
    
    ws2812_program_init(pio, sm, offset, RGB_LED_PIN, 800000, IS_RGBW);

    // pio_gpio_init(pio0, RGB_LED_PIN);

    #ifdef PREBLINK
    for (int i; i < 20; i++) {
        sleep_ms(300);
        gpio_put(SIMPLE_LED_PIN, led_state);
        if (led_state) {
            put_pixel(pio, sm, get_random_dominant_color());
        }
        else {
            get_random_color(color, 32);
            printf("%d rgb(%d, %d, %d)\n", led_state, color[0], color[1], color[2]);
            put_pixel(pio, sm, urgb_u32(color[0], color[1], color[2]));
        }
        led_state = !led_state;
    }
    gpio_put(SIMPLE_LED_PIN, false);
    sleep_ms(3);
    #endif

    uint8_t i;

    i = 0;
    while (true)
    {   
        fill_programs[i](&btn, pio, sm);
        i++;
        i = i % color_ch_len;
        sleep_ms(10);
    }
}

