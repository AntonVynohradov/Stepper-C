#******************************************************************************
# Stepper-C: Stepper Motor Control Implementation
#
# Description: Generates the SVG diagrams of the documentation
#              (docs/assets/images/*.svg). Run after changing a diagram:
#                  python tools/docs/diagrams.py
#
# Author:      Anton Vynohradov
# Email:       avynohradov@systemfromscratch.com
# License:     Apache License 2.0
# Copyright (c) 2026 Anton Vynohradov
#
# SPDX-License-Identifier: Apache-2.0
#*****************************************************************************

"""Documentation diagrams of Stepper-C, built with svgkit (sfs-protocol-library skill).
Preview every diagram (headless Chrome --screenshot) and check its labels against the
code after a change. Curves (trajectory, voltage regions) are computed from the same
equations as the library."""

from __future__ import annotations

import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from svgkit import (BLUE, GREEN, GREY, HERO_DEFS, HERO_STYLE, LIGHT, LINE, NAVY,  # noqa: E402
                    ORANGE, RED, TEAL, WHITE, Iso, Svg)

OUT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..",
                                    "docs", "assets", "images"))

SOFT_TEAL = "#E6F4F5"
SOFT_ORANGE = "#FFF7EC"
SOFT_RED = "#FDECEC"
SOFT_GREEN = "#EAF5EA"


def box(s: Svg, x, y, w, h, title, lines=(), fill=LIGHT, stroke=LINE, color=NAVY,
        size=15, mono_lines=False) -> None:
    """A rounded box with a bold title and smaller lines below it."""
    s.rect(x, y, w, h, fill=fill, stroke=stroke, rx=10)
    top = y + h / 2 - (len(lines) * 17) / 2 + (5 if not lines else -2)
    s.text(x + w / 2, top, title, size=size, weight="bold", fill=color)
    for i, value in enumerate(lines):
        s.text(x + w / 2, top + 20 + i * 17, value, size=12.5, fill=GREY, mono=mono_lines)


def arrow(s: Svg, x1, y1, x2, y2, color=GREY, label="", dy=-8, size=12, dash=False) -> None:
    kind = {TEAL: "teal", ORANGE: "orange", GREEN: "green"}.get(color, "end")
    s.line(x1, y1, x2, y2, color=color, arrow=kind, dash=dash)
    if label:
        s.text((x1 + x2) / 2, (y1 + y2) / 2 + dy, label, size=size, fill=color)


# ==============================================================================
# MODULES
# ==============================================================================

def architecture() -> None:
    s = Svg(960, 560, "Stepper-C modules and the application around them")
    # application contexts
    box(s, 40, 30, 400, 90, "Slow-loop context (main loop or task)",
        ["sstep_slow_loop() at slow_loop_hz", "commands, sstep_get_status()"])
    box(s, 520, 30, 400, 90, "PWM/ADC interrupt",
        ["sstep_fast_loop(sample) at fast_loop_hz", "raw ADC counts: i_a, i_b, vbus"])
    arrow(s, 240, 120, 240, 172)
    arrow(s, 720, 120, 720, 172)
    # library
    s.rect(20, 172, 920, 250, fill=WHITE, stroke=BLUE, rx=14, sw=2)
    s.text(40, 200, "Stepper-C", size=17, weight="bold", fill=BLUE, anchor="start")
    s.text(140, 200, "public API: sstep.h · inc/sstep_ctrl.h · inc/sstep_defs.h · "
           "inc/sstep_config.h", size=12.5, fill=GREY, anchor="start")
    box(s, 40, 216, 580, 76, "Control — src/sstep_ctrl.c",
        ["configuration check · derived gains · state machine",
         "fast loop: current control, trips · slow loop: setpoints, events"],
        fill=SOFT_TEAL, stroke=TEAL)
    box(s, 640, 216, 280, 76, "Stall detection", ["src/sstep_stall.c",
                                                  "back-EMF observer, decision"],
        fill=SOFT_TEAL, stroke=TEAL)
    box(s, 40, 310, 280, 92, "Motion", ["src/sstep_motion.c", "trapezoidal profile,",
                                        "integer Q16 positions"])
    box(s, 340, 310, 280, 92, "Power stage", ["src/sstep_bridge.c", "voltage limit per topology,",
                                              "leg duty cycles"])
    box(s, 640, 310, 280, 92, "Numeric backend", ["src/sstep_math.*, sstep_sin_table.c",
                                                  "Q15/Q31 or float, saturating,",
                                                  "sine/cosine table"])
    s.text(480, 414, "private: everything under src/; struct sstep is opaque",
           size=12, fill=GREY)
    # board
    arrow(s, 480, 422, 480, 470, color=ORANGE)
    s.text(492, 452, "pwm_write(compare[]) · output_enable(on/off)", size=12.5,
           fill=ORANGE, anchor="start")
    box(s, 260, 470, 440, 70, "Board", ["PWM timer · gate drivers · ADC · shunts"],
        fill=SOFT_ORANGE, stroke=ORANGE)
    s.path("M700,505 L940,505 L940,75 L922,75", color=GREY, arrow="end", width=1.6)
    s.text(948, 300, "ADC counts", size=12, fill=GREY, rotate=90)
    s.save(OUT + "/architecture.svg")


