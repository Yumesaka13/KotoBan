#include "Application.h"

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    Application* anApp = new Application();

    if (!anApp->Init())
        return 1;

    anApp->Run();
    anApp->Shutdown();
    delete anApp;
    return 0;
}