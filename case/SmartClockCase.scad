// ============================================================
// SmartClockOS wall-mount case v2 — Sunton ESP32-8048S070 (7" 800x480)
//
// Two printed parts, no assembly screws:
//   SHELL: front bezel + walls + PCB standoffs + sensor pod chin
//   LID:   back plate; top edge hooks under a rigid lip in the shell,
//          bottom edge locks with TWO snap hooks (the only flex parts).
//
// v2 changes after the first test print:
//   - USB-C opening rebuilt from the manufacturer drawing (port center
//     54 mm from board top edge; the old slot pair was misplaced and a
//     post landed on the port). One funnel mouth, ±8 mm of slop.
//   - Corner alignment pins DELETED (they sheared) — cavity walls locate
//     the board, solid towers clamp it. Nothing thin left in the shell.
//   - Lid: 4 fragile snap arms -> rigid top lip + 2 thick bottom hooks
//     (13 wide x 2.4 thick, was 9 x 1.5). Snap windows sit in wall pads
//     double the old thickness so the rims can't tear out.
//   - Board pocket clearance 0.4 -> 0.8 mm/side: drops in with no force.
//   - Walls 2.8 -> 3.2, lid plate 2.2 -> 2.8. Slice with 4 perimeters.
//
// Board: 181 x 108 mm PCB, corner holes D3.2 at 3.5/3.5 from edges,
//        display glass on front; USB-C + 4P power JST on LEFT edge
//        (front view), JST IO row + TF on TOP edge.
// ============================================================

PART = "assembly"; // "assembly" | "shell" | "lid" | "lid_print" | "section"
                   // | "exploded" | "wire_route" | "wall_hang"

$fn = 64;
EPS = 0.01;

// ---------- board ----------
PCB_W = 181.0;
PCB_H = 108.0;
PCB_T = 1.6;
HOLE_IN = 3.5;      // hole center from PCB edges
HOLE_D = 3.2;
GLASS_STACK = 6.6;  // front of glass -> front of PCB (add foam on lip)
COMP_DEPTH = 14.0;  // clearance behind PCB (fits 30x30x10 fan on lid)

// left-edge connector truth (board y measured from TOP edge, front view;
// from the Jingcai interface drawing — do not eyeball these again)
USBC_Y = 54.0;      // USB-C center
JST_Y  = 38.0;      // 4P 1.25 power JST center (stays inside the case)

// ---------- case ----------
WALL = 3.2;         // side walls
FACE = 2.4;         // front bezel plate
LID_T = 2.8;        // back lid plate
LID_RABBET = 3.0;   // wall step depth that receives the lid
CLR = 0.8;          // PCB side clearance (v2: board must drop in freely)
TOP_HEAD = 10.0;    // headroom above PCB top edge for JST wire bends
BOT_GAP = 6.0;      // below PCB bottom edge (snap hardware lives here)

IN_W = PCB_W + 2*CLR;                 // 182.6
IN_H = TOP_HEAD + PCB_H + BOT_GAP;    // 124.0
OUT_W = IN_W + 2*WALL;                // 189.0
OUT_H = IN_H + 2*WALL;                // 130.4
DEPTH = FACE + GLASS_STACK + PCB_T + COMP_DEPTH + LID_RABBET; // 27.6
R_OUT = 6;          // outer corner radius

// window (bezel opening) — lip overlaps the touch glass; active area is
// 153.84 x 85.63 so this leaves ~3.5mm visual margin around it.
WIN_W = 161;
WIN_H = 93;
R_WIN = 3;
WIN_CHAMFER = 1.4;  // decorative stepped reveal on the front face

// PCB placement in case coords (x right, y DOWN from case top-left, z from front face)
PCB_X0 = WALL + CLR;
PCB_Y0 = WALL + TOP_HEAD;
PCB_ZF = FACE + GLASS_STACK;          // front face of PCB
PCB_CX = PCB_X0 + PCB_W/2;
PCB_CY = PCB_Y0 + PCB_H/2;