# ==============================================================================
# THE TWO LOOPS
# ==============================================================================

def data_flow() -> None:
    s = Svg(980, 460, "Data flow of the slow loop and the fast loop")
    # lanes
    s.rect(20, 20, 940, 160, fill="#F7F9FC", stroke=LINE, rx=12)
    s.text(36, 44, "Slow loop · sstep_slow_loop() · 1 kHz", size=14, weight="bold",
           fill=BLUE, anchor="start")
    s.rect(20, 270, 940, 170, fill="#F7F9FC", stroke=LINE, rx=12)
    s.text(36, 294, "Fast loop · sstep_fast_loop() · 20 kHz", size=14, weight="bold",
           fill=TEAL, anchor="start")
    # slow chain
    y = 66
    box(s, 40, y, 170, 96, "Commands", ["move_to, move_by,", "run, stop"])
    box(s, 250, y, 200, 96, "Motion planner", ["position, velocity", "(Q16 microsteps)"])
    box(s, 490, y, 220, 96, "Setpoint", ["angle at period start,", "Δangle per fast period,",
                                         "I amplitude, feed-forward"])
    box(s, 750, y, 190, 96, "Bus voltage", ["V_base / Vbus", "from the last sample"])
    arrow(s, 210, 114, 250, 114)
    arrow(s, 450, 114, 490, 114)
    arrow(s, 750, 114, 710, 114)
    # double buffer
    s.rect(500, 196, 200, 56, fill=WHITE, stroke=NAVY, rx=8)
    s.line(600, 196, 600, 252, color=NAVY)
    s.text(550, 229, "sp[0]", size=13, mono=True)
    s.text(650, 229, "sp[1]", size=13, mono=True)
    s.text(712, 222, "double buffer,", size=12, fill=GREY, anchor="start")
    s.text(712, 238, "sp_pending flag", size=12, fill=GREY, anchor="start")
    arrow(s, 600, 162, 600, 196, color=BLUE)
    arrow(s, 600, 252, 600, 316, color=TEAL)
    # fast chain
    y = 316
    box(s, 34, y, 140, 104, "ADC counts", ["i_a, i_b, vbus", "→ pu, trips"])
    box(s, 196, y, 150, 104, "Angle", ["+ Δangle each", "period; sin, cos"])
    box(s, 368, y, 150, 104, "References", ["iA = I cos φ", "iB = I sin φ"])
    box(s, 540, y, 140, 104, "PI × 2", ["+ back-EMF", "feed-forward"])
    box(s, 702, y, 120, 104, "Limit", ["topology,", "anti-windup"])
    box(s, 844, y, 104, 104, "PWM", ["duty →", "pwm_write()"])
    for x1, x2 in ((174, 196), (346, 368), (518, 540), (680, 702), (822, 844)):
        arrow(s, x1, 368, x2, 368, color=TEAL)
    s.save(OUT + "/data-flow.svg")


# ==============================================================================
# DRIVER STATES
# ==============================================================================

