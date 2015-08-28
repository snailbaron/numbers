#pragma once

#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

class Core
{

private:
    ID2D1Factory *_d2dFactory;
    IDWriteFactory *_dwFactory;
    IWICImagingFactory *_wicFactory;
};