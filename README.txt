

P3 WhiteBalancer v1.2
================================================================================


Harrison Ainsworth / HXA7241 : 2005-2007.  
http://wwww.hxa7241.org/whitebalancer/

2007-12-23




Contents
--------

* Description
* Requirements
* Application usage
* Library usage
* Acknowledgements
* Notices




Description
-----------

WhiteBalancer removes the color cast from an image.

This emulates the perceptual capability of 'color-constancy': it makes the
colors look more like they did in the scene where the image was created. 

It is command-line application and a dynamic library, for Windows and Linux.

Application features:
* reads and writes PPM and RGBE/HDR
* reads and writes PNG and EXR -- if you have the related libraries present
* allows full control of the P3WhiteBalancer library

Library features:
* float-triplet-pixel images accepted (linear, not gamma-corrected)
* HDR or LDR images accepted
* image colorspace and whitepoint specifiable
* original illuminant specifiable, or automatically estimated
* strength of color-shift adjustable
* fast enough for semi-interactive use




Requirements
------------

* Windows 2000 or later, or Linux 2.6.11 or later.
* The host CPU must support SSE1 (Pentium 3 equivalent or later).
* Dynamic libraries for OpenEXR and PNG must be present to use those formats.




Application Usage
-----------------

### general form ###

   p3whitebalancer {--help|-?}
   p3whitebalancer [options] [-x:optionsFilePathName] [-z] imageFilePathName


### options (with defaults shown) ###

image formatting libraries:
   -lp:<string>    libpng pathname
   -le:<string>    openexr pathname
image metadata:
   -ic:<6 floats>  colorspace: 0.64_0.33_0.30_0.60_0.15_0.06
   -iw:<2 floats>  whitepoint: 0.333_0.333
   -ig:<float>     gamma transform (encode): 0.45
   -ii:<3 floats>  original illuminant (rgb): estimated from image
mapping options:
   -ms:<float>     strength (0-1): 0.8
output file name:
   -on:<string>    output file path name (no ext): inputFilePathName_p3wb


### notes ###

The image file name must be last, and must end in '.ppm', '.png',
.exr', '.hdr', '.pic', '.rad', or '.rgbe'.

optionsFilePathName defaults to 'p3whitebalancer-opt.txt'

Options can be put in a file instead the command line. The
-x:CommandFilePathName switch nominates that file, or it defaults to
p3tonemapper-opt.txt. Each option there can be separated by any blankspace.

Options can be in four places: built-in defaults, the image file (only a
subset), the command file, and the command line. They are read in that order,
each overriding the previous.

-z switches on some feedback


### example ###

   p3whitebalancer someimage.exr
   p3whitebalancer -ms:0.6 -on:resultimage someimage.png




Library Usage
-------------

### basics ###

The library has a C interface, defined in p3wbWhiteBalancer.h and
p3wbWhiteBalancer-v12.h . The former, main, part should be stable across
versions. The latter specifies constants that will change with versions. There
are no other dependencies.


### building ###

To build a client: include p3wbWhiteBalancer-v12.h and link with the
p3whitebalancer.lib import library (Windows) or the libp3whitebalancer.so 
library (Linux), or access purely dynamically.


### calling ###

There are two interface sections: meta-versioning, and functions.

__Meta-versioning interface__:
For checking a dynamically linked library supports the interfaces used by the
client.

__Function interface__:
Call a function with an image and parameters, and receive a result image. There
are two alternatives: all parameters, and simple (uses defaults).

For full details, look at p3wbWhiteBalancer-v12.h and p3wbWhiteBalancer.h .


### data range ###

Pre-conditions:
* Input pixels may have any value, however:
   * Very small values and negatives will be clamped to a sub-perceptual
     small value before processing.
   * Very large values will be clamped to a super-perceptual large value before
     processing.
* Non-pixel data must be valid (or it may cause failureful return).

Post-conditions:
* Output pixel channels are positive.
* Input pixels that are black result in black output pixels
* Input pixels containing NaNs pass through unused and unchanged.




Acknowledgements
----------------

* Color adjustment idea adapted from:  
  'Color Transfer Between Images'  
  Reinhard, Ashikhmin, Gooch, Shirley;  
  IEEE Computer Graphics And Applications,  
  2001-10.  
  <http://www.cs.ucf.edu/~reinhard/papers/colourtransfer.pdf>
* Fast log2 approximation adapted from:  
  Laurent de Soras, 2001.  
  <http://flipcode.com/cgi-bin/fcarticles.cgi?show=63828>
* Fast log with adjustable accuracy, adapted from:  
  'Revisiting a basic function on current CPUs: A fast logarithm implementation
  with adjustable accuracy'  
  Oriol Vinyals, Gerald Friedland, Nikki Mirghafori;  
  ICSI,  
  2007-06-21.
* Fast pow approximation adapted from:  
  'A Fast, Compact Approximation of the Exponential Function'  
  Nicol N. Schraudolph;  
  Technical Report IDSIA-07-98,  
  <http://www.idsia.ch/>  
  1998-06-24.




Notices
-------

This project incorporates code headers from:


### OpenExr ###

Copyright (c) 2002, Industrial Light & Magic, a division of Lucas
Digital Ltd. LLC

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
*       Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
*       Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
*       Neither the name of Industrial Light & Magic nor the names of
its contributors may be used to endorse or promote products derived
from this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


### libpng ###

libpng version 1.2.8 - December 3, 2004  
Copyright (c) 1998-2004 Glenn Randers-Pehrson


### zlib ###

zlib version 1.2.3, July 18th, 2005  
(C) 1995-2004 Jean-loup Gailly and Mark Adler
