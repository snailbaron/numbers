#include <Windows.h>
#include <d2d1.h>
#include <comdef.h>
#include <sstream>
#include "application.hpp"
#include "gfx_window.hpp"

void Check(HRESULT hr)
{
    if (FAILED(hr))
    {
        std::wostringstream os;
        os << std::hex << hr;
        MessageBox(NULL, os.str().c_str(), L"Error", MB_OK | MB_ICONERROR);
        exit(1);
    }
}

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int)
{
    // Terminate on heap corruption
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    // Initialize COM
    if (SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
    {
        //Application app(hInstance);
        //if (SUCCEEDED(app.Initialize()))
        //{
        //    app.RunMessageLoop();
        //}

        ID2D1Factory *d2dFactory = NULL;
        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dFactory);
        Check(hr);

        GfxWindow window(hInstance, L"Main Window", d2dFactory);
        Check(window.Initialize());

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }


        CoUninitialize();
    }

    return 0;
}