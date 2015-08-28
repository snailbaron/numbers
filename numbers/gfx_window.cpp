#include "gfx_window.hpp"

const WCHAR GfxWindow::_WND_CLASS_NAME[] = L"GfxWindowClass";
bool GfxWindow::_classRegistered = false;

LRESULT CALLBACK GfxWindow::WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

GfxWindow::GfxWindow(HINSTANCE hInstance, LPCWSTR name, ID2D1Factory *d2dFactory) :
    _hInstance(hInstance),
    _name(name),
    _d2dFactory(d2dFactory),
    _hwnd(NULL),
    _renderTarget(NULL)
{ }

GfxWindow::~GfxWindow()
{
    // Try to destroy window, if already created
    if (_hwnd)
    {
        BOOL res = DestroyWindow(_hwnd);
        if (res)
            _hwnd = NULL;
    }
}

HRESULT GfxWindow::Register()
{
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpszClassName = _WND_CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDI_APPLICATION);
    wc.lpfnWndProc = GfxWindow::WinProc;
    wc.cbWndExtra = sizeof(LONG_PTR);
    if (!RegisterClassEx(&wc))
        return HRESULT_FROM_WIN32(GetLastError());

    _classRegistered = true;    
    return S_OK;
}

HRESULT GfxWindow::Initialize()
{
    // Try to register window class, if not already registered
    if (!_classRegistered)
    {
        HRESULT regResult = Register();
        if (FAILED(regResult))
            return regResult;
    }

    // Create window
    _hwnd = CreateWindowEx(
        0,
        _WND_CLASS_NAME,
        _name.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 800,
        NULL,
        NULL,
        _hInstance,
        this
    );
    if (!_hwnd)
        return HRESULT_FROM_WIN32(GetLastError());

    // Show window immediately after creation
    ShowWindow(_hwnd, SW_SHOWNORMAL);

    // Send WM_PAINT immediately
    if (!UpdateWindow(_hwnd))
        return HRESULT_FROM_WIN32(GetLastError());

    // Create D2D render target for created window
    HRESULT hr = CreateRenderTarget();
    if (FAILED(hr))
        return hr;
    
    return S_OK;
}

HRESULT GfxWindow::CreateRenderTarget()
{
    // Release existing render target, if present
    if (_renderTarget)
    {
        _renderTarget->Release();
        _renderTarget = NULL;
    }

    RECT clientRect;
    if (!GetClientRect(_hwnd, &clientRect))
        return HRESULT_FROM_WIN32(GetLastError());
       
    D2D1_SIZE_U rtSize = D2D1::SizeU(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

    HRESULT hr = _d2dFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(_hwnd, rtSize),
        &_renderTarget
    );

    return hr;
}