#pragma once

#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include "gfx_window.hpp"
#include "event_queue.hpp"
#include "zone_info.hpp"

class Core
{
public:
    Core(HINSTANCE hInstance);
    ~Core();

    ID2D1Factory * GetD2DFactory() { return _d2dFactory; }

    HRESULT Initialize();
    void Finalize();

    HRESULT PlayLevel(LPCWSTR levelFile);

    HRESULT OnRender(const Event::Render &evt);
    HRESULT OnPush(const Event::Push &evt);

private:
    void HandleEvent(const Event::Base &evt);

private:
    HINSTANCE _hInstance;
    IWICImagingFactory *_wicFactory;
    ID2D1Factory *_d2dFactory;
    IDWriteFactory *_dwFactory;
    IDWriteTextFormat *_textFormat;
    ID2D1SolidColorBrush *_brush;
    EventQueue _eventQueue;

    ZoneInfo _zoneInfo;
    IWICBitmap *_editBitmap;
    ID2D1Bitmap *_displayBitmap;
    UINT _currentColorIndex = 0;

    static const UINT COLORS_COUNT = 6;
    const UINT32 COLORS[COLORS_COUNT] = {
        0xFF0000FF, 0x00FF00FF, 0x0000FFFF,
        0xFFFF00FF, 0xFF00FFFF, 0x00FFFFFF
    };
};