def state_machine() -> None:
    s = Svg(920, 420, "Driver states and transitions")
    states = {
        "DISABLED": (60, 160, LIGHT, GREY),
        "HOLDING": (360, 60, SOFT_GREEN, GREEN),
        "MOVING": (660, 160, SOFT_TEAL, TEAL),
        "FAULT": (360, 290, SOFT_RED, RED),
    }
    w, h = 200, 70
    for name, (x, y, fill, color) in states.items():
        s.rect(x, y, w, h, fill=fill, stroke=color, rx=35, sw=2)
        s.text(x + w / 2, y + 32, "SSTEP_STATE_", size=11, fill=GREY, mono=True)
        s.text(x + w / 2, y + 52, name, size=16, weight="bold", fill=color, mono=True)
    # DISABLED -> HOLDING
    s.path("M200,160 C220,110 280,95 360,95", color=GREEN, arrow="green", width=1.8)
    s.text(232, 112, "sstep_enable()", size=12.5, fill=GREEN, anchor="end")
    # HOLDING -> DISABLED
    s.path("M360,115 C300,120 270,150 260,170", color=GREY, arrow="end", width=1.6)
    s.text(268, 140, "sstep_disable()", size=12, fill=GREY, anchor="start")
    # HOLDING -> MOVING
    s.path("M560,95 C640,95 700,110 720,160", color=TEAL, arrow="teal", width=1.8)
    s.text(668, 92, "move_to / move_by / run", size=12.5, fill=TEAL, anchor="start")
    # MOVING -> HOLDING
    s.path("M680,160 C650,135 610,125 560,118", color=GREEN, arrow="green", width=1.6)
    s.text(608, 152, "move done, stopped", size=12, fill=GREEN, anchor="middle")
    # MOVING -> DISABLED
    s.path("M660,205 C520,230 400,230 260,205", color=GREY, arrow="end", width=1.6, )
    s.text(460, 246, "sstep_disable()", size=12, fill=GREY)
    # faults
    s.path("M460,130 L460,290", color=RED, width=1.8)
    s.path("M455,282 L460,290 L465,282", color=RED, width=1.8)
    s.path("M740,230 C720,290 640,320 560,325", color=RED, width=1.8)
    s.path("M570,320 L560,325 L570,331", color=RED, width=1.8)
    s.text(472, 262, "fault", size=12.5, fill=RED, anchor="start")
    s.text(700, 300, "fault", size=12.5, fill=RED, anchor="start")
    # FAULT -> DISABLED
    s.path("M360,325 C250,325 170,290 160,230", color=GREY, arrow="end", width=1.6)
    s.text(190, 318, "sstep_clear_faults()", size=12.5, fill=GREY, anchor="end")
    s.text(460, 395, "Faults: overcurrent, over/undervoltage, callback error, "
           "sstep_report_fault(). Outputs go off at once.", size=12.5, fill=GREY)
    s.save(OUT + "/state-machine.svg")


# ==============================================================================
# MICROSTEPPING
# ==============================================================================

def microstep() -> None:
    s = Svg(960, 400, "Current vector, electrical angle and microsteps")
    cx, cy, r = 190, 200, 140
    s.line(cx - r - 30, cy, cx + r + 30, cy, color=LINE)
    s.line(cx, cy + r + 30, cx, cy - r - 30, color=LINE)
    s.text(cx + r + 34, cy + 5, "iA", size=13, anchor="start", weight="bold")
    s.text(cx + 8, cy - r - 30, "iB", size=13, anchor="start", weight="bold")
    s.parts.append(f'<circle cx="{cx}" cy="{cy}" r="{r}" fill="none" stroke="{BLUE}" '
                   f'stroke-width="1.5" stroke-dasharray="4 4"/>')
    m = 4  # microsteps shown per full step
    for k in range(4 * m):
        a = 2 * math.pi * k / (4 * m)
        x, y = cx + r * math.cos(a), cy - r * math.sin(a)
        full = (k % m) == 0
        s.parts.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{6 if full else 4}" '
                       f'fill="{NAVY if full else TEAL}"/>')
    a = 2 * math.pi * 3 / (4 * m)
    s.line(cx, cy, cx + r * math.cos(a), cy - r * math.sin(a), color=ORANGE, arrow="orange",
           width=2.6)
    s.path(f"M{cx + 46},{cy} A46,46 0 0,0 {cx + 46 * math.cos(a):.1f},{cy - 46 * math.sin(a):.1f}",
           color=ORANGE)
    s.text(cx + 58, cy - 20, "φ", size=16, fill=ORANGE, weight="bold", anchor="start")
    s.parts.append(f'<circle cx="{cx - 150}" cy="{cy + r + 52}" r="6" fill="{NAVY}"/>')
    s.text(cx - 138, cy + r + 56, "full step (90° electrical)", size=12, fill=GREY, anchor="start")
    s.parts.append(f'<circle cx="{cx + 30}" cy="{cy + r + 52}" r="4" fill="{TEAL}"/>')
    s.text(cx + 40, cy + r + 56, "microstep (M = 4)", size=12, fill=GREY, anchor="start")
    # waveforms
    x0, y0, w, h = 420, 70, 500, 260
    s.line(x0, y0 + h / 2, x0 + w, y0 + h / 2, color=LINE)
    for k in range(5):
        x = x0 + k * w / 4
        s.line(x, y0 + 6, x, y0 + h - 6, color=LINE, dash=True)
        s.text(x, y0 + h + 18, f"{k * 90}°", size=11.5, fill=GREY)
    steps = 4 * 8
    for fn, color, label in ((math.cos, BLUE, "iA = I cos φ"), (math.sin, TEAL, "iB = I sin φ")):
        d = []
        for k in range(steps):
            a1 = 2 * math.pi * k / steps
            v = fn(a1)
            xa, xb = x0 + w * k / steps, x0 + w * (k + 1) / steps
            yv = y0 + h / 2 - v * (h / 2 - 16)
            d.append(f"{'M' if k == 0 else 'L'}{xa:.1f},{yv:.1f} L{xb:.1f},{yv:.1f}")
        s.path(" ".join(d), color=color, width=2)
        lx = x0 + (0 if fn is math.cos else 170)
        s.line(lx, y0 - 14, lx + 24, y0 - 14, color=color, width=2.4)
        s.text(lx + 30, y0 - 9, label, size=12.5, fill=color, anchor="start", weight="bold")
    s.text(x0 + w / 2, 26, "One electrical turn = 4 full steps = 2^32 angle units; "
           "a microstep = 2^30 / M", size=13, fill=NAVY)
    s.text(x0 + w / 2, y0 + h + 44, "8 microsteps per full step shown; the fast loop "
           "interpolates between them", size=12, fill=GREY)
    s.save(OUT + "/microstep.svg")


