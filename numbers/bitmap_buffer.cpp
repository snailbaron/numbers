#include "bitmap_buffer.hpp"

BitmapBuffer::BitmapBuffer() :
    _buffer(nullptr),
    _height(0),
    _width(0),
    _stride(0)
{ }

BitmapBuffer::~BitmapBuffer()
{
    if (_buffer)
        delete[] _buffer;
}

HRESULT BitmapBuffer::Load(IWICBitmapSource *source)
{
    HRESULT hr = S_OK;

    // Check pixel format for provided bitmap source
    WICPixelFormatGUID bmPixelFormat;
    hr = source->GetPixelFormat(&bmPixelFormat);
    if (FAILED(hr)) return hr;

    // Only working with 1bpp Black&White pixel formats
    if (!IsEqualGUID(bmPixelFormat, GUID_WICPixelFormatBlackWhite))
        return E_FAIL;

    // Set bitmap size parameters (width, height, stride)
    hr = source->GetSize(&_width, &_height);
    if (FAILED(hr)) return hr;
    _stride = (_width + 7) / 8;

    // Allocate buffer of proper size
    if (_buffer) delete[] _buffer;
    UINT bufferSize = _stride * _height;
    _buffer = new BYTE[bufferSize];

    // Copy bitmap pixels to buffer
    hr = source->CopyPixels(NULL, _stride, bufferSize, _buffer);
    if (FAILED(hr)) return hr;

    return hr;
}

/**
    Get color of pixel (x, y), 0 is black, 1 is white.
    If something is wrong, ignore and return black.
*/
BYTE BitmapBuffer::GetPixel(UINT x, UINT y) const
{
    // Return black if buffer is not allocated
    if (!_buffer)
        return 0;

    // Return black if outside bitmap borders
    if (x < 0 || x >= _width || y < 0 || y >= _height)
        return 0;

    // Return pixel value
    BYTE pixel = (_buffer[y * _stride + x / 8] & (1 << (7 - x % 8))) >> (7 - x % 8);
    return pixel;
}