//--------------------------------------------------------------------
//
// ScreenCapDX11.cpp
// C++ implementation of class to capture screen images on a Windows
// computer using the output duplication features of DirectX11. 
//
// Reference:
//  * https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/desktop-dup-api
//  * https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/DXGIDesktopDuplication
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

#include "ScreenCapDX11.h"

#pragma comment(lib, "D3D11.lib")

//--------------------------------------------------------------------
// Local helpers
//--------------------------------------------------------------------

// Returns true if the given pixel format is 32-bit BGRA or BGRX.
static bool IsFormat32bit(int fmt)
{
    if (fmt != DXGI_FORMAT_B8G8R8A8_UNORM       &&
        fmt != DXGI_FORMAT_B8G8R8X8_UNORM       &&
        fmt != DXGI_FORMAT_B8G8R8A8_TYPELESS    &&
        fmt != DXGI_FORMAT_B8G8R8A8_UNORM_SRGB  &&
        fmt != DXGI_FORMAT_B8G8R8X8_TYPELESS    &&
        fmt != DXGI_FORMAT_B8G8R8X8_UNORM_SRGB)
        return false;

    return true;
}

//--------------------------------------------------------------------
// Public members
//--------------------------------------------------------------------

//
// Begins a screen capture session.  Returns true if successful.
//
bool ScreenCaptureDX11::Startup()
{
    m_frameBuffer.clear();
    m_frameWidth = m_frameHeight = m_frameDepth = m_frameStride = 0;

    if (!InitializeDevice())
        return false;

    if (!StartOutputDuplication(m_device, m_outputDuplication))
        return false;

    return true;
}

//
// Stops the screen capture session and releases any
// allocated resources.
//
void ScreenCaptureDX11::Shutdown()
{
    if (m_stagingTexture)
        m_stagingTexture.Release();

    if (m_outputDuplication)
        m_outputDuplication.Release();

    if (m_device)
        m_device.Release();

    if (m_deviceContext)
        m_deviceContext.Release();

    m_frameBuffer.clear();
}

//
// Attempts to capture the next frame from the screen.
// The captured image is placed in an internal frame
// buffer that can be accessed via GetCapturedFrame().
// Returns true if successful.
//
// May return false because of an error or, more
// typically, because no pixels have changed on
// the screen since the last frame was captured,
// so no new frame is available yet.
//
bool ScreenCaptureDX11::CaptureFrame()
{
    // Assume we won't capture an image.
    m_frameWidth = m_frameHeight = m_frameStride = m_frameDepth = 0;

    if (!m_device || !m_deviceContext || !m_outputDuplication)
        return false; // Not initialized yet!

    // Attempt to capture a new screen image.
    CComPtr<ID3D11Texture2D> cacquiredDesktopImage;
    if (!AcquireNextFrame(m_outputDuplication, cacquiredDesktopImage) ||
        !cacquiredDesktopImage)
    {
        return false;
    }

    // Make sure the captured image is 32-bit.
    DXGI_OUTDUPL_DESC desc;
    m_outputDuplication->GetDesc(&desc);
    if (!IsFormat32bit(desc.ModeDesc.Format))
    {
        // Incompatible image format!
        return false;
    }

    // Copy image from the captured texture to a staging texture,
    // and then from the staging texture to our image buffer.
    if (!CreateStagingTexture(m_device, desc, m_stagingTexture))
    {
        m_outputDuplication->ReleaseFrame();
        return false;
    }
    m_deviceContext->CopyResource(m_stagingTexture, cacquiredDesktopImage);
    bool ok = CopyStagingTextureToMemory(m_deviceContext, m_stagingTexture);
    m_stagingTexture.Release();

    m_outputDuplication->ReleaseFrame();
    return ok;
}

//--------------------------------------------------------------------
// Private members
//--------------------------------------------------------------------

//
// Initializes a DirectX 11 device so we can use it.
// Populates m_device and m_deviceContext, and returns
// true if successful.
//
bool ScreenCaptureDX11::InitializeDevice()
{
    // We will try the driver types in this order.
    static const D3D_DRIVER_TYPE drivers[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE
    };

    // We will try the feature support levels in this order.
    static const D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_1
    };

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = E_FAIL;

    for (const auto &driver : drivers)
    {
        hr = D3D11CreateDevice(nullptr, driver, nullptr, 0,
                featureLevels, static_cast<UINT>(_countof(featureLevels)),
                D3D11_SDK_VERSION, &m_device, &featureLevel,
                &m_deviceContext);
        if (SUCCEEDED(hr) && m_device && m_deviceContext)
        {
            // Success.
            return true;
        }

        if (m_device)
            m_device.Release();
        if (m_deviceContext)
            m_deviceContext.Release();
    }

    // Failed on all drivers!
    return false;
}