// component zone behind PCB (z range)
ZC0 = PCB_ZF + PCB_T;                 // 10.6
ZC1 = ZC0 + COMP_DEPTH;               // 24.6
Z_LID = DEPTH - LID_RABBET + 0.1;     // lid plate front face

// ---------- USB-C funnel (left wall) ----------
PORT_CY = PCB_Y0 + USBC_Y;            // 67.2
THROAT_W = 18;  THROAT_Z0 = ZC0-2.4;  THROAT_Z1 = ZC0+6.2;
MOUTH_W  = 26;  MOUTH_Z0  = ZC0-4.4;  MOUTH_Z1  = ZC0+8.2;

// bottom cable notch (route cable down behind wall)
NOTCH_X0 = 12; NOTCH_W = 12;          // from case left inner

// sensor pod ("chin" on bottom wall, right of center; full depth so it
// prints support-free face-down and samples room air away from case heat)
POD_W = 30;  POD_DROP = 14;  POD_WALL = 2.0;
POD_CX = OUT_W/2 + 34;
POD_HOLE_D = 7;

// keyholes (in lid, near top, hang-down slots)
KEY_SPACING = 100;
KEY_Y = 12;          // circle center, case coords from top
KEY_BOSS_D = 18; KEY_BOSS_H = 5.0;
KEY_HOLE_D = 8.2; KEY_SLOT_W = 4.4; KEY_SLOT_LEN = 7;
KEY_POCKET_D = 10.0; KEY_POCKET_KEEP = 4.0;  // material left in front of head

// PCB standoffs (flat pads under the corner holes — v2 has NO pins;
// the walls locate the board, the lid towers clamp it down)
BOSS_D = 8.0;

// stepped pocket shelves: at board level (z < ZC0) the pocket is only
// board-sized; the extra top/bottom room exists only BEHIND the board
// (z > ZC0) where the wires and snap hardware live. This locks the board
// vertically to ±CLR without pins — solid blocks, nothing to break.
// Keepouts: TF card slot (top), LCD flex fold + touch tail/edge notch
// (bottom) — zones where something wraps over the board edge.
SHELF_TOP_XS = [[2.8, 40], [60, OUT_W-2.8]];      // gap = TF slot
SHELF_BOT_XS = [[2.8, 75], [118, 143]];           // gaps = flex, tail/notch
SHELF_CH = 1.0;                                    // drop-in lead chamfer

// pressure towers on lid: SOLID columns (v1 tubes survived; these are
// 3x stiffer). Diameter capped by the lid plate edge at the corners.
TOWER_D = 8.5;

// top-edge lid retention: rigid lip in the shell, chamfered tongue on
// the lid. Tilt the lid, slide the top edge under the lip, lay it flat.
// Both faces are parallel 45° planes: lifting the lid 0.45mm brings the
// whole 100mm of lip into face contact — no point loads, nothing thin.
LIP_XS = [[30, 80], [OUT_W-80, OUT_W-30]];   // two segments
Y_EDGE = WALL - 0.25 + 0.1;           // lid plate top edge (3.05)
TON_TIP = 1.2;                        // tongue tip thickness at the edge
TON_CLR = 0.45;                       // lift clearance under the lip
// tongue top plane: z = Z_LID + TON_TIP + (y - Y_EDGE), 45° rising
// lip underside:    same slope, TON_CLR above it

// bottom snap hooks (the only flexible parts left)
TAB_W = 13.0; TAB_T = 2.4;            // arm width / thickness
HOOK = 0.8;                           // hook protrusion
PAD_T = 1.8;                          // wall reinforcement pad thickness
TAB_BOT_XS = [98, 160];               // clear of pod / notch / vents
ARM_LEN = 13.0;
ARM_Z_END = Z_LID - ARM_LEN;          // 11.7
HOOK_Z0 = ARM_Z_END + 0.3;            // ramp starts (flush)
HOOK_Z1 = HOOK_Z0 + 4.0;              // flat lock face (square top)
WIN_Z0 = ARM_Z_END - 0.9;             // window in wall+pad; extra room
WIN_Z1 = HOOK_Z1 + 0.15;              // below the hook = release access

