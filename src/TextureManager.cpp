#include "TextureManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)
#include <httplib/httplib.h>

#include <thread>
#include <d3d11.h>

ID3D11ShaderResourceView* CreateTextureFromMemory(ID3D11Device* theDevice, const unsigned char* theData, int theLength)
{
    int aWidth, aHeight, aChannels;

    unsigned char* anImageData = stbi_load_from_memory(theData, theLength, &aWidth, &aHeight, &aChannels, 4);
    if (!anImageData)
        return nullptr;

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = aWidth;
    desc.Height = aHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = anImageData;
    subResource.SysMemPitch = aWidth * 4;
    subResource.SysMemSlicePitch = 0;

    ID3D11Texture2D* aTexture = nullptr;
    HRESULT hr = theDevice->CreateTexture2D(&desc, &subResource, &aTexture);
    if (FAILED(hr))
    {
        stbi_image_free(anImageData);
        return nullptr;
    }

    ID3D11ShaderResourceView* out_srv = nullptr;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    hr = theDevice->CreateShaderResourceView(aTexture, &srvDesc, &out_srv);

    aTexture->Release(); // 创建完 SRV 后，Texture 本身可以释放（引用计数会维持其存在）
    stbi_image_free(anImageData);

    if (FAILED(hr)) return nullptr;
    return out_srv;
}

ID3D11ShaderResourceView* DownloadImage(ID3D11Device* theDevice, const std::string& theUrl)
{
    if (gIsLoading) return nullptr;
    gIsLoading = true;

    ID3D11ShaderResourceView* aNetTexture = nullptr;
    httplib::Client aClient("https://avatars.githubusercontent.com");
    aClient.set_follow_location(true);

    if (auto aRes = aClient.Get("/u/79508714"))
    {
        if (aRes->status == 200)
        {
            aNetTexture = CreateTextureFromMemory(theDevice, (const unsigned char*)aRes->body.data(), aRes->body.size());
        }
    }
    gIsLoading = false;
    return aNetTexture;
}