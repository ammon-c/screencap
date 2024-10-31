## README for ScreenCap - Screen capture code for Windows (C++)

**What Is This?**

*ScreenCap* is a collection of C++ code for capturing screen
images and encoding video files on a Windows computer.  

I wrote this software for my own use.  You may also use it, but
at your own risk.  Refer the [license](#tagLicense) section at
the end of this document for important legal notices.  

**Platform:**

* *ScreenCap* compiles with Microsoft Visual Studio 2022, and
runs on Windows 10 or 11 (64-bit).  

---
<a name="tagBuild"></a>

### Build

* It is assumed that Microsoft Visual Studio 2022 is installed,
and the Microsoft tools are configured to be usable from the
Windows command prompt.  

* At a Windows command prompt, **CD** to the directory where you
have placed the *ScreenCap* files.  

* Then run **NMAKE** at the command prompt.

---
<a name="tagTests"></a>

### Tests

After the software has been built, there are several tests that
can be run to confirm that the software is working as intended. 

* **captest.exe** :  This program does a brief test of the
*ScreenCap* module, capturing up to 100 frames from the screen
and writing the frame images to .BMP files.  On the command
line, the keyword "GDI" or "DX11" must be given to tell the test
program which capture mode to test.  After the test has finished
running, you may examine the .BMP files that were generated to
confirm that the test behaved as expected.  

* **encodetest.exe** :  This program does a brief test of the
*VideoFileEncoder* module, generating a series of video frames
of vertical blue bars that move horizontally across the screen,
and writing the encoded video to a "test.mp4" file.  After the
test has finished running, you may examine the "test.mp4" file
to confirm the test behaved as expected.  

* **capenctest.exe** :  This program does a brief test of both
the ScreenCap module and the VideoFileEncoder module, capturing
a series of up to 100 frames from the screen and writing the
frames to a "test.mp4" video file.  On the command line, the
keyword "GDI" or "DX11" must be given to tell the test program
which capture mode to test.  After the test has finished
running, you may exakine the "test.mp4" file to confirm the test
behaved as expected.  

---
<a name="tagSource"></a>

### Source Code

Some of the important source code files are:

* **ScreenCap.h** :  Include this C++ header file into any
program that wishes to use the ScreenCap module.  This module
supports capturing images from the PC's screen using either
DirectX 11 or GDI APIs.  

* **VideoFileEncoder.h** :  Include this C++ header file into
any program that wishes to use the VideoFileEncoder module. 
This module supports writing a series of bitmap images to a
video file in several file formats (.mp4, .wmv, etc.), using the
Microsoft Media Framework.  

* **ScreenCapDX11.cpp** and **ScreenCapDX11.h** :  C++ code for
capturing screen images using DirectX 11.  

* **ScreenCapGDI.cpp** and **ScreenCapGDI.h** :  C++ code for
capturing screen images using GDI APIs.  

* **VideoFileEncoder.cpp** :  C++ code for encoding images to a
video file using Microsoft Media Framework.  

* **captest.cpp** :  A small C++ program for testing the
ScreenCap module.  

* **encodetest.cpp** :  A small C++ program for testing the
VideoFileEncoder module.  

* **capenctest.cpp** :  A small C++ program for testing both the
ScreenCap and VideoFileEncoder modules together.  

* **makefile** :  An NMAKE build script for compiling the C++
source code into binaries.  

---
<a name="tagToDo"></a>

### To Do / Wish List

* Add support for capturing and encoding audio along with the
video.  

* Add support for capturing video to a memory buffer instead
of a video file.

* Write a variant of this software that works on Linux and/or
macOS, possibly using SDL and/or X11 APIs. 

---
<a name="tagLicense"></a>

### License

```
(C) Copyright 2019,2024 by Ammon R. Campbell.

I wrote this code for use in my own educational and experimental
programs, but you may also freely use it in yours as long as you
abide by the following terms and conditions:

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials
    provided with the distribution.
  * The name(s) of the author(s) and contributors (if any) may not
    be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.  IN OTHER WORDS, USE AT YOUR OWN RISK, NOT OURS.  
```

-*- end -*-
