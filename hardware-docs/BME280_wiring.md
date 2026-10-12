# BME280 wiring — SmartClockOS (ESP32-8048S070)

Verified working config. Genuine BME280 (chip ID 0x60) answering at **0x76**
on the **Wire1** I²C bus.

| BME280 pin | Wire | ESP32 board pin | Connector |
|------------|------|-----------------|-----------|
| VCC        | red    | 3.3 V | P4 |
| GND        | black  | GND   | P4 |
| SDA        | blue   | IO11  | P2 ("SPI") |
| SCL        | yellow | IO12  | P2 ("SPI") |

Firmware pins (`Config.h`): `BME280_SDA 11`, `BME280_SCL 12`, `BME280_ADDR 0x76`.

## Don't cross these wires
- **P2** also carries **IO19** (that's the *touch* SDA — leave it alone) and
  **IO13** (unused, shares the TF-card lines). Only IO11 and IO12 go to the
  sensor.
- **P4** also carries **IO18**, which is the GT911 touch interrupt (TP_INT) —
  off-limits. Only 3.3 V and GND go to the sensor.
- Power the sensor from **3.3 V**, never 5 V (the module's regulator is fine at
  3.3 V and the ESP32 I²C lines are 3.3 V logic).

## Sanity check if it's not reading
- The firmware runs an I²C bus scan at boot — watch `/api/logs` on the web
  dashboard. A healthy sensor shows up at 0x76 with chip ID 0x60.
- First failure last time was simply the **SCL wire unplugged** — reseat both
  JST pins if the indoor temp reads blank.
