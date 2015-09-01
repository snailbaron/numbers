#pragma once

#include <Windows.h>
#include <wincodec.h>

class BitmapBuffer
{
public:
    BitmapBuffer();
    ~BitmapBuffer();

    UINT GetHeight() const { return _height; }
    UINT GetWidth() const { return _width; }
    BYTE GetPixel(UINT x, UINT y) const;

    HRESULT Load(IWICBitmapSource *source);

private:
    BYTE *_buffer;
    UINT _height;
    UINT _width;
    UINT _stride;
};