//
// Copies the image from a staging texture to our internal
// frame buffer pixel array.  Populates the m_frameXXX members
// and returns true if successful.
//
bool ScreenCaptureDX11::CopyStagingTextureToMemory(
    ID3D11DeviceContext *pDeviceContext,
    CComPtr<ID3D11Texture2D> &stagingTexture
    )
{
    if (!pDeviceContext)
        return false;

    D3D11_TEXTURE2D_DESC desc;
    stagingTexture->GetDesc(&desc);

    // Lock the staging texture so we can access its pixel data.
    D3D11_MAPPED_SUBRESOURCE res;
    const auto hr = pDeviceContext->Map(
        stagingTexture,
        D3D11CalcSubresource(0, 0, 0),
        D3D11_MAP_READ,
        0,
        &res
    );
    if (FAILED(hr))
        return false;

    // Copy the texture's pixel data into our image buffer.
    m_frameWidth     = static_cast<int>(desc.Width);
    m_frameHeight    = static_cast<int>(desc.Height);
    m_frameStride    = res.RowPitch;
    m_frameDepth     = 32;
    m_frameBuffer.resize(m_frameStride * m_frameHeight);
    memcpy(m_frameBuffer.data(), res.pData, m_frameBuffer.size());

    pDeviceContext->Unmap(stagingTexture, 0);

    return true;
}

//
// Creates a staging texture compatible with the given
// texture format description.  Populates 'stagingTexture'
// and returns true if successful.
//
bool ScreenCaptureDX11::CreateStagingTexture(
    ID3D11Device *pdevice,
    const DXGI_OUTDUPL_DESC &ddesc,
    CComPtr<ID3D11Texture2D> &stagingTexture
    )
{
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width              = ddesc.ModeDesc.Width;
    desc.Height             = ddesc.ModeDesc.Height;
    desc.Format             = ddesc.ModeDesc.Format;
    desc.ArraySize          = 1;
    desc.BindFlags          = 0;
    desc.MiscFlags          = 0;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.MipLevels          = 1;
    desc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ;
    desc.Usage              = D3D11_USAGE_STAGING;

    const auto hr = pdevice->CreateTexture2D(&desc, nullptr, &stagingTexture);
    if (FAILED(hr) || !stagingTexture)
        return false;

    return true;
}

//
// Starts output duplication on the given device.
// Populates 'cOutputDuplication' and returns true if successful.
//
bool ScreenCaptureDX11::StartOutputDuplication(
        ID3D11Device *pdevice,
        CComPtr<IDXGIOutputDuplication> &cOutputDuplication
        )
{
    CComPtr<IDXGIDevice> dxgiDevice;
    HRESULT hr = pdevice->QueryInterface(__uuidof(dxgiDevice), reinterpret_cast<void**>(&dxgiDevice));
    if (FAILED(hr))
        return false;

    CComPtr<IDXGIAdapter> dxgiAdapter;
    hr = dxgiDevice->GetParent(__uuidof(dxgiAdapter), reinterpret_cast<void**>(&dxgiAdapter));
    if (FAILED(hr))
        return false;

    CComPtr<IDXGIOutput> dxgiOutput;
    hr = dxgiAdapter->EnumOutputs(0, &dxgiOutput);
    if (FAILED(hr))
        return false;

    CComPtr<IDXGIOutput1> dxgiOutput1;
    hr = dxgiOutput->QueryInterface(
        __uuidof(IDXGIOutput1),
        reinterpret_cast<void**>(&dxgiOutput1)
    );
    if (FAILED(hr))
        return false;

    hr = dxgiOutput1->DuplicateOutput(pdevice, &cOutputDuplication);
    if (FAILED(hr))
        return false;

    if (!cOutputDuplication)
        return false;

    return true;
}

//
// Attempts to capture a screen image, placing it into
// 'acquiredDesktopImage'.  Returns true if successful.
//
bool ScreenCaptureDX11::AcquireNextFrame(
    IDXGIOutputDuplication *cOutputDuplication,
    CComPtr<ID3D11Texture2D> &acquiredDesktopImage
    )
{
    // This will be filled with the resource interface of the
    // surface that contains the desktop image.
    CComPtr<IDXGIResource> desktopResource;

    // It may take several tries to get the frame.
    HRESULT hr = E_FAIL;
    for (int attempt = 0; attempt < 4; attempt++)
    {
        DXGI_OUTDUPL_FRAME_INFO finfo = {};
        hr = cOutputDuplication->AcquireNextFrame(50 /*timeout ms*/,
                &finfo, &desktopResource);
        if (SUCCEEDED(hr) && finfo.LastPresentTime.QuadPart == 0)
        {
            // When QuadPart is zero, the frame is still in the process
            // of being captured, but we do still need to release the
            // resource and frame.
            desktopResource.Release();
            cOutputDuplication->ReleaseFrame();
            Sleep(1);
        }
        else if (SUCCEEDED(hr) && !desktopResource)
        {
            // The call succeeded but desktopResource was not populated.
            cOutputDuplication->ReleaseFrame();
            Sleep(1);
        }
        else
        {
            break;
        }
    }

    //
    // Debugging Note:
    //   If we timed out getting the next frame, 'hr' will
    //   contain DXGI_ERROR_WAIT_TIMEOUT (aka 0x887A0027)
    //   when we reach here.
    //

    if (FAILED(hr) || !desktopResource)
        return false;

    // Get the texture for the screen image that was just captured.
    hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D),
                    reinterpret_cast<void **>(&acquiredDesktopImage));
    desktopResource.Release();
    if (FAILED(hr))
        return false;

    return true;
}

