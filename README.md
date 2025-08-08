# FocusFader
This utility for Windows will fade all windows except the one with the keyboard focus, at each focus change (e.g. Alt-Tab).

This is very usefull when several windows are visible on the desktop (e.g. with mutiple monitors) to know which one has the focus, as with the new Win10/11 UI and most applications, the title bar is not changing anymore.

![Screen capture](./capture.mp4)
![Screen capture](./capture.png)

## Installation

Download and execute the executable: [bin/focus_fader.exe](bin/focus_fader.exe)

Or compile from a MinGW shell with `make`.

## Usage
Fading can be enabled, disabled or adjusted from the icon in the system tray.

Shortcuts:
 - Ctrl+Shift+KP_Minus: quit
 - Ctrl+Shift+KP_Plus: enable/disable

A debug log can be generated if run from a terminal.

## Troubleshooting

This software is in alpha version and has probably a lot of bugs, as well as the following limitations:

- cannot change shortcuts
- cannot save transparency threshold
- will increase memory usage of the Desktop Window Manager (usual case with 10 windows: from 100M to 200M)
- with Intel Graphics Control Center, IgfxEM module will slowly consumme up to 20% CPU permanently
- I would like to just gray out windows instead of making them transparent, but have no clue on how to do this