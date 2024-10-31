//
// encodetest.cpp
// A small program to test the VideoFileEncoder module.
// Attempts to encode a video of vertical blue bars
// moving horizontally across the screen.
//

#include "VideoFileEncoder.h"
#include <stdio.h>

int main()
{
    VideoFileEncoder enc(true, true);
    if (!enc.SetEncodingFormat(MFVideoFormat_H264))
    {
        printf("enc.SetEncodingFormat failed!\n");
        CoUninitialize();
        return -1;
    }
    if (!enc.Start(L"test.mp4", 640, 480, 30))
    {
        printf("enc.Start failed!\n");
        CoUninitialize();
        return -1;
    }

    // Send frames to the encoder.
    int64_t timestamp = 0;
    std::vector<uint8_t> frameBuffer(enc.GetWidth() * enc.GetHeight() * sizeof(uint32_t));
    for (DWORD i = 0; i < 500; ++i)
    {
        // Fill the frame with blue vertical bars
        // that move horizontally based on the
        // frame number.
        size_t numPixels = enc.GetWidth() * enc.GetHeight();
        for (size_t j = 0; j < numPixels; ++j)
        {
            frameBuffer[j * sizeof(uint32_t)] = ((i + j) & 0x7F) + 0x80;
        }

        if (!enc.AddFrame(frameBuffer.data(), false, timestamp))
        {
            break;
        }

        timestamp += enc.GetFrameDuration();
    }

    enc.Stop();

    return 0;
}