# ==============================================================================
# CURRENT CONTROL LOOP
# ==============================================================================

def control_loop() -> None:
    s = Svg(1000, 360, "Current control loop of one phase")

    def summ(x, y, signs):
        s.parts.append(f'<circle cx="{x}" cy="{y}" r="15" fill="{WHITE}" stroke="{NAVY}" '
                       f'stroke-width="1.6"/>')
        s.text(x, y + 5, "Σ", size=14, weight="bold")
        for dx, dy, sign in signs:
            s.text(x + dx, y + dy, sign, size=13, fill=NAVY, weight="bold")

    y = 130
    box(s, 20, y - 36, 120, 72, "Angle φ", ["sin, cos table"])
    box(s, 160, y - 36, 110, 72, "× I", ["run or hold", "current"])
    arrow(s, 140, y, 160, y)
    arrow(s, 270, y, 315, y, label="i_ref")
    summ(330, y, [(-24, 22, "+"), (10, 32, "−")])
    arrow(s, 345, y, 390, y, label="e")
    box(s, 390, y - 40, 150, 80, "PI", ["Kp = L ωc", "Ki = R ωc"], fill=SOFT_TEAL, stroke=TEAL)
    arrow(s, 540, y, 580, y)
    summ(595, y, [(-24, 22, "+"), (12, -18, "+")])
    s.line(595, 40, 595, y - 15, color=GREY, arrow="end")
    s.text(605, 52, "back-EMF feed-forward Ke ω", size=12, fill=GREY, anchor="start")
    arrow(s, 610, y, 650, y, label="v")
    box(s, 650, y - 40, 140, 80, "Voltage limit", ["topology, V_base/Vbus"],
        fill=SOFT_ORANGE, stroke=ORANGE)
    arrow(s, 790, y, 830, y)
    box(s, 830, y - 40, 150, 80, "Duty → PWM", ["pwm_write()"])
    # plant
    box(s, 650, 250, 330, 76, "Motor phase", ["R, L, back-EMF; measured by shunt + ADC"],
        fill=LIGHT, stroke=BLUE)
    s.line(905, y + 40, 905, 250, color=GREY, arrow="end")
    s.path(f"M650,288 L330,288 L330,{y + 15}", color=GREY, arrow="end")
    s.text(480, 280, "i_meas (ADC counts → pu)", size=12.5, fill=GREY)
    # anti-windup
    s.path(f"M720,{y + 40} L720,212 L465,212 L465,{y + 40}", color=ORANGE, arrow="orange")
    s.text(592, 230, "anti-windup: Ts R / L · (v_applied − v)", size=12, fill=ORANGE)
    s.text(500, 344, "One loop per phase at fast_loop_hz; closed loop i_meas / i_ref = ωc / (s + ωc)",
           size=12.5, fill=NAVY)
    s.save(OUT + "/control-loop.svg")


