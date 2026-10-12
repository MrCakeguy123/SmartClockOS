# ESP32-8048S070 hardware notes

Board used by SmartClockOS: **Sunton ESP32-8048S070** — 7.0" 800x480 RGB LCD with
ESP32-S3-WROOM-1 (16 MB flash, 8 MB OPI PSRAM), GT911 capacitive touch, and an
I2S audio amp.

The vendor's schematics, datasheets and getting-started guide are not copied
into this repo. They can be downloaded from
http://specs.buho.ch/MCU/ESP32-8048S070/

See `BME280_wiring.md` for how the indoor sensor is wired.

## Pin map (as wired in SmartClockOS `Config.h`, verified working)

**RGB LCD (16-bit RGB565 parallel):**
- Control: DE=41, VSYNC=40, HSYNC=39, PCLK=42, Backlight PWM=2
- Red: R0-R4 = 14, 21, 47, 48, 45
- Green: G0-G5 = 9, 46, 3, 8, 16, 1
- Blue: B0-B4 = 15, 7, 6, 5, 4
- Timing: HFP 210 / HPW 30 / HBP 16, VFP 22 / VPW 13 / VBP 10, PCLK 14 MHz

**GT911 touch (I2C0):** SDA=19, SCL=20, INT=18, RST=38, addr 0x5D (alt 0x14)

**External BME280 (I2C1, added sensor):** SDA=11, SCL=12, addr 0x76

**Note (rev V1.1):** audio BCLK is IO0, not IO19 as on V1.0 — IO19 is used for touch SDA,
so on V1.0 boards audio and touch conflict.

## Arduino build settings

FQBN: `esp32:esp32:esp32s3:PartitionScheme=no_fs,PSRAM=opi` (see `../sketch.yaml`),
custom partition table in `../partitions.csv` (dual OTA app slots, no filesystem).
