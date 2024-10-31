//--------------------------------------------------------------------
//
// capenctest.cpp
// Simple program to test capturing the screen and encoding the
// frames to a .mp4 video file. 
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
#include "VideoFileEncoder.h"
#include <vector>
#if 0 // TODO
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <atlbase.h>
#include <dxgi1_2.h>
#include <d3d11.h>
#endif

const wchar_t *outputFilename = L"test.mp4";
unsigned framesPerSecond = 30;

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf(
            "Usage:\n"
            "    capenctest GDI    - Test capture using Windows GDI.\n"
            "    capenctest DX11   - Test capture using DirectX 11.\n"
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

    VideoFileEncoder encoder(true, true);
    if (!encoder.SetEncodingFormat(MFVideoFormat_H264))
    {
        printf("Failed initializing encoder!\n");
        return -1;
    }

    size_t numFrames = 0;
    uint64_t startTick = GetTickCount64();
    int64_t timestamp = 0;

    for (int iframe = 0; iframe < 100; iframe++)
    {
        // Capture a screen image.
        if (!cap.CaptureFrame() || cap.GetFrameWidth() < 1)
        {
            // No image was captured.
            // Keep trying.
            continue;
        }

        // When we get the first frame, initialize the
        // video encoder with the frame size.
        if (!numFrames)
        {
            printf("Start encoder, width=%u, height=%u, stride=%u, fps=%u\n",
                cap.GetFrameWidth(), cap.GetFrameHeight(),
                cap.GetFrameStride(), framesPerSecond);
            if (!encoder.Start(outputFilename,
                cap.GetFrameWidth(), cap.GetFrameHeight(),
                framesPerSecond))
            {
                printf("Failed starting encoder!\n");
                return -1;
            }
        }

        // Send the captured frame image to the encoder.
        if (!encoder.AddFrame(cap.GetFrameBuffer(), true, timestamp))
        {
            printf("Failed encoding frame!\n");
            return -1;
        }

        numFrames++;
        timestamp += encoder.GetFrameDuration();
    }

    float seconds = static_cast<float>(GetTickCount64() - startTick) / 1000.0f;

    cap.Shutdown();
    if (!encoder.Stop())
    {
        printf("Failed writing video file!\n");
        return -1;
    }

    // Show statistics.
    printf("Frames:  %zu\n", numFrames);
    printf("Time:    %.2f seconds\n", seconds);
    if (seconds > 0.0f)
        printf("FPS:     %.2f\n", numFrames / seconds);

    printf("OK\n");
    return 0;
}
