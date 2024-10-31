//--------------------------------------------------------------------
//
// captest.cpp
// Simple program to test the ScreenCapture class.
// Attempts to capture up to 100 frames from the screen and saves
// them to .BMP files.
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

#include "ScreenCap.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <atlbase.h>
#include <dxgi1_2.h>
#include <d3d11.h>
#include <vector>

//
// Writes a 24-bit BGR or 32-bit BGRA image from memory to a
// Microsoft .BMP file on disk.  Returns true if successful.
//
static bool BmpWrite(const char *szPath, unsigned width, unsigned height,
    unsigned stride, unsigned bitsPerPixel, const void *pBits)
{
    // Check for bogus arguments.
    if (szPath == nullptr || szPath[0] == '\0' || width < 1 || height < 1 ||
        stride < width * 3 || (bitsPerPixel != 24 && bitsPerPixel != 32) ||
        pBits == nullptr)
    {
        return false;
    }

    // Open the output file.
    FILE *fp = nullptr;
    if (fopen_s(&fp, szPath, "wb") || fp == nullptr)
    {
        // Failed opening output file!
        return false;
    }

    unsigned outStride = width * bitsPerPixel / 8;
    while (outStride % 4)
        outStride++;

    // Build BITMAPINFOHEADER to write to file.
    BITMAPINFOHEADER stInfoHdr = {0};
    stInfoHdr.biSize = sizeof(stInfoHdr);
    stInfoHdr.biBitCount = static_cast<WORD>(bitsPerPixel);
    stInfoHdr.biWidth = width;
    stInfoHdr.biHeight = height;
    stInfoHdr.biPlanes = 1;
    stInfoHdr.biSizeImage = outStride * height;

    // Build BITMAPFILEHEADER structure.
    BITMAPFILEHEADER stFileHdr;
    memset(&stFileHdr, 0, sizeof(stFileHdr));
    stFileHdr.bfType = (WORD)'B' + 256 * (WORD)'M';
    stFileHdr.bfSize = sizeof(BITMAPFILEHEADER) + stInfoHdr.biSize + stInfoHdr.biSizeImage;
    stFileHdr.bfOffBits = sizeof(BITMAPFILEHEADER) + stInfoHdr.biSize;

    // Write the BITMAPFILEHEADER to the output file.
    if (fwrite(&stFileHdr, sizeof(stFileHdr), 1, fp) != 1)
    {
        // Write to output file failed!
        fclose(fp);
        _unlink(szPath);
        return false;
    }

    // Write the BITMAPINFOHEADER to the output file.
    if (fwrite(&stInfoHdr, stInfoHdr.biSize, 1, fp) != 1)
    {
        // Write to output file failed!
        fclose(fp);
        _unlink(szPath);
        return false;
    }

    // Write the bitmap bits one scanline at a time.
    const unsigned char *scanline = static_cast<const unsigned char *>(pBits) + stride * (height - 1);
    for (unsigned y = 0; y < height; y++)
    {
        if (fwrite(scanline, outStride, 1, fp) != 1)
        {
            // Write to output file failed!
            fclose(fp);
            _unlink(szPath);
            return false;
        }

        scanline -= stride;
    }

    fclose(fp);
    return true;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf(
            "Usage:\n"
            "    captest GDI    - Test capture using Windows GDI.\n"
            "    captest DX11   - Test capture using DirectX 11.\n"
            );
        return -1;
    }

    // Parse command line.
    ScreenCaptureMode mode = ScreenCaptureMode_Invalid;
    if (_stricmp(argv[1], "GDI") == 0)
    {
        printf("Selected GDI capture mode.\n");
        mode = ScreenCaptureMode_GDI;
    }
    else if (_stricmp(argv[1], "DX11") == 0)
    {
        printf("Selected DX11 capture mode.\n");
        mode = ScreenCaptureMode_DX11;
    }
    if (mode == ScreenCaptureMode_Invalid)
    {
        printf("Unrecognized capture mode '%s'\n", argv[1]);
        return -1;
    }

    ScreenCapture cap;
    if (!cap.Startup(mode))
    {
        printf("Startup failed!\n");
        return -1;
    }

    size_t numFrames = 0;
    uint64_t startTick = GetTickCount64();

    for (int iframe = 0; iframe < 100; iframe++)
    {
        // Capture a screen image.
        if (!cap.CaptureFrame() || cap.GetFrameWidth() < 1)
        {
            // No image was captured.
            // Keep trying.
            continue;
        }

        if (!numFrames)
        {
            printf("First frame width=%u, height=%u, stride=%u\n",
                cap.GetFrameWidth(), cap.GetFrameHeight(),
                cap.GetFrameStride());
        }

        numFrames++;

        // Write the screen image to a BMP file.
        char filename[256] = {0};
        snprintf(filename, sizeof(filename), "frame%d.bmp", iframe);

        printf("Writing %s, %u x %u x %u\n", filename,
            cap.GetFrameWidth(), cap.GetFrameHeight(), cap.GetFrameDepth());

        if (!BmpWrite(filename, cap.GetFrameWidth(), cap.GetFrameHeight(),
                      cap.GetFrameStride(), cap.GetFrameDepth(), cap.GetFrameBuffer()))
        {
            printf("Failed writing image to BMP file!\n");
            return -1;
        }
    }

    float seconds = static_cast<float>(GetTickCount64() - startTick) / 1000.0f;

    cap.Shutdown();

    // Show statistics.
    printf("Frames:  %zu\n", numFrames);
    printf("Time:    %.2f seconds\n", seconds);
    if (seconds > 0.0f)
        printf("FPS:     %.2f\n", numFrames / seconds);

    printf("OK\n");
    return 0;
}
