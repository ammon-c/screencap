//--------------------------------------------------------------------
//
// ScreenCapGDI.cpp
// Implementation of C++ class to capture screen images on a Windows
// computer using Windows GDI APIs.
//
// Reference:
//  * https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-bitblt
//  * https://learn.microsoft.com/en-us/windows/win32/gdi/memory-device-contexts
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

#include "ScreenCapGDI.h"
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>

//
// Begins a screen capture session.  Returns true if
// successful.
//
bool ScreenCaptureGDI::Startup()
{
    // Tell Windows our app is "DPI aware" so it doesn't give us
    // artificially scaled screen size values below. 
    ::SetProcessDPIAware();

    // Determine the dimensions of the frame buffer.
    m_width  = GetSystemMetrics(SM_CXSCREEN);
    m_height = GetSystemMetrics(SM_CYSCREEN);
    if (GetSystemMetrics(SM_CMONITORS) > 1)
    {
       m_width  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
       m_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    }

    // Create a memory display context that's compatible with the screen.
    HDC hdcScreen = GetDC(GetDesktopWindow());
    m_hdcMem = CreateCompatibleDC(hdcScreen);
    ReleaseDC(GetDesktopWindow(), hdcScreen);
    if (m_hdcMem == nullptr)
       return false;

    // Create a DIB section the same size as the screen.
    // This will be our in-memory frame buffer.
    BITMAPINFOHEADER hdr = {0};
    hdr.biSize = sizeof(hdr);
    hdr.biWidth = m_width;
    hdr.biHeight = m_height;
    hdr.biBitCount = 32;
    hdr.biPlanes = 1;
    m_dibSection = CreateDIBSection(reinterpret_cast<HDC>(m_hdcMem),
                         reinterpret_cast<BITMAPINFO *>(&hdr), DIB_RGB_COLORS,
                         reinterpret_cast<void **>(&m_dibBits), nullptr, 0);
    if (m_dibSection == nullptr || m_dibBits == nullptr)
    {
       DeleteDC(reinterpret_cast<HDC>(m_hdcMem));
       m_hdcMem = nullptr;
       return false;
    }

    m_depth = hdr.biBitCount;
    m_stride = m_width * hdr.biBitCount / 8;
    while (m_stride % 4)
       ++m_stride;

    // Select the DIB section we just created into the memory display context.
    // This will allow Windows to draw on the DIB *and* allows us direct access
    // to the pixels of the DIB.
    m_dibOld = static_cast<HBITMAP>(SelectObject(reinterpret_cast<HDC>(m_hdcMem), m_dibSection));

    GdiFlush();
    return true;
}

//
// Stops the screen capture session and releases any
// allocated resources.
//
void ScreenCaptureGDI::Shutdown()
{
   GdiFlush();
   if (m_dibOld)
      SelectObject(reinterpret_cast<HDC>(m_hdcMem), m_dibOld);
   if (m_hdcMem)
      DeleteDC(reinterpret_cast<HDC>(m_hdcMem));
   if (m_dibSection)
      DeleteObject(m_dibSection);

    m_dibOld = nullptr;
    m_hdcMem = nullptr;
    m_dibSection = nullptr;
    m_width = m_height = m_depth = m_stride = 0;
}

//
// Attempts to capture the next frame from the screen.
// The captured image is placed in an internal image
// buffer that can be accessed via GetCapturedFrame()
// (see below).  Returns true if successful.
//
bool ScreenCaptureGDI::CaptureFrame()
{
    if (m_hdcMem == nullptr || m_width == 0)
        return false; // Not initialized yet!

    // Copy pixels from the screen's display context to our
    // frame buffer.
    GdiFlush();
    HDC hdcScreen = GetDC(GetDesktopWindow());
    BitBlt(reinterpret_cast<HDC>(m_hdcMem), 0, 0, m_width, m_height,
       hdcScreen, 0, 0, SRCCOPY | CAPTUREBLT);
    ReleaseDC(GetDesktopWindow(), hdcScreen);

    // GDI captures the scanlines in bottom-to-top order, so
    // we need to reorder the scanlines in top-to-bottom order.
    std::vector<uint8_t> scanline(m_stride);
    uint8_t *scanline1 = m_dibBits;
    uint8_t *scanline2 = m_dibBits + (m_height - 1) * m_stride;
    for (unsigned y = 0; y < m_height / 2; y++)
    {
        memcpy(scanline.data(), scanline1, m_stride);
        memcpy(scanline1, scanline2, m_stride);
        memcpy(scanline2, scanline.data(), m_stride);

        scanline1 += m_stride;
        scanline2 -= m_stride;
    }

    return true;
}