// 30mm fan bay (snap-in, on lid inner face, over the WROOM/CPU zone)
FAN_CX = 160; FAN_CY = 72;
FAN_SIZE = 30.8; FAN_RECESS = 1.6;    // frame pocket
FAN_PIN_SP = 24; FAN_PIN_D = 3.2; FAN_PIN_H = 8;

// wall standoff feet on the shell rim (air gap feeds the lid intake grille)
FOOT_D = 8; FOOT_H = 3.5;

// vents
VENT_L = 9; VENT_W = 1.8; VENT_PITCH = 14;

// ------------------------------------------------------------
module rrect(w, h, r) { offset(r) offset(-r) square([w, h]); }

module case_outline() { rrect(OUT_W, OUT_H, R_OUT); }
module cavity_outline() { translate([WALL, WALL]) rrect(IN_W, IN_H, R_OUT-WALL); }

function pcb_hole(i) =
    [PCB_X0 + (i%2==0 ? HOLE_IN : PCB_W-HOLE_IN),
     PCB_Y0 + (i<2   ? HOLE_IN : PCB_H-HOLE_IN)];

// ============================================================
// SHELL
// ============================================================
module shell() {
    difference() {
        union() {
            // body: face + walls
            linear_extrude(DEPTH) case_outline();
            // sensor pod chin: full depth, fused to the bottom wall
            translate([POD_CX-POD_W/2, OUT_H-1, 0])
                linear_extrude(DEPTH) rrect(POD_W, POD_DROP+1, 2.5);
        }

        // hollow cavity (leaves face plate)
        translate([0,0,FACE]) linear_extrude(DEPTH) cavity_outline();

        // lid rabbet: widen cavity near the back so lid drops in
        translate([0,0,DEPTH-LID_RABBET])
            linear_extrude(LID_RABBET+EPS)
            offset(0.25) cavity_outline();

        // bezel window + chamfered reveal step
        translate([PCB_CX-WIN_W/2, PCB_CY-WIN_H/2, -EPS])
            linear_extrude(FACE+2*EPS) rrect(WIN_W, WIN_H, R_WIN);
        translate([PCB_CX-(WIN_W+2*WIN_CHAMFER)/2, PCB_CY-(WIN_H+2*WIN_CHAMFER)/2, -EPS])
            linear_extrude(1.0) rrect(WIN_W+2*WIN_CHAMFER, WIN_H+2*WIN_CHAMFER, R_WIN+WIN_CHAMFER);

        // ---- USB-C funnel: chamfered mouth outside -> straight throat.
        // Both bigger than any cable overmold, so the plug passes fully
        // through the wall and seats regardless of wall thickness.
        hull() {
            translate([-EPS, PORT_CY-MOUTH_W/2, MOUTH_Z0])
                cube([0.1, MOUTH_W, MOUTH_Z1-MOUTH_Z0]);
            translate([WALL*0.55, PORT_CY-THROAT_W/2, THROAT_Z0])
                cube([0.1, THROAT_W, THROAT_Z1-THROAT_Z0]);
        }
        translate([WALL*0.5, PORT_CY-THROAT_W/2, THROAT_Z0])
            cube([WALL*0.5+CLR+1, THROAT_W, THROAT_Z1-THROAT_Z0]);

        // bottom cable notch (near left)
        translate([WALL+NOTCH_X0, OUT_H-WALL-PAD_T-EPS, ZC0])
            cube([NOTCH_W, WALL+PAD_T+2*EPS, ZC1-ZC0+LID_RABBET+EPS]);

        // top vents
        for (i=[0:8])
            translate([OUT_W/2 + (i-4)*VENT_PITCH - VENT_L/2, -EPS, ZC0+1])
                cube([VENT_L, WALL+2*EPS, VENT_W*2]);
        // bottom vents (left of the left snap pad, clear of notch)
        for (i=[0:3])
            translate([WALL+28 + i*VENT_PITCH, OUT_H-WALL-EPS, ZC0+1])
                cube([VENT_L, WALL+2*EPS, VENT_W*2]);

        // pod cavity: open toward the back (lid closes it when mounted)
        translate([POD_CX-POD_W/2+POD_WALL, OUT_H-1-EPS, POD_WALL+1.5])
            cube([POD_W-2*POD_WALL, POD_DROP-POD_WALL+1+EPS, DEPTH-POD_WALL-1.5+EPS]);
        // wire hole through the case bottom wall into the pod
        translate([POD_CX, OUT_H-WALL/2, (ZC0+ZC1)/2])
            rotate([90,0,0]) cylinder(d=POD_HOLE_D, h=WALL*3, center=true);
        // pod bottom slits (room-air intake)
        for (i=[-1:1])
            translate([POD_CX + i*8 - VENT_W/2, OUT_H+POD_DROP-POD_WALL-EPS, POD_WALL+3])
                cube([VENT_W, POD_WALL+2*EPS, DEPTH-2*POD_WALL-6]);
        // pod front slits (visible "thermometer grille" on the chin face)
        for (i=[-1:1])
            translate([POD_CX + i*8 - VENT_W/2, OUT_H+2, -EPS])
                cube([VENT_W, POD_DROP-4, POD_WALL+1.5+2*EPS]);

        // snap windows: through the bottom wall AND its reinforcement pad
        for (x = TAB_BOT_XS)
            translate([x-TAB_W/2-0.5, OUT_H-WALL-PAD_T-2*EPS, WIN_Z0])
                cube([TAB_W+1, WALL+PAD_T+4*EPS, WIN_Z1-WIN_Z0]);

        // orientation engravings on the cavity floor (read from the back:
        // mirrored). 0.5 deep into the inner face.
        translate([0,0,FACE-0.5]) linear_extrude(0.5+EPS) {
            translate([9.5, PORT_CY]) rotate(90) mirror([1,0])
                text("USB", size=5.5, halign="center", valign="center", font="Arial:style=Bold");
            translate([OUT_W/2, 16.6]) mirror([1,0])
                text("TOP", size=5.5, halign="center", valign="center", font="Arial:style=Bold");
        }
    }

