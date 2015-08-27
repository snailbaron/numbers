#include "application.hpp"
#include <cmath>
#include <queue>
#include <windowsx.h>
#include <ctime>
#include <sstream>

struct ZoneInfo
{
    ZoneInfo() : hsum(0), wsum(0), count(0) { }
    UINT hsum;
    UINT wsum;
    UINT count;
};

Application::Application(HINSTANCE hInstance) :
    _hInstance(hInstance)
{ }

Application::~Application()
{
    if (_d2dFactory != NULL)
    {
        _d2dFactory->Release();
        _d2dFactory = NULL;
    }

    if (_renderTarget != NULL)
    {
        _renderTarget->Release();
        _renderTarget = NULL;
    }

    if (_grayBrush != NULL)
    {
        _grayBrush->Release();
        _grayBrush = NULL;
    }
}

void Application::RunMessageLoop()
{
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

HRESULT Application::Initialize()
{
    // Create device-independent resources
    HRESULT hr = CreateDeviceIndependentResources();
    if (FAILED(hr))
        return hr;

    LoadNewImage(L"lev1.bmp");
    CalculateZoneMap();

    // Register window class
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WinProc;
    wc.cbWndExtra = sizeof(LONG_PTR);
    wc.hInstance = _hInstance;
    wc.hCursor = LoadCursor(NULL, IDI_APPLICATION);
    wc.lpszClassName = L"MainWndClass";
    RegisterClassEx(&wc);

    // Create window. Size is adjusted using desktop DPI.
    FLOAT dpiX, dpiY;
    _d2dFactory->GetDesktopDpi(&dpiX, &dpiY);

    _hwnd = CreateWindowEx(
        0,
        L"MainWndClass",
        L"Main Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        static_cast<UINT>(ceil(640.f * dpiX / 96.f)),
        static_cast<UINT>(ceil(480.f * dpiY / 96.f)),
        NULL,
        NULL,
        _hInstance,
        this
    );
    hr = _hwnd ? S_OK : E_FAIL;

    if (SUCCEEDED(hr))
    {
        ShowWindow(_hwnd, SW_SHOWNORMAL);
        UpdateWindow(_hwnd);
    }

    CreateDeviceResources();

    return hr;
}

HRESULT Application::CreateDeviceIndependentResources()
{
    HRESULT hr = S_OK;

    // Create D2D factory
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_d2dFactory);
    if (FAILED(hr)) return hr;

    // Create WIC factory
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&_wicFactory)
        );
    if (FAILED(hr)) return hr;

    // Create DirectWrite factory
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **)&_dwFactory);
    if (FAILED(hr)) return hr;

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

HRESULT Application::CreateDeviceResources()
{
    HRESULT hr = S_OK;
    if (!_renderTarget)
    {
        RECT rc;
        GetClientRect(_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

        hr = _d2dFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(_hwnd, size),
            &_renderTarget
        );

        if (SUCCEEDED(hr))
        {
            hr = _renderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Gray),
                &_grayBrush
            );
        }
    }

    if (SUCCEEDED(hr))
        if (!_d2dBitmap)
            hr = UpdateD2DImage();

    return hr;      
}

void Application::DiscardDeviceResources()
{
    if (_renderTarget)
    {
        _renderTarget->Release();
        _renderTarget = NULL;
    }

    if (_grayBrush)
    {
        _grayBrush->Release();
        _grayBrush = NULL;
    }
}

