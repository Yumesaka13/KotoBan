#include "Application.h"

/*
Update Logs:
1.Formatted the variable names.
2.Added an image loading function and a test of rendering images.
*/

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