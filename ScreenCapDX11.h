//--------------------------------------------------------------------
//
// ScreenCapDX11.h
// Header file of C++ class to capture screen images on a Windows
// computer using the output duplication features of DirectX11. 
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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <atlbase.h>
#include <dxgi1_2.h>
#include <d3d11.h>
#include <vector>
#include <cstdint>

//
// This class manages a screen capture session, using
// the output duplication features in DirectX 11.
// 
class ScreenCaptureDX11
{
public:
    ScreenCaptureDX11()  { }
    ~ScreenCaptureDX11() { Shutdown(); }

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
    // May return false because of an error or, more
    // typically, because no pixels have changed on
    // the screen since the last frame was captured,
    // so no new frame is available yet.
    //
    bool CaptureFrame();

    //
    // Return size and format of the frame buffer that
    // contains the captured image.
    //
    unsigned GetFrameWidth()  const { return m_frameWidth;  }
    unsigned GetFrameHeight() const { return m_frameHeight; }
    unsigned GetFrameDepth()  const { return m_frameDepth;  }
    unsigned GetFrameStride() const { return m_frameStride; }

    //
    // Returns a pointer to the frame buffer pixels of
    // the captured image.
    //
    const uint8_t *GetFrameBuffer() const { return m_frameBuffer.data(); }
    const uint8_t *GetFrameBufferScanlinePtr(unsigned y) const { return m_frameBuffer.data() + (m_frameStride * y); }
    const uint8_t *GetFrameBufferPixelPtr(unsigned y, unsigned x) const { return m_frameBuffer.data() + (m_frameStride * y) + (x * m_frameDepth / 8); }

private:
    CComPtr<ID3D11Device>           m_device;
    CComPtr<ID3D11DeviceContext>    m_deviceContext;
    CComPtr<ID3D11Texture2D>        m_stagingTexture;
    CComPtr<IDXGIOutputDuplication> m_outputDuplication;

    // The pixels of the captured image.
    std::vector<uint8_t> m_frameBuffer;

    // Size and format of the captured frame buffer image.
    unsigned m_frameWidth  = 0;
    unsigned m_frameHeight = 0;
    unsigned m_frameDepth  = 0; // Pixel depth in bits-per-pixel.
    unsigned m_frameStride = 0; // Number of bytes between scanlines.

    bool InitializeDevice();
    bool StartOutputDuplication(
            ID3D11Device *pdevice,
            CComPtr<IDXGIOutputDuplication> &cOutputDuplication);
    bool CreateStagingTexture(
            ID3D11Device* pdevice,
            const DXGI_OUTDUPL_DESC &duplDesc,
            CComPtr<ID3D11Texture2D> &stagingTexture);
    bool CopyStagingTextureToMemory(
            ID3D11DeviceContext *pDeviceContext,
            CComPtr<ID3D11Texture2D> &stagingTexture);
    bool AcquireNextFrame(
            IDXGIOutputDuplication* cOutputDuplication,
            CComPtr<ID3D11Texture2D> &acquiredDesktopImage);
};

