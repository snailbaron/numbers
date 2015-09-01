#include "gfx_window.hpp"
#include <windowsx.h>
#include "utils.hpp"

const WCHAR GfxWindow::_WND_CLASS_NAME[] = L"GfxWindowClass";
ATOM GfxWindow::_wndClassAtom = NULL;

/**
    Static window procedure. Used to save/restore pointer to Window object,
    and pass the message to its method.
 */
LRESULT CALLBACK GfxWindow::WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Get pointer to Window object (from LPCREATESTRUCT if we're in WM_NCCREATE,
    // or from window's LongPtr otherwize.
    GfxWindow *wnd = nullptr;
    if (msg == WM_NCCREATE)
    {
        LPCREATESTRUCT cs = (LPCREATESTRUCT) lparam;
        GfxWindow *wnd = (GfxWindow *)cs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, PtrToLong(wnd));
    }
    else
    {
        wnd = (GfxWindow *)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    // Pass this message to Window's method, or to default window procedure
    if (wnd)
    {
        // Set Window's HWND, in case we have not yet returned from CreateWindow
        if (!wnd->_hwnd) wnd->_hwnd = hwnd;
        return wnd->HandleMessage(msg, wparam, lparam);
    }
    else
    {
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}

/**
    Method for message handling.
    Used to generate events for an event queue.
*/
LRESULT GfxWindow::HandleMessage(UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
        case WM_SIZE:
        {
            UINT width = LOWORD(lparam);
            UINT height = HIWORD(lparam);
            if (_renderTarget)
                _renderTarget->Resize(D2D1::SizeU(width, height));
            return 0;
        }

        case WM_DISPLAYCHANGE:
        {
            InvalidateRect(_hwnd, NULL, FALSE);
            return 0;
        }

        case WM_PAINT:
        {
            if (_evtQueue)
                _evtQueue->Push(new Event::Render(_renderTarget));
            ValidateRect(_hwnd, NULL);
            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            int x = GET_X_LPARAM(lparam), y = GET_Y_LPARAM(lparam);
            if (_evtQueue)
                _evtQueue->Push(new Event::Push(_hwnd, _renderTarget, x, y));
            return 0;
        }
    }

    return DefWindowProc(_hwnd, msg, wparam, lparam);
}

GfxWindow::GfxWindow(HINSTANCE hInstance, LPCWSTR name, ID2D1Factory *d2dFactory) :
    _hInstance(hInstance),
    _name(nullptr),
    _d2dFactory(d2dFactory),
    _hwnd(NULL),
    _renderTarget(NULL),
    _evtQueue(nullptr)
{
    // Copy window name
    size_t nameLen = wcslen(name);
    _name = new WCHAR[nameLen + 1];
    wcscpy_s(_name, nameLen + 1, name);
}

GfxWindow::~GfxWindow()
{
    // Free resources
    if (_name) delete[] _name;

    // Destroy window
    if (_hwnd && DestroyWindow(_hwnd))
        _hwnd = NULL;
}

/** Register window class, if not already done */
HRESULT GfxWindow::Register()
{
    if (!_wndClassAtom)
    {
        WNDCLASSEX wc = {0};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpszClassName = _WND_CLASS_NAME;
        wc.hCursor = LoadCursor(NULL, IDI_APPLICATION);
        wc.lpfnWndProc = GfxWindow::WinProc;
        wc.cbWndExtra = sizeof(LONG_PTR);
        _wndClassAtom = RegisterClassEx(&wc);
        if (!_wndClassAtom)
            return HRESULT_FROM_WIN32(GetLastError());
    }
    return S_OK;
}

/** Initialize window */
HRESULT GfxWindow::Initialize()
{
    // Try to register window class, if not already registered
    if (!_wndClassAtom)
    {
        HRESULT regResult = Register();
        if (FAILED(regResult)) return regResult;
    }

    // Create window
    _hwnd = CreateWindowEx(
        0,
        MAKEINTATOM(_wndClassAtom),
        _name,
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
    // Release previous render target, if present
    ReleaseCOM(&_renderTarget);

    // Get client area size
    RECT clientRect;
    if (!GetClientRect(_hwnd, &clientRect))
        return HRESULT_FROM_WIN32(GetLastError());
       
    // Create render target
    D2D1_SIZE_U rtSize = D2D1::SizeU(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
    HRESULT hr = _d2dFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(_hwnd, rtSize),
        &_renderTarget
    );

    return hr;
}

/**
    Register event queue to pass events to.
    May be null to omit event generation.
*/
void GfxWindow::RegisterEventQueue(EventQueue *queue)
{
    _evtQueue = queue;
}