# ==============================================================================
# POWER STAGE TOPOLOGIES
# ==============================================================================

def _leg(s: Svg, x, top, bottom, label) -> float:
    """One half-bridge between the rails; returns the y of its output node."""
    mid = (top + bottom) / 2
    for y in (top + 18, mid + 14):
        s.rect(x - 16, y, 32, 34, fill=WHITE, stroke=NAVY, rx=4)
    s.line(x, top, x, top + 18, color=NAVY)
    s.line(x, top + 52, x, mid + 14, color=NAVY)
    s.line(x, mid + 48, x, bottom, color=NAVY)
    s.parts.append(f'<circle cx="{x}" cy="{mid + 6}" r="4" fill="{NAVY}"/>')
    s.text(x, bottom + 20, label, size=13, weight="bold", fill=NAVY)
    return mid + 6


def _coil(s: Svg, x1, x2, y, label, color) -> None:
    s.rect(x1, y - 12, x2 - x1, 24, fill=WHITE, stroke=color, rx=12, sw=2)
    s.text((x1 + x2) / 2, y + 5, label, size=12.5, weight="bold", fill=color)


def _region(s: Svg, cx, cy, r, poly, circle) -> None:
    s.line(cx - r - 14, cy, cx + r + 14, cy, color=LINE)
    s.line(cx, cy + r + 14, cx, cy - r - 14, color=LINE)
    pts = " ".join(f"{cx + a * r:.1f},{cy - b * r:.1f}" for a, b in poly)
    s.parts.append(f'<polygon points="{pts}" fill="{SOFT_ORANGE}" stroke="{ORANGE}" '
                   f'stroke-width="1.8"/>')
    s.parts.append(f'<circle cx="{cx}" cy="{cy}" r="{circle * r:.1f}" fill="none" '
                   f'stroke="{TEAL}" stroke-width="2" stroke-dasharray="5 4"/>')
    s.text(cx + r + 18, cy + 4, "vA", size=11.5, fill=GREY, anchor="start")
    s.text(cx - 6, cy - r - 18, "vB", size=11.5, fill=GREY, anchor="end")


def topologies() -> None:
    s = Svg(1000, 520, "Full bridge and shared-leg topologies with their voltage limits")
    top, bottom = 70, 250
    for x0, title in ((20, "SSTEP_TOPOLOGY_FULL_BRIDGE · 4 legs"),
                      (520, "SSTEP_TOPOLOGY_SHARED_LEG · 3 legs")):
        s.rect(x0, 16, 460, 488, fill="#F7F9FC", stroke=LINE, rx=12)
        s.text(x0 + 230, 44, title, size=14, weight="bold", fill=BLUE, mono=True)
    # full bridge
    xs = (70, 170, 300, 400)
    s.line(50, top, 430, top, color=RED, width=2)
    s.line(50, bottom, 430, bottom, color=GREY, width=2)
    s.text(436, top + 4, "Vbus", size=12, fill=RED, anchor="start")
    s.text(436, bottom + 4, "0", size=12, fill=GREY, anchor="start")
    nodes = [_leg(s, x, top, bottom, n) for x, n in zip(xs, ("A+", "A−", "B+", "B−"))]
    for xa, xb, n, label, color in ((70, 170, nodes[0], "A", BLUE), (300, 400, nodes[2], "B", TEAL)):
        s.line(xa, n, xa + 18, n, color=color, width=1.6)
        s.line(xb - 18, n, xb, n, color=color, width=1.6)
        _coil(s, xa + 18, xb - 18, n, label, color)
    s.text(250, 288, "vA = A+ − A−;  vB = B+ − B−", size=12.5, fill=NAVY, mono=True)
    _region(s, 250, 400, 66, [(1, 1), (-1, 1), (-1, -1), (1, -1)], 1.0)
    s.text(250, 494, "max(|vA|, |vB|) ≤ Vbus · circle: amplitude Vbus", size=12, fill=GREY)
    # shared leg
    xs = (600, 740, 880)
    s.line(580, top, 940, top, color=RED, width=2)
    s.line(580, bottom, 940, bottom, color=GREY, width=2)
    s.text(944, top + 4, "Vbus", size=12, fill=RED, anchor="start")
    s.text(944, bottom + 4, "0", size=12, fill=GREY, anchor="start")
    nodes = [_leg(s, x, top, bottom, n) for x, n in zip(xs, ("U", "V", "W (shared)"))]
    ya, yb = nodes[0] - 34, nodes[0] - 14
    s.line(600, ya, 620, ya, color=BLUE, width=1.6)
    _coil(s, 620, 722, ya, "A (U − W)", BLUE)
    s.line(722, ya, 880, ya, color=BLUE, width=1.6)
    s.line(740, yb, 766, yb, color=TEAL, width=1.6)
    _coil(s, 766, 862, yb, "B (V − W)", TEAL)
    s.line(862, yb, 880, yb, color=TEAL, width=1.6)
    for x, y in ((600, ya), (880, ya), (740, yb), (880, yb)):
        s.parts.append(f'<circle cx="{x}" cy="{y}" r="4" fill="{NAVY}"/>')
    s.text(750, 288, "vA = U − W;  vB = V − W", size=12.5, fill=NAVY, mono=True)
    _region(s, 750, 400, 66, [(1, 0), (1, 1), (0, 1), (-1, 0), (-1, -1), (0, -1)],
            1 / math.sqrt(2))
    s.text(750, 494, "max(vA, vB, 0) − min(vA, vB, 0) ≤ Vbus · circle: Vbus / √2",
           size=12, fill=GREY)
    s.save(OUT + "/topologies.svg")


