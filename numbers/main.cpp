#include <Windows.h>
#include "application.hpp"

void Fail(PCWSTR msg)
{
    MessageBox(NULL, msg, L"Error", MB_OK | MB_ICONERROR);
    exit(1);
}

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int)
{
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    if (SUCCEEDED(CoInitialize(NULL)))
    {
        Application app(hInstance);
        if (SUCCEEDED(app.Initialize()))
        {
            app.RunMessageLoop();
        }
        CoUninitialize();
    }


    return 0;
}