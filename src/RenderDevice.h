#pragma once
#include <d3d11.h>

class RenderDevice {
public:
    bool                    mSwapChainOccluded;
    UINT                    mResizeWidth;
    UINT                    mResizeHeight;

public:
    RenderDevice();
    ~RenderDevice() {};
    bool                    CreateDevice(HWND hWnd);
    void                    CleanupDevice();
    void                    CreateRenderTarget();
    void                    CleanupRenderTarget();
    void                    Present();
    ID3D11Device*           GetDevice() { return mD3DDevice; }
    ID3D11DeviceContext*    GetContext() { return mD3DDeviceContext; }
    ID3D11RenderTargetView* GetRTV() { return mRenderTargetView; }
    IDXGISwapChain*         GetSwapChain()  { return mSwapChain; }

private:
    ID3D11Device*           mD3DDevice = nullptr;
    ID3D11DeviceContext*    mD3DDeviceContext = nullptr;
    IDXGISwapChain*         mSwapChain = nullptr;
    ID3D11RenderTargetView* mRenderTargetView = nullptr;
};