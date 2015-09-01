#include "zone_info.hpp"
#include "utils.hpp"
#include <queue>
#include <list>
#include <tuple>
#include <algorithm>
#include <ctime>
#include "bitmap_buffer.hpp"

ZoneInfo::ZoneInfo()
{ }

ZoneInfo::~ZoneInfo()
{ }

const std::tuple<UINT, UINT> & ZoneInfo::GetZoneCenter(const UINT &zoneNumber) const
{
    return _zoneCenters[zoneNumber - 1];
}

/**
    Build a zone map for a given bitmap source.
    Only works with 1bpp black & white images.
 */
HRESULT ZoneInfo::Build(IWICBitmapSource *source)
{
    HRESULT hr = S_OK;

    BitmapBuffer bmBuffer;
    hr = bmBuffer.Load(source);
    if (FAILED(hr)) return hr;

    // Create a clean zone map for this bitmap
    UINT bmHeight = bmBuffer.GetHeight();
    UINT bmWidth = bmBuffer.GetWidth();
    _zoneMap = Matrix2D<int>(bmHeight, bmWidth);

    // Create accompanying zone information data
    std::list<ZoneInfo::ColorStats> zoneInfos;

    // Build zone map (currently a simple BFS)
    UINT colorCount = 0;
    for (UINT i = 0; i < bmHeight; i++)
    {
        for (UINT j = 0; j < bmWidth; j++)
        {
            // Mark separating lines as -1 in zone map
            if (!bmBuffer.GetPixel(j, i))
            {
                _zoneMap[i][j] = -1;
                continue;
            }

            // Move along, if this pixel is already processed
            if (_zoneMap[i][j])
                continue;

            // Allocate new color, and add statistics for it
            colorCount++;
            zoneInfos.push_back(ZoneInfo::ColorStats(colorCount, bmWidth, bmHeight));

            // Begin BFS from current pixel
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
                if (!bmBuffer.GetPixel(l, k))
                {
                    _zoneMap[k][l] = -1;
                    continue;
                }

                // Ignore already processed pixels
                if (_zoneMap[k][l])
                    continue;

                // Mark pixel with selected color
                _zoneMap[k][l] = colorCount;

                // Add this pixel's data to color's statistics
                ZoneInfo::ColorStats &stats = zoneInfos.back();
                stats.count++;
                stats.hSum += k;
                stats.wSum += l;
                if (k < stats.top) stats.top = k;
                if (k > stats.bottom) stats.bottom = k;
                if (l < stats.left) stats.left = l;
                if (l > stats.right) stats.right = l;

                // Process neighboring pixels next
                q.push(std::make_tuple(k - 1, l));
                q.push(std::make_tuple(k + 1, l));
                q.push(std::make_tuple(k, l - 1));
                q.push(std::make_tuple(k, l + 1));
            }
        }
    }

    // Calculate zone centers to display numbers
    _zoneCenters.clear();
    srand((unsigned int)time(nullptr));
    std::for_each(zoneInfos.begin(), zoneInfos.end(), [&](const ZoneInfo::ColorStats &stats) {
        UINT ch = (UINT) (stats.hSum / stats.count);
        UINT cw = (UINT) (stats.wSum / stats.count);

        for (int i = 0; i < 100; i++)
        {
            UINT h = FlatRand(stats.top + 10, stats.bottom - 10);
            UINT w = FlatRand(stats.left + 10, stats.right - 10);
            if (_zoneMap[h][w] == stats.color && _zoneMap[h-10][w] == stats.color && _zoneMap[h+10][w] == stats.color && _zoneMap[h][w-10] == stats.color && _zoneMap[h][w+10] == stats.color)
            {
                ch = h;
                cw = w;
                break;
            }
        }

        _zoneCenters.push_back(std::make_tuple(ch, cw));
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

    return hr;
}
