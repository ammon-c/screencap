//--------------------------------------------------------------------
//
// VideoFileEncoder.cpp
// Implementation of C++ class to create a video file from
// a series of bitmap image frames.  Uses Microsoft Media
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

#include "VideoFileEncoder.h"

// Auto-link to the MMF libaries.
#pragma comment(lib, "ole32")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfplat")

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

//----------------------------------------------------------
// Private members
//----------------------------------------------------------

bool VideoFileEncoder::SetFrameFormat(
    uint32_t width,
    uint32_t height,
    uint32_t fps
    )
{
    if (width < 1 || height < 1 || fps < 1)
        return false;

    m_width = width;
    m_height = height;
    m_fps = fps;
    m_frameDuration = 10 * 1000 * 1000 / m_fps;
    m_bitRate = static_cast<uint32_t>(width * height * 2.5);
    m_pixels.resize(width * height * sizeof(uint32_t));

    return true;
}

HRESULT VideoFileEncoder::InitializeSinkWriter(
    IMFSinkWriter **ppWriter,
    DWORD *pStreamIndex,
    const wchar_t *filename
    )
{
    // This code was adapted from the example in Microsoft's documentation.

    *ppWriter = nullptr;
    *pStreamIndex = 0;

    IMFSinkWriter   *pSinkWriter = nullptr;
    IMFMediaType    *pMediaTypeOut = nullptr;   
    IMFMediaType    *pMediaTypeIn = nullptr;   
    DWORD           streamIndex = 0;

    HRESULT hr = MFCreateSinkWriterFromURL(filename, nullptr, nullptr, &pSinkWriter);

    // Set the output media type.
    if (SUCCEEDED(hr))
        hr = MFCreateMediaType(&pMediaTypeOut);   
    if (SUCCEEDED(hr))
        hr = pMediaTypeOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    if (SUCCEEDED(hr))
        hr = pMediaTypeOut->SetGUID(MF_MT_SUBTYPE, m_encodingFormat);
    if (SUCCEEDED(hr))
        hr = pMediaTypeOut->SetUINT32(MF_MT_AVG_BITRATE, m_bitRate);
    if (SUCCEEDED(hr))
        hr = pMediaTypeOut->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    if (SUCCEEDED(hr))
        hr = MFSetAttributeSize(pMediaTypeOut, MF_MT_FRAME_SIZE, m_width, m_height);
    if (SUCCEEDED(hr))
        hr = MFSetAttributeRatio(pMediaTypeOut, MF_MT_FRAME_RATE, m_fps, 1);
    if (SUCCEEDED(hr))
        hr = MFSetAttributeRatio(pMediaTypeOut, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    if (SUCCEEDED(hr))
        hr = pSinkWriter->AddStream(pMediaTypeOut, &streamIndex);   

    // Set the input media type.
    if (SUCCEEDED(hr))
        hr = MFCreateMediaType(&pMediaTypeIn);   
    if (SUCCEEDED(hr))
        hr = pMediaTypeIn->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    if (SUCCEEDED(hr))
        hr = pMediaTypeIn->SetGUID(MF_MT_SUBTYPE, m_inputFormat);
    if (SUCCEEDED(hr))
        hr = pMediaTypeIn->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    if (SUCCEEDED(hr))
        hr = MFSetAttributeSize(pMediaTypeIn, MF_MT_FRAME_SIZE, m_width, m_height);
    if (SUCCEEDED(hr))
        hr = MFSetAttributeRatio(pMediaTypeIn, MF_MT_FRAME_RATE, m_fps, 1);   
    if (SUCCEEDED(hr))
        hr = MFSetAttributeRatio(pMediaTypeIn, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);   
    if (SUCCEEDED(hr))
        hr = pSinkWriter->SetInputMediaType(streamIndex, pMediaTypeIn, nullptr);   

    if (SUCCEEDED(hr))
        hr = pSinkWriter->BeginWriting();

    if (SUCCEEDED(hr))
    {
        *ppWriter = pSinkWriter;
        (*ppWriter)->AddRef();
        *pStreamIndex = streamIndex;
    }

    SafeRelease(&pSinkWriter);
    SafeRelease(&pMediaTypeOut);
    SafeRelease(&pMediaTypeIn);
    return hr;
}

HRESULT VideoFileEncoder::WriteFrame(
    IMFSinkWriter *pWriter, 
    DWORD streamIndex, 
    const LONGLONG& timestamp
    )
{
    IMFSample *pSample = nullptr;
    IMFMediaBuffer *pBuffer = nullptr;

    const LONG cbWidth = sizeof(uint32_t) * m_width;
    const DWORD cbBuffer = cbWidth * m_height;

    BYTE *pData = nullptr;

    HRESULT hr = MFCreateMemoryBuffer(cbBuffer, &pBuffer);

    if (SUCCEEDED(hr))
        hr = pBuffer->Lock(&pData, nullptr, nullptr);

    if (SUCCEEDED(hr))
    {
        hr = MFCopyImage(
                pData,            // Dest buffer
                cbWidth,          // Dest stride
                m_pixels.data(),  // Src buffer
                cbWidth,          // Src stride
                cbWidth,          // Image width in bytes (not pixels!)
                m_height          // Image height in pixels
            );
    }

    if (pBuffer)
        pBuffer->Unlock();

    if (SUCCEEDED(hr))
        hr = pBuffer->SetCurrentLength(cbBuffer);
    if (SUCCEEDED(hr))
        hr = MFCreateSample(&pSample);
    if (SUCCEEDED(hr))
        hr = pSample->AddBuffer(pBuffer);
    if (SUCCEEDED(hr))
        hr = pSample->SetSampleTime(timestamp);
    if (SUCCEEDED(hr))
        hr = pSample->SetSampleDuration(m_frameDuration);
    if (SUCCEEDED(hr))
        hr = pWriter->WriteSample(streamIndex, pSample);

    SafeRelease(&pSample);
    SafeRelease(&pBuffer);
    return hr;
}

//----------------------------------------------------------
// Public members
//----------------------------------------------------------

VideoFileEncoder::VideoFileEncoder(bool doMFStartup, bool doCoInitialize) :
    m_doMFStartup(doMFStartup), m_doCoInitialize(doCoInitialize)
{
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    MFStartup(MF_VERSION);
}

VideoFileEncoder::~VideoFileEncoder()
{
    Stop();
    if (m_doMFStartup)
        MFShutdown();
    if (m_doCoInitialize)
        CoUninitialize();
}

//
// Specify the format to which the video frames will be encoded.
//
bool VideoFileEncoder::SetEncodingFormat(GUID fmt)
{
    m_encodingFormat = fmt;
    return true;
}

//
// Start encoding video frames to the specified file in the
// specified frame format.
//
bool VideoFileEncoder::Start(const wchar_t *filename, uint32_t width, uint32_t height, uint32_t fps)
{
    if (m_pSinkWriter)
        Stop();

    // TODO: mp4/h264 format must have even width and height
    // parameters AND must be no larger than 1920x1080.
    // Need to check these!

    if (!SetFrameFormat(width, height, fps))
        return false;

    m_pSinkWriter = nullptr;
    m_stream = 0;
    HRESULT hr = InitializeSinkWriter(&m_pSinkWriter, reinterpret_cast<DWORD *>(&m_stream), filename);
    if (!SUCCEEDED(hr))
        return false;

    return true;
}

//
// Finish encoding video frames to the output file.
//
bool VideoFileEncoder::Stop()
{
    if (!m_pSinkWriter)
        return false;

    HRESULT hr = m_pSinkWriter->Finalize();
    SafeRelease(&m_pSinkWriter);

    return SUCCEEDED(hr);
}

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
bool VideoFileEncoder::AddFrame(const void *pixels, bool flipY, uint64_t timestamp)
{
    if (!pixels || m_pixels.empty())
        return false;

    memcpy(m_pixels.data(), pixels, m_width * m_height * sizeof(uint32_t));

    if (flipY)
    {
        unsigned stride = m_width * sizeof(uint32_t);
        std::vector<uint8_t> scanline(stride);
        uint8_t *scanline1 = m_pixels.data();
        uint8_t *scanline2 = m_pixels.data() + (m_height - 1) * stride;
        for (unsigned y = 0; y < m_height / 2; y++)
        {
            memcpy(scanline.data(), scanline1, stride);
            memcpy(scanline1, scanline2, stride);
            memcpy(scanline2, scanline.data(), stride);
    
            scanline1 += stride;
            scanline2 -= stride;
        }
    }

    HRESULT hr = WriteFrame(m_pSinkWriter, m_stream, timestamp);
    return SUCCEEDED(hr);
}