    // reinforcement pads around the snap windows (inside the cavity,
    // fused to the bottom wall; the window rims can't tear out now)
    for (x = TAB_BOT_XS)
        difference() {
            translate([x-TAB_W/2-5, OUT_H-WALL-PAD_T, ZC0])
                cube([TAB_W+10, PAD_T+EPS, DEPTH-LID_RABBET-0.4-ZC0]);
            translate([x-TAB_W/2-0.5, OUT_H-WALL-PAD_T-2*EPS, WIN_Z0])
                cube([TAB_W+1, WALL+PAD_T+4*EPS, WIN_Z1-WIN_Z0]);
        }

    // top-edge retention lip: a 45° wedge under the back rim overhanging
    // the rabbet. Underside plane sits TON_CLR above the lid tongue's
    // chamfer plane; prints face-down without supports (45° underside).
    // Profile (y,z): root buried 0.6 into the wall, tip where the
    // underside plane meets the back face.
    for (seg = LIP_XS)
        translate([seg[0], 0, 0])
            rotate([90,0,90])
            linear_extrude(seg[1]-seg[0])
            polygon([[WALL-0.85, Z_LID+TON_TIP+TON_CLR + (WALL-0.85-Y_EDGE)],
                     [Y_EDGE + (DEPTH-(Z_LID+TON_TIP+TON_CLR)), DEPTH],
                     [WALL-0.85, DEPTH]]);

    // wall standoff feet on the back rim (intake air gap when hung)
    for (fx=[6, OUT_W-6], fy=[6, OUT_H-6])
        translate([fx, fy, DEPTH-EPS]) cylinder(d=FOOT_D, h=FOOT_H+EPS);

    // PCB standoff pads (flat tops — no pins, nothing to shear)
    for (i=[0:3]) {
        p = pcb_hole(i);
        translate([p[0], p[1], FACE-EPS]) cylinder(d=BOSS_D, h=GLASS_STACK+EPS);
    }

