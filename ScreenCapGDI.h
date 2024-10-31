//--------------------------------------------------------------------
//
// ScreenCapGDI.h
// Header file of C++ class to capture screen images on a Windows
// computer using Windows GDI APIs.
//
//--------------------------------------------------------------------
// (C) Copyright 1996,2019,2024 by Ammon R. Campbell.
//
// I wrote this code for use in my own educational and experimental
// programs, but you may also freely use it in yours as long as you
// abide by the following terms and conditions:
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials
//     provided with the distribution.
//   * The name(s) of the author(s) and contributors (if any) may not
//     be used to endorse or promote products derived from this
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.  IN OTHER WORDS, USE AT YOUR OWN RISK, NOT OURS.  
//--------------------------------------------------------------------

#pragma once
#include <cstdint>

//---------------------------------------------------------------
// A class to grab screenshots using Windows GDI.
//---------------------------------------------------------------
class ScreenCaptureGDI
{
public:
    ScreenCaptureGDI()  { }
    ~ScreenCaptureGDI() { Shutdown(); }

    //
    // Begins a screen capture session.  Returns true if
    // successful.
    //
    bool Startup();

    //
    // Stops the screen capture session and releases any
    // allocated resources.
    //
    void Shutdown();

    //
    // Attempts to capture the next frame from the screen.
    // The captured image is placed in an internal image
    // buffer that can be accessed via GetCapturedFrame()
    // (see below).  Returns true if successful.
    //
    bool CaptureFrame();

    // Retrieve the dimensions and format of the captured
    // frame image.
    unsigned GetFrameWidth()  const { return m_width;  }
    unsigned GetFrameHeight() const { return m_height; }
    unsigned GetFrameDepth()  const { return m_depth;  }
    unsigned GetFrameStride() const { return m_stride; }

    //
    // Returns a pointer to the frame buffer pixels of
    // the captured image.
    //
    const uint8_t *GetFrameBuffer() const { return m_dibBits; }
    const uint8_t *GetFrameBufferScanlinePtr(unsigned y) const { if (!m_dibBits) return nullptr; return m_dibBits + (m_stride * y); }
    const uint8_t *GetFrameBufferPixelPtr(unsigned y, unsigned x) const { if (!m_dibBits) return nullptr; return m_dibBits + (m_stride * y) + (x * m_depth / 8); }

private:
    unsigned       m_width = 0;               // Width of frame in pixels.
    unsigned       m_height = 0;              // Height of frame in pixels.
    unsigned       m_depth = 0;               // Color depth of frame in bits per pixel.
    int            m_stride = 0;              // Offset between first byte of each scanline in DIB section.
    // Note these are void pointer so we can avoid having to include windows.h in here.
    void *         m_hdcMem = nullptr;        // Handle to memory display context that we created.
    void *         m_dibSection = nullptr;    // DIB section selected into memory display context.
    void *         m_dibOld = nullptr;        // Original DIB section from display context so we can restore it later.
    uint8_t *      m_dibBits = nullptr;       // Pointer to the raw pixel array of m_dibSection.
};

