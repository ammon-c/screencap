//--------------------------------------------------------------------
//
// ScreenCap.h
// Header file of C++ class to capture screen images on a Windows
// computer.  This ScreenCapture class is a thin wrapper around the
// ScreenCaptureGDI and ScreenCaptureDX11 classes.
//
// ToDo:
//  * Refactor this so ScreenCaptureGDI and ScreenCaptureDX11 are
//    child classes of ScreenCapture.
//
//--------------------------------------------------------------------
// (C) Copyright 2019,2024 by Ammon R. Campbell.
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
#include "ScreenCapGDI.h"
#include "ScreenCapDX11.h"

enum ScreenCaptureMode
{
    ScreenCaptureMode_Invalid = 0,
    ScreenCaptureMode_GDI     = 1,  // GDI capture is slow but very reliable.
    ScreenCaptureMode_DX11    = 2   // DX11 capture is fast but depends on DirectX drivers.
};

//
// This class manages a screen capture session.
// 
class ScreenCapture
{
public:
    ScreenCapture()  { }
    ~ScreenCapture() { Shutdown(); }

    //
    // Begins a screen capture session.  Returns true if
    // successful.
    //
    bool Startup(ScreenCaptureMode mode)
    {
        // Shut down the previous capture session first.
        if (m_mode != ScreenCaptureMode_Invalid)
            Shutdown();

        m_mode = mode;
        if (mode == ScreenCaptureMode_GDI)
        {
            m_capgdi = new ScreenCaptureGDI;
            return m_capgdi->Startup();
        }
        else if (mode == ScreenCaptureMode_DX11)
        {
            m_capdx11 = new ScreenCaptureDX11;
            return m_capdx11->Startup();
        }

        return false;
    }

    //
    // Stops the screen capture session and releases any
    // allocated resources.
    //
    void Shutdown()
    {
        if (m_capgdi)
            delete m_capgdi;

        if (m_capdx11)
            delete m_capdx11;

        m_capgdi = nullptr;
        m_capdx11 = nullptr;
        m_mode = ScreenCaptureMode_Invalid;
    }

    //
    // Attempts to capture the next frame from the screen.
    // The captured image is placed in an internal image
    // buffer that can be accessed via GetCapturedFrame()
    // (see below).  Returns true if successful.
    //
    // May return false because of an error or, more
    // typically, because no pixels have changed on
    // the screen since the last frame was captured,
    // so no new frame is available yet.
    //
    bool CaptureFrame()
    {
        if (m_capgdi)
            return m_capgdi->CaptureFrame();
        if (m_capdx11)
            return m_capdx11->CaptureFrame();

        return false;
    }

    //
    // Returns the current screen capture mode (GDI or DX11).
    //
    ScreenCaptureMode GetCaptureMode() const { return m_mode; }

    //
    // Return size and format of the frame buffer that
    // contains the captured image.
    //
    unsigned GetFrameWidth()  const
    {
        if (m_capgdi)
            return m_capgdi->GetFrameWidth();
        if (m_capdx11)
            return m_capdx11->GetFrameWidth();
        return 0;
    }
    unsigned GetFrameHeight() const
    {
        if (m_capgdi)
            return m_capgdi->GetFrameHeight();
        if (m_capdx11)
            return m_capdx11->GetFrameHeight();
        return 0;
    }
    unsigned GetFrameDepth()  const
    {
        if (m_capgdi)
            return m_capgdi->GetFrameDepth();
        if (m_capdx11)
            return m_capdx11->GetFrameDepth();
        return 0;
    }
    unsigned GetFrameStride() const
    {
        if (m_capgdi)
            return m_capgdi->GetFrameStride();
        if (m_capdx11)
            return m_capdx11->GetFrameStride();
        return 0;
    }

    //
    // Returns a pointer to the frame buffer pixels of
    // the captured image.
    //
    const uint8_t *GetFrameBuffer() const
    {
        if (m_capgdi)
            return m_capgdi->GetFrameBuffer();
        if (m_capdx11)
            return m_capdx11->GetFrameBuffer();
        return nullptr;
    }
    const uint8_t *GetFrameBufferScanlinePtr(unsigned y) const
    {
        return GetFrameBuffer() + (GetFrameStride() * y);
    }
    const uint8_t *GetFrameBufferPixelPtr(unsigned y, unsigned x) const
    {
        return GetFrameBuffer() + (GetFrameStride() * y) + (x * GetFrameDepth() / 8);
    }

private:
    ScreenCaptureMode m_mode = ScreenCaptureMode_Invalid;
    class ScreenCaptureGDI  *m_capgdi  = nullptr;
    class ScreenCaptureDX11 *m_capdx11 = nullptr;
};

