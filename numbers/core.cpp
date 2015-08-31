#include "core.hpp"
#include "utils.hpp"
#include "bitmap_loader.hpp"
#include "zone_info.hpp"
#include <sstream>
#include <windowsx.h>

Core::Core(HINSTANCE hInstance) :
    _hInstance(hInstance),
    _wicFactory(NULL),
    _d2dFactory(NULL),
    _dwFactory(NULL),
    _textFormat(NULL),
    _brush(NULL),
    _zoneInfo(),
    _editBitmap(NULL),
    _displayBitmap(NULL),
    _currentColor(1),
    _finished(false)
{
}

Core::~Core()
{
}

HRESULT Core::Initialize()
{
    HRESULT hr = S_OK;

    // Create WIC factory
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&_wicFactory)
    );
    if (FAILED(hr)) return hr;

    // Create D2D factory
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_d2dFactory);
    if (FAILED(hr)) return hr;

    // Create DirectWrite factory
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **)&_dwFactory);
    if (FAILED(hr)) return hr;

    // Register gfx window class for faster use later
    hr = GfxWindow::Register();
    if (FAILED(hr)) return hr;

    // Create text format for window
    hr = _dwFactory->CreateTextFormat(
        L"Calibri",
        NULL,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        20.0f,
        L"en-us",
        &_textFormat
        );
    if (FAILED(hr)) return hr;

    hr = _textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    if (FAILED(hr)) return hr;
    hr = _textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    if (FAILED(hr)) return hr;

    return hr;
}

void Core::Finalize()
{
    ReleaseCOM(&_dwFactory);
    ReleaseCOM(&_d2dFactory);
    ReleaseCOM(&_wicFactory);
}

HRESULT Core::PlayLevel(LPCWSTR levelFile)
{
    HRESULT hr = S_OK;

    _finished = false;

    // Load and process level bitmap
    BitmapLoader loader(_wicFactory);

    hr = loader.Load(levelFile, &_zoneInfo, &_editBitmap);
    if (FAILED(hr)) return hr;

    // Create main window for level
    GfxWindow window(_hInstance, L"Main Window", _d2dFactory);
    hr = window.Initialize();
    if (FAILED(hr)) return hr;

    _eventQueue.Clear();
    window.RegisterEventQueue(&_eventQueue);

    RECT clientRect;
    GetClientRect(window.GetHwnd(), &clientRect);
    InvalidateRect(window.GetHwnd(), &clientRect, FALSE);

    hr = window.GetRenderTarget()->CreateBitmapFromWicBitmap(_editBitmap, &_displayBitmap);
    if (FAILED(hr)) return hr;

    hr = window.GetRenderTarget()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &_brush);
    if (FAILED(hr)) return hr;

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                done = true;
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (done) break;

        while (!_eventQueue.Empty())
        {
            Event::Base *evt = _eventQueue.Pull();
            HandleEvent(evt);
            delete evt;

            if (_finished)
                done = true;
        }
    }

    return hr;
}

HRESULT Core::OnRender(const Event::Render *evt)
{
    HRESULT hr = S_OK;

    evt->renderTarget->BeginDraw();
    evt->renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

    D2D1_SIZE_F rtSize = evt->renderTarget->GetSize();
    D2D1_RECT_F rtRect = { 0, 0, rtSize.width, rtSize.height };
    evt->renderTarget->DrawBitmap(_displayBitmap, rtRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);

    D2D1_SIZE_F bmSize = _displayBitmap->GetSize();
    for (UINT i = 1; i <= _zoneInfo.GetColorCount(); i++)
    {
        UINT x, y;
        std::tie(y, x) = _zoneInfo.GetZoneCenter(i);

        UINT color = _zoneInfo.GetColor(i);
        std::wostringstream os;
        os << color;

        D2D1_RECT_F textRect = { x * rtSize.width / bmSize.width, y * rtSize.height / bmSize.height, 0, 0 };
        textRect.right = textRect.left + 50.f;
        textRect.bottom = textRect.top + 50.f;
        evt->renderTarget->DrawTextW(os.str().c_str(), os.str().length(), _textFormat, textRect, _brush);
    }
    hr = evt->renderTarget->EndDraw();

    return hr;
}

HRESULT Core::OnPush(const Event::Push *evt)
{
    D2D1_SIZE_F rtSize = evt->renderTarget->GetSize();
    D2D1_SIZE_F bmSize = _displayBitmap->GetSize();
    UINT xp = static_cast<UINT>(evt->x * bmSize.width / rtSize.width);
    UINT yp = static_cast<UINT>(evt->y * bmSize.height / rtSize.height);

    HRESULT hr = S_OK;

    UINT targetZone = _zoneInfo.GetZoneNumber(xp, yp);
    if (_zoneInfo.GetColor(targetZone) != _currentColor)
        return S_OK;

    UINT w, h;
    hr = _editBitmap->GetSize(&w, &h);
    WICRect rect = { 0, 0, (INT)w, (INT)h };
    IWICBitmapLock *lock;
    hr = _editBitmap->Lock(&rect, WICBitmapLockWrite, &lock);

    UINT stride;
    hr = lock->GetStride(&stride);

    UINT bufSize;
    BYTE *buffer;
    hr = lock->GetDataPointer(&bufSize, &buffer);

    int one = 1, two = 1;
    for (size_t i = 0; i < h; i++)
        for (size_t j = 0; j < w; j++)
            if (_zoneInfo.GetZoneNumber(j, i) == targetZone)
                *((UINT32 *)(buffer + i * stride + j * 4)) = COLORS[targetZone % COLORS_COUNT];

    hr = lock->Release();

    _displayBitmap->Release();
    hr = evt->renderTarget->CreateBitmapFromWicBitmap(_editBitmap, &_displayBitmap);
    if (FAILED(hr)) return hr;
        
    InvalidateRect(evt->hwnd, NULL, FALSE);

    _currentColor++;
    if (_currentColor > _zoneInfo.GetColorCount())
        _finished = true;

    return hr;
}

/** Dispatch events coming from event queue */
bool Core::HandleEvent(Event::Base *evt)
{
    if (Event::Render *render = dynamic_cast<Event::Render *>(evt))
        OnRender(render);
    else if (Event::Push *push = dynamic_cast<Event::Push *>(evt))
        OnPush(push);

    return true;
}
