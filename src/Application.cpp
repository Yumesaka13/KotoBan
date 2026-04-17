#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)
#include <httplib/httplib.h>

#include "Application.h"
#include "RenderDevice.h"
#include "NewTab.h"
#include "BangumiAPI.h"
#include "TextureManager.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_dx11.h>
#include <json/json.hpp>

Application::Application()
{
    mHwnd = 0;
    mLastMessageId = 0;
}

bool Application::Init()
{
    ImGui_ImplWin32_EnableDpiAwareness();
    float aMainScale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProcStatic, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"KotoBan", nullptr };
    ::RegisterClassExW(&wc);

    mHwnd = ::CreateWindowW(wc.lpszClassName, L"KotoBan", WS_OVERLAPPEDWINDOW, 100, 100, (int)(800 * aMainScale), (int)(480 * aMainScale), nullptr, nullptr, wc.hInstance, nullptr);

    if (!mRenderDevice.CreateDevice(mHwnd))
    {
        mRenderDevice.CleanupDevice();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(mHwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(mHwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsLight();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(aMainScale);
    style.FontScaleDpi = aMainScale;

    ImGui_ImplWin32_Init(mHwnd);
    ImGui_ImplDX11_Init(mRenderDevice.GetDevice(), mRenderDevice.GetContext());

    io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyh.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
}

void Application::UpdateUI()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 7.0f));

    ImGui::Begin("KotoBan", nullptr, window_flags);

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Hi! ");
    ImGui::SameLine(0.0f, 10.0f);

    UpdateTabs();





    ImGui::PopStyleVar(4);
    ImGui::End();
}

void Application::UpdateTabs()
{
    if (ImGui::BeginTabBar("MainTabs", ImGuiTabBarFlags_None))
    {

        if (ImGui::BeginTabItem("首页"))
        {
            if (ImGui::Button("刷新讨论列表")) 
            {
                FetchMessages();
            }

            ImGui::Separator();

            if (mChatHistory.empty())
            {
                ImGui::TextDisabled("暂无数据，请点击刷新...");
                static bool aTestImage = false;
                static ID3D11ShaderResourceView* anImage = nullptr;
                if (!aTestImage)
                {
                    anImage = DownloadImage(mRenderDevice.GetDevice(), "");
                    aTestImage = true;
                }
                if (anImage)
                {
                    ImGui::Image((ImTextureID)anImage, ImVec2(266, 266), ImVec2(0, 0), ImVec2(1, 1));
                }
            }
            else
            {
                std::lock_guard<std::mutex> lock(mChatMutex);
                for (const auto& msg : mChatHistory) 
                {
                    ImGui::TextWrapped("%s:", msg->nickname.c_str());
                    ImGui::SameLine();
                    ImGui::TextWrapped("%s", msg->content.c_str());
                    ImGui::Separator();
                }
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("设置"))
        {
            ImGui::Text("Hello, Hell!");
            
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void Application::FetchMessages()
{
    for (Message* aMessage : mChatHistory)
        delete aMessage;

    httplib::Client aClient("https://bgmchat.ry.mk");
    httplib::Headers aHeaders = { { "User-Agent", "866905/KotoBan/(Developing Version)" } };

    if (auto aRes = aClient.Get("/api/messages", aHeaders))
    {
        if (aRes->status != 200)
            return;

        auto data = nlohmann::json::parse(aRes->body);
        std::lock_guard<std::mutex> lock(mChatMutex);
        for (auto& item : data)
        {
            int currentId = item["id"].get<int>();
            if (currentId > mLastMessageId)
            {
                Message* aMessage = new Message();
                aMessage->id = currentId;
                aMessage->nickname = item["nickname"].get<std::string>();
                aMessage->content = item["message"].get<std::string>();

                mChatHistory.push_back(aMessage);
                mLastMessageId = currentId;
            }
        }
    }
}

void Application::Run()
{
    bool anExit = false;
    while (!anExit)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                anExit = true;
        }
        if (anExit)
            break;

        if (mRenderDevice.mSwapChainOccluded && mRenderDevice.GetSwapChain()->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        mRenderDevice.mSwapChainOccluded = false;
        if (mRenderDevice.mResizeWidth != 0 && mRenderDevice.mResizeHeight != 0)
        {
            mRenderDevice.CleanupRenderTarget();
            mRenderDevice.GetSwapChain()->ResizeBuffers(0, mRenderDevice.mResizeWidth, mRenderDevice.mResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            mRenderDevice.mResizeWidth = mRenderDevice.mResizeHeight = 0;
            mRenderDevice.CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        UpdateUI();

        ImGui::Render();

        ImVec4 aClearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        const float aClearColorWithAlpha[4] = { aClearColor.x * aClearColor.w, aClearColor.y * aClearColor.w, aClearColor.z * aClearColor.w, aClearColor.w };
        ID3D11RenderTargetView* aRTV = mRenderDevice.GetRTV(); 
        mRenderDevice.GetContext()->OMSetRenderTargets(1, &aRTV, nullptr);
        mRenderDevice.GetContext()->ClearRenderTargetView(aRTV, aClearColorWithAlpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        HRESULT hr = mRenderDevice.GetSwapChain()->Present(1, 0);
        mRenderDevice.mSwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }
}

void Application::Shutdown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    mRenderDevice.CleanupDevice();
    ::DestroyWindow(mHwnd);
    ::UnregisterClassW(L"KotoBan", GetModuleHandle(nullptr));
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Application::WndProcStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    Application* pApp = (Application*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pApp = (Application*)pCreate->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pApp);
    }

    if (pApp) return pApp->WndProc(hWnd, msg, wParam, lParam);
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
LRESULT Application::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        mRenderDevice.mResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        mRenderDevice.mResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
