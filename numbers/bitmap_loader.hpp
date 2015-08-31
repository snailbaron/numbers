#pragma once

#include <Windows.h>
#include <wincodec.h>
#include "zone_info.hpp"

class BitmapLoader
{
public:
    BitmapLoader(IWICImagingFactory *wicFactory);
    ~BitmapLoader();

    HRESULT Load(LPCWSTR fileName, ZoneInfo *zoneInfo, IWICBitmap **displayBitmap);

private:
    IWICImagingFactory *_wicFactory;
};