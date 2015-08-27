#pragma once

#include <Windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <dwrite.h>
#include <list>

class Application
{
public:
    Application(HINSTANCE hInstance);
    ~Application();

    ID2D1HwndRenderTarget * GetRenderTarget() { return _renderTarget; }
    IWICBitmapSource * GetBitmap() { return _bwBitmap; }

    HRESULT Initialize();
    void RunMessageLoop();

private:
    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();

    HRESULT OnRender();
    void OnResize(UINT width, UINT height);

    BOOL CalculateZoneMap();

    HRESULT LoadNewImage(const WCHAR *fileName);
    HRESULT UpdateD2DImage();
    HRESULT Fill(UINT x, UINT y);

    static const UINT COLORS_COUNT = 6;
    UINT32 COLORS[COLORS_COUNT] = {
        0xFFFF0000, 0xFF00FF00, 0xFF0000FF,
        0xFFFFFF00, 0xFFFF00FF, 0xFF00FFFF,
    };
        
    // Window procedure
    static LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
    HINSTANCE _hInstance;
    HWND _hwnd = NULL;

    // Direct2D
    ID2D1Factory *_d2dFactory = NULL;
    ID2D1HwndRenderTarget *_renderTarget = NULL;
    ID2D1SolidColorBrush *_grayBrush = NULL;
    ID2D1Bitmap *_d2dBitmap = NULL;

    // WIC
    IWICImagingFactory *_wicFactory = NULL;
    IWICBitmapSource *_bwBitmap = NULL;
    IWICBitmap *_editBitmap = NULL;

    // DirectWrite
    IDWriteFactory *_dwFactory = NULL;
    IDWriteTextFormat *_textFormat = NULL;


    // Image zone map
    int **_zoneMap = nullptr;
    int *_zoneMapData = nullptr;
    UINT _colorCount = 0;
    int *_colorOrder = nullptr;
    UINT _currentColorIndex = 0;
    std::list<std::pair<UINT, UINT>> _zoneCenters;
    
};
