#
# NMAKE build script to compile the screen capture and video
# file encoding software.
#

.SUFFIXES: .cpp

.cpp.obj:
    cl -nologo -c -W4 -WX -EHsc -Zi $<

all: captest.exe encodetest.exe capenctest.exe

captest.exe: captest.obj ScreenCapDX11.obj ScreenCapGDI.obj
    link /NOLOGO /DEBUG /OUT:$@ $** d3d11.lib gdi32.lib

capenctest.exe: capenctest.obj ScreenCapDX11.obj ScreenCapGDI.obj VideoFileEncoder.obj
    link /NOLOGO /DEBUG /OUT:$@ $** d3d11.lib gdi32.lib

encodetest.exe: encodetest.obj VideoFileEncoder.obj
    link /NOLOGO /DEBUG /OUT:$@ $** d3d11.lib gdi32.lib

captest.obj:           captest.cpp ScreenCap.h ScreenCapDX11.h ScreenCapGDI.h
capenctest.obj:        capenctest.cpp ScreenCap.h ScreenCapDX11.h ScreenCapGDI.h VideoFileEncoder.h
encodetest.obj:        encodetest.cpp VideoFileEncoder.h
ScreenCapDX11.obj:     ScreenCapDX11.cpp ScreenCapDX11.h
ScreenCapGDI.obj:      ScreenCapGDI.cpp  ScreenCapGDI.h
VideoFileEncoder.obj:  VideoFileEncoder.cpp VideoFileEncoder.h

clean:
    if exist *.obj del *.obj
    if exist *.exe del *.exe
    if exist *.pdb del *.pdb
    if exist *.ilk del *.ilk
    if exist frame*.bmp del frame*.bmp
    if exist test*.mp4 del test*.mp4

