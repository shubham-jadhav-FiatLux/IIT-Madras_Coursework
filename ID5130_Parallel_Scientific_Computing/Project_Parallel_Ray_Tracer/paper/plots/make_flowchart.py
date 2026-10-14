#!/usr/bin/env python3
"""Render the rendering-pipeline flowchart as figures/flowchart.pdf.

Vertical layout that fits inside one LaTeX column (≈3.3 in wide). Three
visual phases are coloured by lane:
  * sequential setup on rank 0 (light blue)
  * parallel render within each rank (light green)
  * cross-rank communication and output (light amber)
"""
import os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "figures",
                   "flowchart.pdf")

plt.rcParams.update({
    "font.family": "serif",
    "font.size": 8.5,
    "savefig.bbox": "tight",
    "savefig.dpi": 300,
})

C_SETUP    = "#E8EEF5"
C_PARALLEL = "#E9F1E1"
C_COMM     = "#F5E6DC"
C_OUTPUT   = "#F2F2F2"
C_BORDER   = "#5B6470"
C_ARROW    = "#5B6470"
C_TXT      = "#2A2F36"

# (label, fill, lane_label_or_None_to_skip)
specs = [
    ("Scene parser",                    C_SETUP,    None),
    ("BVH build (binned SAH)",          C_SETUP,    None),
    ("Tile list ($32{\\times}32$)",     C_SETUP,    None),
    ("Preview pass (adaptive only)",    C_PARALLEL, None),
    ("Render tiles (dynamic)",          C_PARALLEL, None),
    ("Pack pixels per rank",            C_PARALLEL, None),
    ("MPI Gatherv + assembly",          C_COMM,     None),
    ("PPM image output",                C_OUTPUT,   None),
]

# Geometry
W = 62.0   # box width (in fig units)
H = 8.5    # box height
gap = 3.5  # uniform arrow gap
n = len(specs)
total_h = n * H + (n - 1) * gap
margin_top = 5.0
margin_bot = 3.0
gaps = [gap] * (n - 1)

fig_h = (total_h + margin_top + margin_bot) / 100 * 4.0  # ~4 in tall
fig, ax = plt.subplots(figsize=(3.35, fig_h))
ax.set_xlim(0, 100)
ax.set_ylim(-margin_bot, total_h + margin_top)
ax.axis("off")

# Boxes centred slightly left to leave room for the right-side legend
x_left = (100 - W) / 2 - 6
y_cur = total_h - H

centres = []
for i, (label, fill, lane) in enumerate(specs):
    if lane:
        ax.text(50, y_cur + H + 1.6, lane,
                ha="center", va="bottom", fontsize=7,
                style="italic", color="#5B6470")
    box = FancyBboxPatch(
        (x_left, y_cur), W, H,
        boxstyle="round,pad=0.3,rounding_size=0.7",
        linewidth=0.7, edgecolor=C_BORDER, facecolor=fill)
    ax.add_patch(box)
    ax.text(x_left + W/2, y_cur + H/2, label,
            ha="center", va="center", fontsize=7.8, color=C_TXT)
    centres.append((x_left + W/2, y_cur, y_cur + H))
    if i < n - 1:
        y_cur -= (H + gaps[i])

# Down-arrows between consecutive boxes
for i in range(n - 1):
    cx_top = centres[i][0]
    y_from = centres[i][1]
    y_to   = centres[i + 1][2]
    ax.add_patch(FancyArrowPatch(
        (cx_top, y_from - 0.1), (cx_top, y_to + 0.1),
        arrowstyle="->", mutation_scale=8, color=C_ARROW, lw=0.8))

# Lane legend (right side, vertical) -- aligned with the lanes' colour bands
def _band_y(top_idx, bot_idx):
    top_y = centres[top_idx][2]    # top of first box in lane
    bot_y = centres[bot_idx][1]    # bottom of last box in lane
    return (top_y + bot_y) / 2

LX = x_left + W + 2.0
ax.text(LX, _band_y(0, 2),
        "Sequential\n(rank 0)",
        ha="left", va="center", fontsize=6.8,
        style="italic", color="#5B6470")
ax.text(LX, _band_y(3, 5),
        "Parallel\n(OpenMP)",
        ha="left", va="center", fontsize=6.8,
        style="italic", color="#5B6470")
ax.text(LX, _band_y(6, 7),
        "Cross-rank\n+ output",
        ha="left", va="center", fontsize=6.8,
        style="italic", color="#5B6470")

plt.savefig(OUT)
plt.close()
print("wrote", OUT)
