#include "RenderDevice.h"

RenderDevice::RenderDevice()
{
    mSwapChainOccluded = false;
    mResizeWidth = 0;
    mResizeHeight = 0;
}

bool RenderDevice::CreateDevice(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &mSwapChain, &mD3DDevice, &featureLevel, &mD3DDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &mSwapChain, &mD3DDevice, &featureLevel, &mD3DDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void RenderDevice::CleanupDevice()
{
    CleanupRenderTarget();
    if (mSwapChain) { mSwapChain->Release(); mSwapChain = nullptr; }
    if (mD3DDeviceContext) { mD3DDeviceContext->Release(); mD3DDeviceContext = nullptr; }
    if (mD3DDevice) { mD3DDevice->Release(); mD3DDevice = nullptr; }
}

void RenderDevice::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    mSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    mD3DDevice->CreateRenderTargetView(pBackBuffer, nullptr, &mRenderTargetView);
    pBackBuffer->Release();
}

void RenderDevice::CleanupRenderTarget()
{
    if (mRenderTargetView) { mRenderTargetView->Release(); mRenderTargetView = nullptr; }
}
