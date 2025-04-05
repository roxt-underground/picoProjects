# LCD project
This projec about self-made library for display on ST7789 driver over SPI

My display has pins:

| pin | description                                                                   |
|-----|-------------------------------------------------------------------------------|
| GND | this pin for ground (-)                                                       |
| VDD | power supply (3.3v+)                                                          |
| SCL | data clock, typically called as SCLK or SCK                                   |
| SDA | data input, same as MOSI                                                      |
| RES | hardware reset. Must be pulled up when display working. Pull down for reset.  |
| DC  | Switch beatween data and commands. Pull down for command. Pull up for data.   |
| CS  | SPI bus device select. Pull down for select current device.                   |
| BL  | Backlight power supply. Can be connected to VDD or dedicated PWM pin.         |


# Thanks:

- Take some examples from https://github.com/ArmDeveloperEcosystem/
- Upir on you youtube: https://www.youtube.com/watch?v=pbqgrv5YSf0 
- Display driver datasheet https://www.waveshare.com/w/upload/a/ae/ST7789_Datasheet.pdf

