metatoy
=======
Metatoy is mainly a tech demo for COM32, my experimental (and still quite buggy)
DOS protected mode kernel, which starts as a flat binary (COM file).

About the game
--------------
To be determined...

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
Copyright (C) John Tsiombikas <nuclear@mutantstargoat.com>

This program is free software, feel free to use, modify, and/or redistribute it
under the terms of the GNU General Public License v3, or at your option any
later version published by the Free Software Foundation. See COPYING for
details.

Build
-----
You need gcc, nasm, and GNU make. If you do, just type `make`.