    // pocket shelves (see params): solid blocks fused to face + walls,
    // topped at the PCB back plane with a 45° drop-in lead chamfer
    for (seg = SHELF_TOP_XS)
        translate([seg[0], 0, 0]) rotate([90,0,90])
            linear_extrude(seg[1]-seg[0])
            polygon([[2.6, FACE-0.1], [2.6, ZC0],
                     [PCB_Y0-CLR-SHELF_CH, ZC0],
                     [PCB_Y0-CLR, ZC0-SHELF_CH],
                     [PCB_Y0-CLR, FACE-0.1]]);
    for (seg = SHELF_BOT_XS)
        translate([seg[0], 0, 0]) rotate([90,0,90])
            linear_extrude(seg[1]-seg[0])
            polygon([[OUT_H-2.6, FACE-0.1], [OUT_H-2.6, ZC0],
                     [PCB_Y0+PCB_H+CLR+SHELF_CH, ZC0],
                     [PCB_Y0+PCB_H+CLR, ZC0-SHELF_CH],
                     [PCB_Y0+PCB_H+CLR, FACE-0.1]]);
}

// ============================================================
// LID (lip + two snaps — no screws)
// ============================================================

// bottom-edge cantilever snap arm. Sits against the reinforcement pad
// face; hook protrudes outward into the padded window. Outer surface
// ramps from flush (bottom) to full hook, square lock face on top.
module snap_arm(x) {
    padFace = OUT_H - WALL - PAD_T;       // pad inner face
    yArm = padFace - 0.15 - TAB_T;        // arm inner face
    // arm: from lid plate down to ARM_Z_END
    translate([x-TAB_W/2, yArm, ARM_Z_END])
        cube([TAB_W, TAB_T, Z_LID - ARM_Z_END + LID_T - 0.2]);
    // hook wedge on the outer face
    hull() {
        translate([x-TAB_W/2, yArm+TAB_T-0.1, HOOK_Z0])
            cube([TAB_W, 0.1, HOOK_Z1-HOOK_Z0]);
        translate([x-TAB_W/2, yArm+TAB_T+HOOK-0.1, HOOK_Z1-1.4])
            cube([TAB_W, 0.1, 1.4]);
    }
}

