Welcome to my walk down memory lane!

This BigInt code was part of a utility library written by DejaVu
Software, Inc. - a small software company that wrote software for
early handheld computers. DejaVu's founding members (Scott Harker and
me, Jorj Bauer) released products for the Apple Newton, Palm Pilot,
Windows CE 2.x through 6.x, and Apple iOS.

BigInt was used for our shareware registration codes but served a
larger purpose in PockeTTY, a terminal emulator with SSH and SSL
support. The SSH and SSL implementations used this BigInt code as the
basis of our self-implemented encryption support.

DejaVu Software, Inc. closed its doors in 2015 and we are releasing
this code under the MIT license.

BigInt was originally written and debugged on 32-bit Linux machines
and then ported to Windows CE. Because of the diversity of Windows CE
platforms and various idiosyncrasies in the compilers, we wrote this
code to support multiple data storage bit-sizes. It currently supports
16-bit, 32-bit, and 64-bit storage and, as I'm distributing it,
expects to be compiled on a 32-bit architecture.

To change the architecture, change this line in BigInt.h:

   #define BIGINT_PRIMITIVE_SIZE 32

Supported values are 64, 32, and 16. (Actually, anything that's not 64
or 32 will wind up compiling for 16.)

There was a time when compiling for larger bit-sizes meant a
performance boost. Minor testing with LLVM 6.1.0 shows almost no
difference between the 64 and 32 bit storage engines, presumably due
to modern compiler optimizations.

Casual testing shows the Lua-wrapped implementation to be about the
same speed as the original C++ code. In practical situations, it's
only half that speed because of dynamic type conversion and having to
create new BigInt arguments on the fly.