# ==============================================================================
# MOTION PROFILE
# ==============================================================================

def trajectory() -> None:
    """Profile of the closed-loop test move: 6400 microsteps, v_max 16 000/s,
    accel 160 000/s² (0.1 s ramps, 0.3 s at v_max, 0.5 s in total)."""
    s = Svg(960, 400, "Trapezoidal motion profile")
    dist, vmax, acc = 6400.0, 16000.0, 160000.0
    ta = vmax / acc
    tc = (dist - vmax * ta) / vmax
    total = 2 * ta + tc

    def vel(t):
        if t < ta:
            return acc * t
        if t < ta + tc:
            return vmax
        return max(0.0, vmax - acc * (t - ta - tc))

    def pos(t):
        n, p, dt = 400, 0.0, t / 400 if t > 0 else 0.0
        for k in range(n):
            p += vel((k + 0.5) * dt) * dt
        return p

    x0, w = 180, 740
    for y0, h, title, fn, top, color, unit in (
            (40, 140, "velocity", vel, vmax, BLUE, "16 000 microsteps/s"),
            (230, 140, "position", pos, dist, TEAL, "6 400 microsteps")):
        s.line(x0, y0 + h, x0 + w, y0 + h, color=LINE)
        s.line(x0, y0, x0, y0 + h, color=LINE)
        s.text(x0 - 10, y0 + 6, unit, size=11.5, fill=GREY, anchor="end")
        s.text(x0 - 10, y0 + h / 2 + 4, title, size=13, fill=color, anchor="end",
               weight="bold")
        pts = []
        for k in range(201):
            t = total * k / 200
            pts.append(f"{'M' if k == 0 else 'L'}{x0 + w * t / total:.1f},"
                       f"{y0 + h - fn(t) / top * h:.1f}")
        s.path(" ".join(pts), color=color, width=2.4)
    for t, label in ((0, "0"), (ta, "0.1 s"), (ta + tc, "0.4 s"), (total, "0.5 s")):
        x = x0 + w * t / total
        s.line(x, 40, x, 370, color=LINE, dash=True)
        s.text(x, 390, label, size=11.5, fill=GREY)
    s.text(x0 + w * ta / total / 2, 30, "accel", size=12, fill=GREY)
    s.text(x0 + w * (ta + tc / 2) / total, 30, "v_max", size=12, fill=GREY)
    s.text(x0 + w * (ta + tc + ta / 2) / total, 30, "decel", size=12, fill=GREY)
    s.save(OUT + "/trajectory.svg")


# ==============================================================================
# INTEGRATION TIMING
# ==============================================================================

