# SmartClockOS Wall-Mount Case v2 (ESP32-8048S070) — screwless

Two-part 3D-printable wall case for the Sunton 7" board. **No assembly
screws** — the back cover hooks under a lip at the top edge and locks with
two snap hooks at the bottom. The only screws anywhere are the two in your
wall.

**v2** is a full rework after the first test print:
- USB-C opening rebuilt from the manufacturer's interface drawing (the v1
  slot pair was ~10 mm off and a post landed on the port). Now one big
  funnel-mouthed opening with ±8 mm of slop — any cable fits.
- The four fragile corner pins are **gone** (they snapped). The walls
  locate the board now; nothing thin left to break in the shell.
- Lid: four skinny snap arms → **rigid top lip + 2 thick bottom hooks**
  (13 × 2.4 mm, was 9 × 1.5). You can't force it wrong: tilt, hook, click.
- Snap slots sit in double-thickness wall pads so the rims can't tear out.
- Board pocket clearance doubled (0.4 → 0.8 mm/side) — the display drops
  in with **zero force**.
- Stepped pocket shelves lock the board vertically (the job the old pins
  did), with keepouts for the TF slot and the display's flex cables. When
  wall-hung, the board rests on solid plastic, not friction.
- Walls 2.8 → 3.2 mm, lid 2.2 → 2.8 mm, everything printed at 4 walls.

## Files
- `SmartClock_Shell.stl` — front bezel + walls + standoff pads + sensor chin
- `SmartClock_Lid.stl` — back plate, lip tongue + snap hooks (pre-flipped, print as-is)
- `SmartClockCase.scad` — parametric source (OpenSCAD)
- `AssemblyDiagram.html` — open in a browser: exploded view, wire route, wall mounting
- `Renders.html` — gallery of v2 renders (front/back/interiors/cutaway/USB funnel)

## Print settings (PLA)
- Nozzle 205–215 °C, bed 60 °C (what you used on v1 is fine)
- 0.2 mm layers, **4 perimeters** (important — hooks/lip/towers become
  solid at 4 walls), 15–25 % infill, **no supports**
- Both files are already in print orientation (shell face-down, lid
  outer-face-down)
- Footprint 189 × 145 mm — fits a 220 × 220 bed
- **Re-slice fresh.** These files replace the v1 STLs — don't reuse old
  gcode.

## Hardware
- 2× wall screws (#6 / 3.5 mm, head ≤ 7.5 mm) — **100 mm apart, level**,
  left ~5 mm proud of the wall (~8 mm if you install a fan later)
- 2–3 mm foam / EVA tape strips for the bezel lip (this provides the
  clamping preload — don't skip it)

## How it holds together (no screws)
- The board sits in a pocket with 0.8 mm of play on **all four sides**
  (side walls left/right, chamfered shelf steps top/bottom) and rests on
  four flat corner pads. No pins.
- The lid's four **solid towers** land on the PCB corners and clamp the
  board/glass sandwich against the foam on the bezel lip.
- The lid's **top edge slides under two rigid lips** in the shell —
  nothing flexes there, so nothing can break there.
- The lid's **bottom edge locks with two wide snap hooks** into slots in
  reinforced wall pads, plus friction nubs steady the sides.
- **To open**: push the two hooks inward through the bottom-wall slots
  (fingernail or small flathead, they're ~4 mm deep), swing the bottom of
  the cover out, then unhook the top edge. Never pry the middle.

## Assembly (see AssemblyDiagram.html for pictures)
1. Stick foam strips on the inside of the bezel lip (around the window).
2. Lay the shell face-down and drop the display in, glass first. It must
   fall in freely — if it doesn't, a wire is trapped under it.
3. Plug in the JST wires (BME280: SDA→P2 `IO11`, SCL→P2 `IO12`, power
   from P4). Bend the wires down over the top edge — there's 10 mm of
   headroom for the bend. The engraved **USB** mark on the cavity floor
   shows which edge the ports face.
4. Route the BME280 wires down the back cavity and through the Ø7 mm hole
   into the sensor chin; seat the sensor in the chin pocket (slides in
   from the back; the cover closes it).
5. USB-C power cable through the left funnel; route the cable down through
   the bottom-left notch. A right-angle USB-C plug looks cleanest.
6. **Cover on: tilt ~15°, top edge first.** Slide the top edge under the
   lips, swing the bottom down, press until you hear **two clicks**.
7. Hang on the two wall screws — keyholes grab, case slides down to lock.

## Tuning (SmartClockCase.scad)
- `GLASS_STACK` (6.6) — front-of-glass to front-of-PCB; adjust ±0.5 if
  the display sits proud or loose, re-export the shell.
- `HOOK` (0.8) — snap retention; 0.6 if closing feels too stiff, 1.0 if
  the bottom edge rattles.
- `USBC_Y` (54.0) — USB-C center, measured from the board's top edge
  (from the manufacturer drawing — don't eyeball).
- `KEY_SPACING` (100) — wall screw spacing.
- Export: `openscad -o shell.stl --render -D 'PART="shell"' SmartClockCase.scad`
  and `-D 'PART="lid_print"'` for the lid.

## Design notes
- Vents top + bottom create a convection path; WROOM antenna sees only
  plastic, WiFi unaffected.
- The sensor chin connects to the interior only via the wire hole and
  samples room air through slits on its face and underside — display
  heat exits the top vents on the opposite side.
- Fan bay for a 30×30×10 mm 5 V fan is on the lid (see `FAN.md`) — the
  case works fine with or without one installed.
- TF card and BOOT/RESET are enclosed — flash over WiFi
  (open the clock's address in a browser; it is shown in the clock's
  settings).
