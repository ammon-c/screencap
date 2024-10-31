//--------------------------------------------------------------------
//
// VideoFileEncoder.h
// Header file for C++ class to create a video file from a
// series of bitmap image frames.  Uses Microsoft Media
// Foundation to do most of the work.
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <cstdint>
#include <vector>

class VideoFileEncoder
{
public:
    //
    // When constructing, two flags must be passed:
    //
    //  doMFStartup : If true, MFStartup will be called
    //     when the object is created and MFShutdown will be
    //     called when the object is destroyed.  This flag
    //     should be false if the calling application already
    //     calls MFStartup/MFShutdown.
    //
    //  doCoInitialize : If true, CoInitialize will be
    //     called when the object is created and CoUninitialize
    //     will be called when the object is destroyed.  This
    //     flag should be false if the calling application
    //     already calls CoInitialize/CoUninitialize.
    //
    VideoFileEncoder(bool doMFStartup, bool doCoInitialize);
    VideoFileEncoder() = delete;

    ~VideoFileEncoder();

    //
    // Specify the format to which the video frames will be encoded.
    //  Use MFVideoFormat_H264 for .mp4, or MFVideoFormat_WMV3 for
    //  .wmv.  See Windows API docs for other possible formats.
    //
    bool SetEncodingFormat(GUID fmt);

    //
    // Start encoding video frames to the specified file in the
    // specified frame format.
    //
    bool Start(
            const wchar_t *filename,
            uint32_t width,
            uint32_t height,
            uint32_t fps);

    //
    // Finish encoding video frames to the output file.
    // Returns true if successful.
    //
    bool Stop();

    //
    // Adds the next frame to the video stream.
    // The data pointed to by 'pixels' must be in the format
    // specified to the Start() member.  Pixels are assumed
    // to be 32 bits each (BGRA or BGRX).
    //
    // If flipY is true, the scanlines in 'pixels' are in
    // bottom-to-top order rather than top-to-bottom order.
    //
    // Returns true if successful.
    //
    bool AddFrame(const void *pixels, bool flipY, uint64_t timestamp);

    uint32_t GetWidth()             const { return m_width; }
    uint32_t GetHeight()            const { return m_height; }
    uint32_t GetFrameDuration()     const { return m_frameDuration; }
    uint32_t GetBitRate()           const { return m_bitRate; }
    const uint8_t *GetFrameBuffer() const { return m_pixels.data(); }

private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_fps = 0;
    uint32_t m_frameDuration = 0;
    uint32_t m_bitRate = 0;
    GUID     m_encodingFormat = MFVideoFormat_H264;
    GUID     m_inputFormat = MFVideoFormat_RGB32;
    std::vector<uint8_t> m_pixels;
    IMFSinkWriter *m_pSinkWriter = nullptr;
    uint32_t m_stream = 0;
    bool     m_doMFStartup = false;
    bool     m_doCoInitialize = false;

    bool SetFrameFormat(uint32_t width, uint32_t height, uint32_t fps);
    HRESULT InitializeSinkWriter(IMFSinkWriter **ppWriter,
        DWORD *pStreamIndex, const wchar_t *filename);
    HRESULT WriteFrame(IMFSinkWriter *pWriter, DWORD streamIndex, 
        const LONGLONG& timestamp);
};