def integration() -> None:
    s = Svg(980, 400, "PWM, ADC and the two loops on a microcontroller")
    x0, w, periods = 130, 740, 4
    pw = w / periods
    # carrier
    y_top, y_bot = 40, 120
    d = []
    for k in range(periods):
        xa = x0 + k * pw
        d.append(f"{'M' if k == 0 else 'L'}{xa:.1f},{y_bot} L{xa + pw / 2:.1f},{y_top} "
                 f"L{xa + pw:.1f},{y_bot}")
    s.path(" ".join(d), color=BLUE, width=2)
    s.text(x0 - 12, 86, "PWM counter", size=12.5, fill=BLUE, anchor="end")
    s.text(x0 - 12, 102, "(center-aligned)", size=11, fill=GREY, anchor="end")
    for k in range(periods):
        xc = x0 + k * pw + pw / 2
        s.line(xc, y_top, xc, 330, color=LINE, dash=True)
        # ADC
        s.rect(xc, 140, 50, 26, fill=SOFT_ORANGE, stroke=ORANGE, rx=4)
        s.text(xc + 25, 158, "ADC", size=11.5, fill=ORANGE, weight="bold")
        # fast loop
        s.rect(xc + 54, 186, 92, 26, fill=SOFT_TEAL, stroke=TEAL, rx=4)
        s.text(xc + 100, 204, "fast loop", size=11.5, fill=TEAL, weight="bold")
        arrow(s, xc + 146, 199, x0 + (k + 1) * pw - 2, 240, color=GREY)
        s.rect(x0 + (k + 1) * pw - 34, 240, 68, 24, fill=WHITE, stroke=NAVY, rx=4)
        s.text(x0 + (k + 1) * pw, 257, "preload", size=11, fill=NAVY)
    s.text(x0 - 12, 158, "ADC trigger", size=12.5, fill=ORANGE, anchor="end")
    s.text(x0 - 12, 204, "EOC interrupt", size=12.5, fill=TEAL, anchor="end")
    s.text(x0 - 12, 257, "compare update", size=12.5, fill=NAVY, anchor="end")
    # slow loop
    s.rect(x0 + 20, 290, w - 40, 30, fill=LIGHT, stroke=BLUE, rx=6)
    s.text(x0 + w / 2, 310, "slow loop: once every fast_loop_hz / slow_loop_hz periods "
           "(20 at 20 kHz / 1 kHz), preempted by the interrupt", size=12, fill=BLUE)
    s.text(x0 - 12, 310, "Main loop/task", size=12.5, fill=BLUE, anchor="end")
    s.text(x0 + w / 2, 366, "Sample at the center of the period (low-side switches on); the new "
           "duty cycles take effect at the next period boundary.", size=12.5, fill=NAVY)
    s.save(OUT + "/integration.svg")


# ==============================================================================
# HERO
# ==============================================================================

