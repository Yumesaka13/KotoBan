#pragma once
#include <vector>
#include <mutex>
#include <windows.h>
#include "RenderDevice.h"
#include "BangumiAPI.h"


class Application {
public:
    HWND                    mHwnd;
    RenderDevice            mRenderDevice;
    std::vector<Message*>   mChatHistory;
    std::mutex              mChatMutex;
    int                     mLastMessageId;

public:
    Application();
    ~Application() {};
    bool                    Init();
    void                    UpdateUI();
    void                    UpdateTabs();
    void                    Run();
    void                    Shutdown();
    void                    FetchMessages();

private:
    static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT                 WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};