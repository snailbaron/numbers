#include "bitmap_loader.hpp"

BitmapLoader::BitmapLoader(IWICImagingFactory *wicFactory) :
    _wicFactory(wicFactory)
{ }

BitmapLoader::~BitmapLoader()
{
}

HRESULT BitmapLoader::Load(LPCWSTR fileName, ZoneInfo *zoneInfo, IWICBitmap **displayBitmap)
{
    HRESULT hr = S_OK;

    // Create decoder for image file
    IWICBitmapDecoder *decoder = NULL;
    hr = _wicFactory->CreateDecoderFromFilename(fileName, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
    if (SUCCEEDED(hr))
    {
        // Read first frame of image
        IWICBitmapFrameDecode *frame = NULL;
        hr = decoder->GetFrame(0, &frame);
        if (SUCCEEDED(hr))
        {
            // Create converter to black & white for zone map creation
            IWICFormatConverter *bwConverter = NULL;
            hr = _wicFactory->CreateFormatConverter(&bwConverter);
            if (SUCCEEDED(hr))
            {
                // Initialize black & white converter
                hr = bwConverter->Initialize(
                    frame,
                    GUID_WICPixelFormatBlackWhite,
                    WICBitmapDitherTypeNone,
                    NULL,
                    0.0,
                    WICBitmapPaletteTypeCustom
                );
                if (SUCCEEDED(hr))
                {
                    // Convert to PBGRA for later display
                    IWICFormatConverter *displayConverter = NULL;
                    hr = _wicFactory->CreateFormatConverter(&displayConverter);
                    if (SUCCEEDED(hr))
                    {
                        hr = displayConverter->Initialize(
                            bwConverter,
                            GUID_WICPixelFormat32bppPBGRA,
                            WICBitmapDitherTypeNone,
                            NULL,
                            0.f,
                            WICBitmapPaletteTypeCustom
                            );
                        if (SUCCEEDED(hr))
                        {
                            // Convert to bitmap for editing
                            if (*displayBitmap)
                                (*displayBitmap)->Release();
                            hr = _wicFactory->CreateBitmapFromSource(displayConverter, WICBitmapCacheOnLoad, displayBitmap);
                        }
                        displayConverter->Release();
                    }
                }

                hr = zoneInfo->Build(bwConverter);

                bwConverter->Release();
            }
        }
        decoder->Release();
    }

    return hr;
}