def hero() -> None:
    """Animated isometric hero: the controller board with the Stepper-C chip and three
    half-bridges drives a NEMA 17 motor; the two phase currents (cosine and sine) flow
    along the wires, the shaft turns."""
    iso = Iso()
    p = iso.p
    # board and traces
    iso.box(0, 10, 0, 7.2, -0.4, 0, "url(#boardTop)", "#0a1628", "#071122", "#1e3a66", 1.2)
    traces = []
    for i in range(6):
        y = 2.3 + i * 0.55
        traces.append([p(2.0, y), p(1.1, y), p(0.7, y - 0.4), p(0.25, y - 0.4)])
        x = 2.3 + i * 0.55
        traces.append([p(x, 2.0), p(x, 1.1), p(x + 0.35, 0.75), p(x + 0.35, 0.25)])
    for trace in traces:
        iso.path(trace, "#1d5fbf", 1.3, ' opacity="0.7"')
    for i, y in enumerate((1.6, 3.2, 4.8)):
        trace = [p(5.4, 3.6), p(6.0, y + 0.45), p(6.9, y + 0.45)]
        iso.path(trace, "#1d5fbf", 1.6)
        iso.path(trace, "#5ea6ff", 1.6, ' class="flow"')
    # three half-bridges (high-side and low-side switch each) and the shunts
    for y in (1.6, 3.2, 4.8):
        for x in (6.9, 7.9):
            iso.box(x, x + 0.8, y, y + 0.9, 0, 0.42, "#14243c", "#0c182b", "#09121f", "#2a4870")
            iso.box(x + 0.15, x + 0.65, y + 0.9, y + 1.05, 0, 0.1, "#8fa6c6", "#6b819f", "#5a6f8b")
    # phase connector
    iso.box(9.2, 10.0, 2.4, 4.8, 0, 0.55, "#c9d6ea", "#8aa0bf", "#6f86a6")
    # library chip
    for i in range(7):
        c = 2.25 + i * 0.45
        iso.box(c - 0.08, c + 0.08, 5.2, 5.45, 0, 0.12, "#8fa6c6", "#6b819f", "#5a6f8b")
        iso.box(5.2, 5.45, c - 0.08, c + 0.08, 0, 0.12, "#8fa6c6", "#6b819f", "#5a6f8b")
    iso.box(2.0, 5.2, 2.0, 5.2, 0, 0.75, "url(#chipTop)", "#0a1322", "#060d18", "#2f8cff", 1.0)
    iso.text(3.6, 3.55, 0.76, "Stepper-C", 0.5, "#e6edf7", 800, 0.03)
    iso.text(3.6, 4.25, 0.76, "CURRENT CONTROL", 0.24, "#5ea6ff", 600, 0.06)
    iso.led(2.45, 2.45, 0.76)
    iso.led(9.6, 2.7, 0.56, color="#5ea6ff", r=2.4)

    # wires: phase A (blue) and phase B (teal) to the motor
    mx0, mx1, my0, my1 = 18.0, 22.2, 1.4, 5.6
    for k, (color, glow, y_board, y_motor) in enumerate(
            (("#1e5aa8", "#5ea6ff", 2.9, 2.8), ("#0f8b9a", "#4fd1d9", 4.2, 4.2))):
        wire = [p(10.0, y_board, 0.3), p(11.4, y_board, 0.3), p(16.0, y_motor, 0.3),
                p(mx0, y_motor, 0.3)]
        iso.path(wire, "#0e2a4f", 6)
        iso.path(wire, color, 2.2)
        # the phase current as a glowing wave travelling along the wire
        wave = []
        for n in range(121):
            t = n / 120
            x = 11.0 + t * (mx0 - 11.8)
            z = 1.6 + 0.6 * math.sin(2 * math.pi * (2.5 * t) + (0 if k == 0 else math.pi / 2))
            wave.append(p(x, y_board + (y_motor - y_board) * t, z + 0.7 * k))
        iso.path(wave, color, 2.4, ' opacity="0.55" filter="url(#glow)"')
        iso.path(wave, glow, 2.0, ' class="flow"')

    # NEMA 17 motor: end caps, laminated body, boss and shaft
    iso.box(mx0, mx1, my0, my1, -0.4, 0.5, "#c9d6ea", "#8aa0bf", "#6f86a6")
    for n in range(6):
        z0 = 0.5 + n * 0.45
        iso.box(mx0, mx1, my0, my1, z0, z0 + 0.45, "#1b2a40",
                "#131f31" if n % 2 == 0 else "#17253a", "#0e1726" if n % 2 == 0 else "#111d2f",
                "#2a4870", 0.6)
    iso.box(mx0, mx1, my0, my1, 3.2, 3.8, "#c9d6ea", "#8aa0bf", "#6f86a6")
    cx, cy = (mx0 + mx1) / 2, (my0 + my1) / 2
    iso.disc(cx, cy, 3.8, 1.3, [(1.0, "#8aa0bf"), (0.92, "#b5c4da"), (0.5, "#6f86a6")])
    iso.disc(cx, cy, 3.85, 0.32, [(1.0, "#e6edf7"), (0.55, "#9aabc4")])
    ox, oy = p(cx, cy, 3.86)
    iso.parts.append(
        f'<g transform="matrix({iso._matrix("top")} {ox:.1f} {oy:.1f})">'
        '<g><line x1="0" y1="0" x2="0.3" y2="0" stroke="#2f8cff" stroke-width="0.09" '
        'stroke-linecap="round"/><animateTransform attributeName="transform" type="rotate" '
        'from="0" to="360" dur="4s" repeatCount="indefinite"/></g></g>')
    for corner_x, corner_y in ((mx0 + 0.45, my0 + 0.45), (mx1 - 0.45, my0 + 0.45),
                               (mx0 + 0.45, my1 - 0.45), (mx1 - 0.45, my1 - 0.45)):
        iso.disc(corner_x, corner_y, 3.8, 0.16, [(1.0, "#5a6f8b"), (0.5, "#2a3a52")])

    label = "Stepper-C drives the two phase currents of a stepper motor"
    path = os.path.join(OUT, "hero-3d.svg")
    with open(path, "w", encoding="utf-8", newline=chr(10)) as f:
        f.write(iso.svg(label, HERO_DEFS, HERO_STYLE, glow_at=(11, 4)))


def main() -> None:
    os.makedirs(OUT, exist_ok=True)
    hero()
    architecture()
    data_flow()
    state_machine()
    microstep()
    control_loop()
    topologies()
    trajectory()
    integration()
    for name in sorted(os.listdir(OUT)):
        if name.endswith(".svg"):
            print("wrote", os.path.join("docs", "assets", "images", name))


if __name__ == "__main__":
    main()
