#pragma once
#include <string>
#include <atomic>
class ID3D11ShaderResourceView;
class ID3D11Device;

ID3D11ShaderResourceView* CreateTextureFromMemory(ID3D11Device* theDevice, const unsigned char* theData, int theLength);
ID3D11ShaderResourceView* DownloadImage(ID3D11Device* theDevice, const std::string& theUrl);

static std::atomic<bool> gIsLoading{ false };