LRESULT CALLBACK Application::WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    // When creating window, save application ptr to user data
    if (msg == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT) lparam;
        Application *app = (Application *) pcs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, PtrToLong(app));
        return 1;
    }

    // Get application ptr from window's user data
    Application *app = reinterpret_cast<Application *>(static_cast<LONG_PTR>(GetWindowLongPtr(hwnd, GWLP_USERDATA)));

    // Process some messages, if application ptr is available
    if (app)
    {
        switch (msg)
        {
            case WM_SIZE:
            {
                UINT width = LOWORD(lparam);
                UINT height = HIWORD(lparam);
                app->OnResize(width, height);
                return 0;
            }

            case WM_DISPLAYCHANGE:
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;

            case WM_PAINT:
            {
                app->OnRender();
                ValidateRect(hwnd, NULL);
                return 0;
            }

            case WM_LBUTTONDOWN:
            {
                WORD x = GET_X_LPARAM(lparam), y = GET_Y_LPARAM(lparam);
                D2D1_SIZE_F rtSize = app->GetRenderTarget()->GetSize();

                UINT bw, bh;
                app->GetBitmap()->GetSize(&bw, &bh);

                UINT xp = x * bw / (UINT)rtSize.width;
                UINT yp = y * bw / (UINT)rtSize.height;

                app->Fill(xp, yp);
                return 0;
            }

            case WM_DESTROY:
                PostQuitMessage(0);
                return 1;
        }

    }

    // If message is still not processed, use default WinProc
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

HRESULT Application::OnRender()
{
    HRESULT hr = CreateDeviceResources();
    if (SUCCEEDED(hr))
    {
        D2D1_SIZE_F bsize = _d2dBitmap->GetSize();

        D2D1_SIZE_F size = _renderTarget->GetSize();
        D2D1_RECT_F rect = { 0, 0, size.width, size.height };

        _renderTarget->BeginDraw();
        _renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
        _renderTarget->DrawBitmap(_d2dBitmap, rect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);

        UINT i = 0;
        std::for_each(_zoneCenters.begin(), _zoneCenters.end(), [&](const std::pair<UINT, UINT> &p) {
            i++;
            std::wostringstream os;

            UINT j;
            for (j = 0; j < _colorCount-1 && _colorOrder[j] != i; j++);
            os << j+1;


            D2D1_RECT_F rect = { p.second * size.width / bsize.width, p.first * size.height / bsize.height, 0, 0 };
            rect.right = rect.left;
            rect.bottom = rect.top;
            _renderTarget->DrawTextW(os.str().c_str(), os.str().length(), _textFormat, rect, _grayBrush);
        });
        hr = _renderTarget->EndDraw();

        if (hr == D2DERR_RECREATE_TARGET)
        {
            hr = S_OK;
            DiscardDeviceResources();
        }
    }

    return hr;
}

void Application::OnResize(UINT width, UINT height)
{
    if (_renderTarget)
        _renderTarget->Resize(D2D1::SizeU(width, height));
}

BOOL Application::CalculateZoneMap()
{
    UINT frameWidth, frameHeight;
    _bwBitmap->GetSize(&frameWidth, &frameHeight);

    UINT frameStride = (frameWidth + 7) / 8;
    UINT frameBufferSize = frameStride * frameHeight;
    BYTE *frameBuffer = new BYTE[frameBufferSize];
    _bwBitmap->CopyPixels(NULL, frameStride, frameBufferSize, frameBuffer);

    if (_zoneMapData) delete[] _zoneMapData;
    if (_zoneMap) delete[] _zoneMap;

    _zoneMapData = new int[frameHeight * frameWidth];
    ZeroMemory(_zoneMapData, frameHeight * frameWidth * sizeof(int));
    _zoneMap = new int*[frameHeight];
    for (UINT i = 0; i < frameHeight; i++)
        _zoneMap[i] = &_zoneMapData[i * frameWidth];

    std::list<ZoneInfo> zoneInfos;
    int colorCount = 0;
    for (UINT i = 0; i < frameHeight; i++)
    {
        for (UINT j = 0; j < frameWidth; j++)
        {
            BYTE topFrameBit = (frameBuffer[i * frameStride + j / 8] & (1 << (7 - j % 8))) >> (7 - j % 8);
            if (!topFrameBit)
            {
                _zoneMap[i][j] = -1;
                continue;
            }

            if (_zoneMap[i][j])
                continue;

            colorCount++;
            zoneInfos.push_back(ZoneInfo());

            std::queue<std::pair<UINT, UINT>> q;
            q.push(std::make_pair(i, j));
            while (!q.empty())
            {
                UINT k = q.front().first, l = q.front().second;
                q.pop();

                if (k < 0 || k >= frameHeight || l < 0 || l >= frameWidth)
                    continue;

                BYTE frameBit = (frameBuffer[k * frameStride + l / 8] & (1 << (7 - l % 8))) >> (7 - l % 8);
                if (!frameBit)
                {
                    _zoneMap[k][l] = -1;
                    continue;
                }

                if (k >= 0 && k < frameHeight && l >= 0 && l < frameWidth && !_zoneMap[k][l])
                {
                    _zoneMap[k][l] = colorCount;
                    zoneInfos.back().hsum += k;
                    zoneInfos.back().wsum += l;
                    zoneInfos.back().count++;

                    q.push(std::make_pair(k - 1, l));
                    q.push(std::make_pair(k + 1, l));
                    q.push(std::make_pair(k, l - 1));
                    q.push(std::make_pair(k, l + 1));
                }
            }
        }
    }
    _colorCount = colorCount;

    _zoneCenters.clear();
    std::for_each(zoneInfos.begin(), zoneInfos.end(), [&](const ZoneInfo &zi) {
        _zoneCenters.push_back(std::make_pair(zi.hsum / zi.count, zi.wsum / zi.count));
    });

    if (_colorOrder) delete[] _colorOrder;
    _colorOrder = new int[_colorCount];
    for (UINT i = 0; i < _colorCount; i++)
        _colorOrder[i] = i + 1;

    srand((unsigned int)time(nullptr));
    for (UINT i = 0; i < _colorCount; i++)
    {
        int r = rand() % _colorCount;
        int temp = _colorOrder[i];
        _colorOrder[i] = _colorOrder[r];
        _colorOrder[r] = temp;
    }

    delete[] frameBuffer;

    return TRUE;
}