module lid() {
    difference() {
        union() {
            // plate (fits rabbet with clearance)
            translate([0,0,Z_LID])
                linear_extrude(LID_T)
                offset(-0.1) cavity_outline();
            // keyhole bosses on inner face
            for (s=[-1,1])
                translate([OUT_W/2 + s*KEY_SPACING/2, KEY_Y+2, Z_LID-KEY_BOSS_H])
                    cylinder(d=KEY_BOSS_D, h=KEY_BOSS_H+EPS);
            // snap arms (bottom edge only)
            for (x = TAB_BOT_XS) snap_arm(x);
            // pressure towers: SOLID columns that clamp the PCB corners
            // onto the standoff pads (tips land 0.2 above the PCB back)
            for (i=[0:3]) {
                p = pcb_hole(i);
                translate([p[0], p[1], ZC0+0.2])
                    cylinder(d=TOWER_D, h=Z_LID-(ZC0+0.2)+EPS);
            }
            // fan bay: shallow frame pocket walls + 4 mounting pins
            translate([FAN_CX-FAN_SIZE/2-2, FAN_CY-FAN_SIZE/2-2, Z_LID-FAN_RECESS])
                difference() {
                    cube([FAN_SIZE+4, FAN_SIZE+4, FAN_RECESS]);
                    translate([2,2,-EPS]) cube([FAN_SIZE, FAN_SIZE, FAN_RECESS+2*EPS]);
                }
            for (sx=[-1,1], sy=[-1,1])
                translate([FAN_CX+sx*FAN_PIN_SP/2, FAN_CY+sy*FAN_PIN_SP/2, Z_LID-FAN_PIN_H]) {
                    cylinder(d1=FAN_PIN_D, d2=FAN_PIN_D-0.4, h=FAN_PIN_H);
                    cylinder(d1=FAN_PIN_D+2.6, d2=FAN_PIN_D, h=2.2); // root fillet
                }
            // friction nubs on left/right edges (steady the free sides)
            for (sx=[0,1], sy=[-1,1])
                translate([sx==0 ? WALL+0.05 : OUT_W-WALL-0.05,
                           OUT_H/2 + sy*34, Z_LID+LID_T/2])
                    scale([1,1.6,1.6]) sphere(d=1.2);
        }

        // top-edge tongue: 45° chamfer off the back-outer corner; slides
        // under the shell lip when the lid goes in top-edge-first. (In
        // the print orientation this is just a chamfer — no supports.)
        // Removes everything above the plane z = Z_LID+TON_TIP+(y-Y_EDGE).
        translate([-EPS,0,0]) rotate([90,0,90])
            linear_extrude(OUT_W+2*EPS)
            polygon([[Y_EDGE-2, Z_LID+TON_TIP-2],
                     [Y_EDGE+6, Z_LID+TON_TIP+6],
                     [Y_EDGE-2, Z_LID+TON_TIP+6]]);

        // notches in the plate edge around the snap pads
        for (x = TAB_BOT_XS)
            translate([x-TAB_W/2-5.3, OUT_H-WALL-PAD_T-0.2, Z_LID-EPS])
                cube([TAB_W+10.6, WALL+PAD_T+0.2, LID_T+2*EPS]);

        // keyhole cuts
        for (s=[-1,1]) {
            cx = OUT_W/2 + s*KEY_SPACING/2;
            // through-cut: circle + upward slot
            translate([cx, KEY_Y+KEY_SLOT_LEN, 0]) {
                translate([0,0,Z_LID-KEY_BOSS_H-1])
                    cylinder(d=KEY_HOLE_D, h=KEY_BOSS_H+LID_T+3);
                translate([-KEY_SLOT_W/2, -KEY_SLOT_LEN-2, Z_LID-KEY_BOSS_H-1])
                    cube([KEY_SLOT_W, KEY_SLOT_LEN+2, KEY_BOSS_H+LID_T+3]);
            }
            // head pocket from inner side (leave KEEP mm at outer face)
            translate([cx, KEY_Y+KEY_SLOT_LEN, 0]) {
                translate([0,0,Z_LID-KEY_BOSS_H-EPS])
                    cylinder(d=KEY_POCKET_D, h=KEY_BOSS_H+LID_T-KEY_POCKET_KEEP);
                translate([-KEY_POCKET_D/2, -KEY_SLOT_LEN-2, Z_LID-KEY_BOSS_H-EPS])
                    cube([KEY_POCKET_D, KEY_SLOT_LEN+2, KEY_BOSS_H+LID_T-KEY_POCKET_KEEP]);
            }
        }

        // fan intake grille through the lid (fed by the wall-gap standoff)
        for (ring=[[7,8],[11,14]])
            for (k=[0:ring[1]-1])
                translate([FAN_CX+ring[0]*cos(360*k/ring[1]),
                           FAN_CY+ring[0]*sin(360*k/ring[1]), Z_LID-1])
                    cylinder(d=3.4, h=LID_T+3);
        // small passive slit column on the left half (cross-flow)
        for (j=[0:2])
            translate([52, 40 + j*22, Z_LID-1])
                cube([VENT_W*1.6, 14, LID_T+3]);

        // version mark on the inner face
        translate([FAN_CX-28, FAN_CY+22, Z_LID-EPS]) linear_extrude(0.4+EPS)
            text("v2", size=6, font="Arial:style=Bold");
    }
}

