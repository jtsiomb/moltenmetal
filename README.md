Molten Metal
============
Molten metal is mainly a tech demo for COM32, my experimental (and still quite
buggy) DOS protected mode kernel, which starts as a flat binary (COM file).

About the game
--------------
There is no goal or fail state. You just play with shapes pulled out of a pool
of molten metal.

### Controls
Grab with the left mouse button and move the shapes around, then let them fall
back into the pool by releasing the mouse button.

  - space: grab/release shape without using the mouse.
  - tab: change shape.
  - -/+: change the resolution of the voxel field in 5 voxel increments.
  - '0': return to the default voxel field resolution.

About COM32
-----------
The idea is to facilitate writing small protected mode DOS programs, without
having to rely on DOS-specific toolchains. Use COM32 and GCC on GNU/Linux, build
it as a flat binary, and have it run in 32bit protected mode under DOS.
The downside is mainly that programs are constrained to 64kb, and that there's
no debugger support yet.

Beyond the COM loader and DOS-specific additions, the main bulk of the kernel is
really a large chunk out of my earlier pcboot project:
https://github.com/jtsiomb/pcboot

Frankly there isn't much of a point in any of this, other than being a fun hack.

License
-------
Copyright (C) 2023 John Tsiombikas <nuclear@mutantstargoat.com>

This program is free software, feel free to use, modify, and/or redistribute it
under the terms of the GNU General Public License v3, or at your option any
later version published by the Free Software Foundation. See COPYING for
details.

Build
-----
The data files are not included in the git repo. If you cloned the git repo, you
need to grab a copy of the data directory from the latest release archive before
building.

### DOS
To build this program you'll need gcc, nasm, and GNU make. Only tested building
from an x86 64bit GNU/Linux machine, it will need some minor makefile
adjustments to build on anything else.

Run `make -f Makefile.dos` to build the DOS 32bit COM executable.

### UNIX/Windows
The cross-platform non-DOS version uses OpenGL to display the software-rendered
framebuffer into a window.

Run `make` to build.