HRESULT Application::Fill(UINT x, UINT y)
{
    HRESULT hr = S_OK;

    UINT targetZone = _zoneMap[y][x];
    if (targetZone != _colorOrder[_currentColorIndex])
        return hr;

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

    int one=1, two=1;
    for (size_t i = 0; i < 500; i++)
        for (size_t j = 0; j < 500; j++)
            if (_zoneMap[i][j] == targetZone)
                *((UINT32 *) (buffer + i * stride + j * 4)) = COLORS[targetZone % COLORS_COUNT];

    hr = lock->Release();

    hr = UpdateD2DImage();
    InvalidateRect(_hwnd, NULL, FALSE);

    _currentColorIndex++;

    return hr;
}

HRESULT Application::LoadNewImage(const WCHAR *fileName)
{
    HRESULT hr = S_OK;

    // Read first frame of image file
    IWICBitmapDecoder *decoder = NULL;
    hr = _wicFactory->CreateDecoderFromFilename(L"lev1.bmp", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
    if (FAILED(hr)) return hr;

    IWICBitmapFrameDecode *frame = NULL;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) return hr;

    // Convert to black & white for zone map building
    IWICFormatConverter *bwConverter = NULL;
    hr = _wicFactory->CreateFormatConverter(&bwConverter);
    if (FAILED(hr)) return hr;

    hr = bwConverter->Initialize(
        frame,
        GUID_WICPixelFormatBlackWhite,
        WICBitmapDitherTypeNone,
        NULL,
        0.0,
        WICBitmapPaletteTypeCustom
        );
    if (FAILED(hr)) return hr;

    _bwBitmap = bwConverter;

    // Convert to PBGRA for later display
    IWICFormatConverter *displayConverter = NULL;
    hr = _wicFactory->CreateFormatConverter(&displayConverter);
    if (FAILED(hr)) return hr;

    hr = displayConverter->Initialize(
        bwConverter,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        NULL,
        0.f,
        WICBitmapPaletteTypeCustom
        );
    if (FAILED(hr)) return hr;

    // Convert to bitmap for editing
    hr = _wicFactory->CreateBitmapFromSource(displayConverter, WICBitmapCacheOnLoad, &_editBitmap);
    if (FAILED(hr)) return hr;

    return hr;
}

HRESULT Application::UpdateD2DImage()
{
    if (_d2dBitmap)
        _d2dBitmap->Release();

    HRESULT hr = _renderTarget->CreateBitmapFromWicBitmap(_editBitmap, &_d2dBitmap);
    return hr;
}