# Fan wiring — 30mm 5V fan (0.1 A) into SmartClockOS

The firmware (v2.4.0+) drives the fan with 25 kHz PWM on **IO17**, using the
ESP32-S3's internal die-temperature sensor as the thermostat:

- OFF below 55 °C die temp (hysteresis: once running, stops at 48 °C)
- 30 % → 100 % duty as die temp climbs 48 → 70 °C
- **Silent during night mode** unless the chip somehow exceeds 75 °C
- Live status on the web dashboard: "CPU / Fan" tile (die °C / duty %)

No fan connected? Nothing breaks — IO17 just wiggles harmlessly.

## Parts
- One of your 30×30×10 mm 5 V fans (XH2.54 2-pin plug)
- A logic-level N-channel MOSFET or NPN transistor. Easiest zero-solder
  option: a "MOSFET trigger switch module" (AO3400/D4184 boards with screw
  terminals, ~$1). Classic parts: AO3400, IRLZ34N, or S8050 NPN + 1 kΩ
  base resistor.
- A 1N4148 or 1N5819 diode across the fan (+ to fan −, band toward fan +)
  — skip only if your MOSFET module already has one.

## Connections (all on JST connectors, no soldering to the display board)

```
P1 "power" connector          MOSFET / module            Fan
─────────────────────         ───────────────            ───────────
5V  ──────────────────────────────────────────────────►  RED (fan +)
GND ─────────────►  Source / GND
                    Drain / OUT  ─────────────────────►  BLACK (fan −)
P5 connector
IO17 ─────────────►  Gate / SIG (through 100 Ω if bare MOSFET)
GND on module and P5 must be common (they are — same board GND).
```

- **5 V + GND**: from the 4-pin `P1` power connector (labeled on silk;
  5 V is the VIN pin). The fan draws 0.1 A — trivial next to the board's
  0.5 A budget on a 2 A USB supply.
- **Gate/signal**: `IO17` — it's broken out on the P3 and P5 JST
  connectors (silk-labeled). P5 also gives you the GND pin right there.
- Low-side switching: the fan's black wire goes to the MOSFET, not
  directly to GND.

## Mounting (in the fan-ready case, v2 STLs)
1. Press the fan onto the four pins on the lid's inner face, frame into
   the shallow pocket. Label/sticker side toward the lid (air must blow
   AWAY from the lid, at the board).
2. Route its cable along the lid toward the top headroom band with the
   other wires.
3. The grille behind the fan breathes through the wall gap created by the
   four standoff feet on the shell rim — so with the fan build, wall
   screws should be left ~8 mm proud instead of 4–5 mm.

## Tuning (Config.h)
`FAN_ON_C / FAN_OFF_C / FAN_MAX_C / FAN_MIN_DUTY / FAN_NIGHT_OVERRIDE_C`
— defaults are conservative. Watch the "CPU / Fan" tile for a day and
tighten to taste. Set `ENABLE_FAN 0` to remove the feature entirely.
