#include <Windows.h>
#include <sstream>
#include <fstream>
#include "core.hpp"
#include "matrix.hpp"
#include "gfx_window.hpp"

void ReportFail(HRESULT hr)
{
    std::wostringstream os;
    os << std::hex << hr;
    MessageBox(NULL, os.str().c_str(), L"Error", MB_OK | MB_ICONERROR);
    exit(1);
}

void Check(HRESULT hr)
{
    if (FAILED(hr))
        ReportFail(hr);
}

HRESULT GlobalInit()
{
    HRESULT hr = S_OK;

    // Terminate on heap corruption. If fails, continue anyway.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    // Initialize COM library
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    return hr;
}

void GlobalFinalize()
{
    // Close COM library
    CoUninitialize();
}

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int)
{
    try {

    HRESULT hr = GlobalInit();
    if (SUCCEEDED(hr))
    {
        Core core(hInstance);
        hr = core.Initialize();
        if (SUCCEEDED(hr))
        {
            core.PlayLevel(L"lev1.bmp");
            core.PlayLevel(L"lev2.bmp");
            core.PlayLevel(L"lev3.bmp");
            core.PlayLevel(L"lev4.bmp");
        }
        GlobalFinalize();
    }
    else
    {
        ReportFail(hr);
    }

    }
    catch (std::exception &ex)
    {
        MessageBoxA(NULL, ex.what(), "Error", MB_OK | MB_ICONERROR);
    }

    return 0;
}