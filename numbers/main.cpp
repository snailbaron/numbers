#include <Windows.h>
#include <sstream>
#include "core.hpp"

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
    HRESULT hr = GlobalInit();
    if (SUCCEEDED(hr))
    {
        Core core(hInstance);
        hr = core.Initialize();
        if (SUCCEEDED(hr))
        {
            core.PlayLevel(L"lev1.bmp");
        }
        GlobalFinalize();
    }
    else
    {
        ReportFail(hr);
    }

    return 0;
}