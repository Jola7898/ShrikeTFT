# ShrikeTFT

Driving a 2.4" **ST7789 240x320 SPI TFT** from the ESP32-S3 on the
[Vicharak Shrike-Fi](https://vicharak.in) (ESP32-S3 + FPGA) board, using
PlatformIO, the Arduino framework and [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI).

The FPGA isn't used here. The display hangs directly off the ESP32's SPI pins.

## What it does

1. Fills the screen red, green, blue, white, then black (a quick colour and wiring check)
2. Shows a splash screen with the resolution and pin map
3. Bounces a colour-changing box around, with a live FPS counter in a status bar

It runs at around 90 fps at 40 MHz SPI.

## Hardware

- Vicharak Shrike-Fi (ESP32-S3FN8)
- 2.4" ST7789 240x320 SPI display (9 pins: VCC, GND, CS, RST, DC, MOSI, SCK, LED, MISO)
- 9 jumper wires

## Wiring

| Display | Shrike-Fi | Notes |
|---------|-----------|-------|
| VCC     | 3.3V      | |
| GND     | GND       | Make sure this one is solid (see troubleshooting) |
| SCK     | ESP_IO36  | SPI clock |
| MOSI    | ESP_IO35  | SPI data out |
| MISO    | ESP_IO37  | Only used for reading back from the display |
| CS      | ESP_IO34  | Chip select |
| DC      | ESP_IO33  | Data / command |
| RST     | ESP_IO38  | Reset |
| LED     | ESP_IO18  | Backlight (can also go straight to 3.3V) |

IO33–IO38 on the right-hand header are the ESP32-S3's native FSPI pins, so the
display gets full-speed SPI.

Pins to avoid:

- **GPIO8–13**: used internally by the ESP32 to program and talk to the FPGA
- **IO29**: connected to the flash chip
- **IO0, IO45, IO46**: strapping pins that affect boot

The full board pinout is in [`docs/shrike_fi_pinouts.jpg`](docs/shrike_fi_pinouts.jpg).

## Building and flashing

You need [PlatformIO](https://platformio.org/) (the VS Code extension or the CLI).

```bash
pio run                 # build
pio run -t upload       # flash
pio device monitor      # serial output at 115200
```

TFT_eSPI is downloaded automatically from `lib_deps`, so there's nothing to install by hand.

## Configuration

All the display settings are `-D` flags in [`platformio.ini`](platformio.ini):
driver, resolution, pins, fonts and SPI speed. The library's `User_Setup.h` is
never edited, so updating TFT_eSPI won't break anything.

Two flags matter for this board:

- `USE_FSPI_PORT=1`: **required** on the ESP32-S3 with Arduino core 2.0.x.
  Without it, TFT_eSPI writes to the wrong SPI peripheral's registers and the
  display never receives anything, even though the code appears to run fine.
- `TFT_INVERSION_OFF=1`: my panel shows inverted colours with the ST7789 default
  (white background instead of black). Remove this if yours looks inverted.

## Troubleshooting

| Symptom | Likely cause |
|---------|--------------|
| Screen completely dark | No power or backlight. Check GND and VCC first (a loose ground wire was the problem for me), then try LED straight to 3.3V |
| Backlight on but plain white | No data reaching the display. Check CS, DC, RST, SCK and MOSI, and make sure `USE_FSPI_PORT=1` is set |
| Inverted colours (white background) | Toggle `TFT_INVERSION_OFF` / `TFT_INVERSION_ON` |
| Red and blue swapped | Uncomment `TFT_RGB_ORDER=TFT_BGR` |
| Glitches or noise | Lower `SPI_FREQUENCY` to `27000000`, and use shorter wires |
| Picture offset or garbled at the edges | The module may actually be an ILI9341. Swap `ST7789_DRIVER` for `ILI9341_DRIVER` |

## Project layout

```
├── platformio.ini     board, library and display config
├── src/main.cpp       demo sketch
└── docs/              board pinout
```
