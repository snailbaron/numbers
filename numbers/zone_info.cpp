#include "zone_info.hpp"
#include "utils.hpp"
#include <queue>
#include <list>
#include <tuple>
#include <algorithm>
#include <ctime>

ZoneInfo::ZoneInfo() :
    _zoneMap(nullptr),
    _zoneMapHeight(0),
    _zoneMapWidth(0)
{
}

ZoneInfo::~ZoneInfo()
{
    if (_zoneMap)
        Free2D(_zoneMap);

    _zoneCenters.clear();
    _colorOrder.clear();
}

HRESULT ZoneInfo::Build(IWICBitmapSource *source)
{
    HRESULT hr = S_OK;

    // Check pixel format for provided bitmap source. Only working with 1bpp BW images.
    WICPixelFormatGUID bmPixelFormat;
    hr = source->GetPixelFormat(&bmPixelFormat);
    if (FAILED(hr)) return hr;

    if (!IsEqualGUID(bmPixelFormat, GUID_WICPixelFormatBlackWhite))
        return E_FAIL;

    UINT bmWidth, bmHeight;
    hr = source->GetSize(&bmWidth, &bmHeight);
    if (FAILED(hr)) return hr;
    
    UINT stride = (bmWidth + 7) / 8;
    UINT bmBufferSize = stride * bmHeight;
    BYTE *bmBuffer = new BYTE[bmBufferSize];
    hr = source->CopyPixels(NULL, stride, bmBufferSize, bmBuffer);
    if (SUCCEEDED(hr))
    {
        // Create a clean zone map for this bitmap
        if (_zoneMap) Free2D<UINT>(_zoneMap);
        _zoneMap = Allocate2D<UINT>(bmHeight, bmWidth);
        _zoneMapHeight = bmHeight;
        _zoneMapWidth = bmWidth;

        // Create accompanying zone information data
        std::list<std::tuple<UINT64, UINT64, UINT>> zoneInfos;

        // Build zone map (currently a simple BFS)
        UINT colorCount = 0;
        for (UINT i = 0; i < bmHeight; i++)
        {
            for (UINT j = 0; j < bmWidth; j++)
            {
                BYTE topFrameBit = (bmBuffer[i * stride + j / 8] & (1 << (7 - j % 8))) >> (7 - j % 8);
                if (!topFrameBit)
                {
                    _zoneMap[i][j] = -1;
                    continue;
                }

                if (_zoneMap[i][j])
                    continue;

                colorCount++;
                zoneInfos.push_back(std::make_tuple(0, 0, 0));

                std::queue<std::tuple<UINT, UINT>> q;
                q.push(std::make_tuple(i, j));
                while (!q.empty())
                {
                    UINT k, l;
                    std::tie (k, l) = q.front();
                    q.pop();

                    // Ignore out-of-bitmap pixels
                    if (k < 0 || k >= bmHeight || l < 0 || l >= bmWidth)
                        continue;

                    // Ignore pixels belonging to borders
                    BYTE bmBit = (bmBuffer[k * stride + l / 8] & (1 << (7 - l % 8))) >> (7 - l % 8);
                    if (!bmBit)
                    {
                        _zoneMap[k][l] = -1;
                        continue;
                    }

                    // Ignore already processed pixels
                    if (_zoneMap[k][l])
                        continue;

                    _zoneMap[k][l] = colorCount;
                    std::get<0>(zoneInfos.back()) += k;
                    std::get<1>(zoneInfos.back()) += l;
                    std::get<2>(zoneInfos.back())++;

                    q.push(std::make_tuple(k - 1, l));
                    q.push(std::make_tuple(k + 1, l));
                    q.push(std::make_tuple(k, l - 1));
                    q.push(std::make_tuple(k, l + 1));
                }
            }
        }

        // Calculate zone centers to display numbers
        _zoneCenters.clear();
        std::for_each(zoneInfos.begin(), zoneInfos.end(), [&](const std::tuple<UINT64, UINT64, UINT> &t) {
            UINT64 hSum, wSum;
            UINT count;
            std::tie(hSum, wSum, count) = t;
            _zoneCenters.push_back(std::make_tuple(hSum / count, wSum / count));
        });

        // Mix available colors in a random sequence
        _colorOrder.clear();
        for (UINT i = 0; i < colorCount; i++)
            _colorOrder.push_back(i + 1);

        srand((unsigned int) time(nullptr));
        for (UINT i = 0; i < colorCount; i++)
        {
            UINT r = rand() % colorCount;
            UINT temp = _colorOrder[i];
            _colorOrder[i] = _colorOrder[r];
            _colorOrder[r] = temp;
        }
    }

    delete[] bmBuffer;

    return hr;
}