// ============================================================
// mock board for assembly preview (true connector positions)
// ============================================================
module mock_board() {
    color("gold") translate([PCB_X0, PCB_Y0, PCB_ZF]) cube([PCB_W, PCB_H, PCB_T]);
    color([0.1,0.1,0.12]) translate([PCB_CX-165/2, PCB_CY-100/2, PCB_ZF-GLASS_STACK+1.5]) cube([165, 100, GLASS_STACK-1.5]);
    color([0.2,0.25,0.3]) translate([PCB_CX-153.84/2, PCB_CY-85.63/2, PCB_ZF-GLASS_STACK]) cube([153.84, 85.63, 1]);
    // JST IO row on back near top edge
    color("white") translate([PCB_X0+77, PCB_Y0+1, ZC0]) cube([58, 6, 6]);
    // USB-C on left edge (center 54 from top) + 4P power JST (center 38)
    color("silver") translate([PCB_X0-1.3, PCB_Y0+USBC_Y-4.6, ZC0]) cube([8, 9.2, 3.4]);
    color("white")  translate([PCB_X0+0.5, PCB_Y0+JST_Y-3.5,  ZC0]) cube([5, 7, 5.5]);
}

// ============================================================
if (PART == "shell") shell();
else if (PART == "lid_print") lid_print();
else if (PART == "exploded") exploded();
else if (PART == "wire_route") wire_route();
else if (PART == "wall_hang") wall_hang();
else if (PART == "lid") lid();
else if (PART == "section") {
    color([0.35,0.4,0.48]) difference() { shell();
        translate([65, -10, -10]) cube([OUT_W, OUT_H+150, DEPTH+60]); }
    color([0.3,0.7,0.4]) difference() { lid();
        translate([65, -10, -10]) cube([OUT_W, OUT_H+150, DEPTH+60]); }
    %mock_board();
}
else if (PART == "section_snap") {
    // cut through a bottom snap arm, looking at the hook/window fit
    color([0.35,0.4,0.48]) difference() { shell();
        translate([-10, -10, -10]) cube([TAB_BOT_XS[0]+10, OUT_H+40, DEPTH+60]); }
    color([0.3,0.7,0.4]) difference() { lid();
        translate([-10, -10, -10]) cube([TAB_BOT_XS[0]+10, OUT_H+40, DEPTH+60]); }
    %mock_board();
}
else if (PART == "xfit") {
    // collision check: must export an EMPTY stl (lid never touches shell)
    intersection() { shell(); lid(); }
}
else if (PART == "xboard") {
    // collision check: case parts vs the board+glass volume (solid mock,
    // slightly generous). Bosses stop AT the pcb front plane and towers
    // 0.2 above the back plane, so this must also be EMPTY.
    intersection() {
        union() { shell(); lid(); }
        union() {
            translate([PCB_X0, PCB_Y0, PCB_ZF]) cube([PCB_W, PCB_H, PCB_T]);
            translate([PCB_CX-165/2, PCB_CY-100/2, FACE+0.05])
                cube([165, 100, GLASS_STACK-0.05]);
        }
    }
}
else { // assembly
    color([0.35,0.4,0.48]) shell();
    color([0.3,0.55,0.45]) lid();
    %mock_board();
}

// print-oriented lid export (outer face on bed, towers up)
module lid_print() { translate([0, 0, DEPTH - 0.1]) rotate([180, 0, 0]) translate([0, -OUT_H, 0]) mirror([0,1,0]) lid(); }

// ============================================================
// ASSEMBLY DIAGRAM MODES (exploded / wire_route / wall_hang)
// ============================================================

module arrow_to(from, to, d=4, col="red") {
    v = [to[0]-from[0], to[1]-from[1], to[2]-from[2]];
    L = norm(v);
    head = min(2.6*d, L*0.45);
    color(col)
    translate(from)
    rotate([0, acos(v[2]/L), atan2(v[1], v[0])]) {
        cylinder(d=d, h=L-head);
        translate([0,0,L-head]) cylinder(d1=d*2.2, d2=0.2, h=head);
    }
}

module mock_sensor() {
    color([0.55,0.3,0.75]) cube([13, 17, 2]);       // purple GY-BME280 pcb
    color("silver") translate([2.5, 3, 2]) cube([8, 8, 1.4]); // sensor can
}

module wall_mock() {
    color([0.82,0.8,0.76]) translate([-40, -35, 0]) cube([OUT_W+95, OUT_H+80, 5]);
    // two mounting screws at keyhole circle positions, heads proud
    for (s=[-1,1]) color([0.25,0.25,0.28])
        translate([OUT_W/2 + s*KEY_SPACING/2, KEY_Y+KEY_SLOT_LEN, -4.6]) {
            cylinder(d=3.4, h=5);
            translate([0,0,-2.6]) cylinder(d=7.6, h=2.6);
        }
}

// green wire path: top JST -> over edge -> down the back -> pod hole -> sensor
WPATH = [[95,16,15],[95,8,16],[99,10.5,19],[105,29,20],[116,61,20],
         [125,97,19.5],[128.5,119,17],[128.5,125,15.6],[128.5,132,13],[128.5,136,11]];
module wirepath() {
    color([0.1,0.75,0.3])
    for (i=[0:len(WPATH)-2])
        hull() { translate(WPATH[i]) sphere(d=2.6); translate(WPATH[i+1]) sphere(d=2.6); }
}

module exploded() {
    EB = -52;   // board offset (in front)
    EL = 56;    // lid offset (behind)
    EW = 145;   // wall offset

    color([0.35,0.4,0.48]) shell();
    translate([0,0,EB]) mock_board();
    // lid shown tilted the way it actually goes in: top edge leads
    color([0.3,0.55,0.45])
        translate([0, OUT_H*0.12, EL]) rotate([-12,0,0]) translate([0,-OUT_H*0.12,0]) lid();
    translate([0,0,DEPTH+EW]) wall_mock();

    // sensor below the chin, about to slide up/in
    translate([POD_CX-6.5, OUT_H+40, 6]) mock_sensor();

    // RED: display drops into the shell (onto the corner pads)
    arrow_to([-20, 64.8, EB+14], [-20, 64.8, 3], 4.5, "red");
    arrow_to([OUT_W+20, 100, EB+14], [OUT_W+20, 100, 3], 4.5, "red");
    // ORANGE: lid TOP edge hooks under the lip first...
    arrow_to([OUT_W/2, -30, EL+4], [OUT_W/2, -5, 25], 5.5, "orange");
    // ...then the BOTTOM edge swings down and clicks
    arrow_to([-24, OUT_H+18, EL+30], [-24, OUT_H+2, 27], 4.5, "orange");
    arrow_to([OUT_W+24, OUT_H+18, EL+30], [OUT_W+24, OUT_H+2, 27], 4.5, "orange");
    // GREEN: sensor slides up into the chin pod
    arrow_to([POD_CX, OUT_H+37, 9], [POD_CX, OUT_H+11, 9], 3.6, [0.1,0.75,0.3]);
    // BLUE: case onto wall screws, then slide down to lock
    arrow_to([OUT_W+42, 30, DEPTH+46], [OUT_W+42, 30, DEPTH+EW-3], 4.5, [0.2,0.45,0.95]);
    arrow_to([OUT_W+56, 36, DEPTH+EW-8], [OUT_W+56, 74, DEPTH+EW-8], 4.5, [0.2,0.45,0.95]);
}

module wire_route() {
    color([0.35,0.4,0.48]) shell();
    mock_board();
    wirepath();
    translate([POD_CX-6.5, OUT_H+3, 6]) mock_sensor();
    // green entry arrow at the JST end
    arrow_to([83, 31, 34], [93, 18, 18], 2.6, [0.1,0.75,0.3]);
}

module wall_hang() {
    // assembled case seen from the back, screws seated in the keyholes,
    // blue arrow = the case sliding DOWN to lock onto the screw shanks
    color([0.35,0.4,0.48]) shell();
    color([0.3,0.55,0.45]) lid();
    for (s=[-1,1]) color([0.2,0.2,0.24])
        translate([OUT_W/2 + s*KEY_SPACING/2, KEY_Y+KEY_SLOT_LEN, DEPTH-KEY_POCKET_KEEP-2]) {
            cylinder(d=3.4, h=KEY_POCKET_KEEP+6);
            translate([0,0,-2.4]) cylinder(d=7.4, h=2.4);
        }
    arrow_to([OUT_W/2, 22, DEPTH+9], [OUT_W/2, 86, DEPTH+9], 5, [0.2,0.45,0.95]);
}
