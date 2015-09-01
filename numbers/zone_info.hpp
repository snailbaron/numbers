#pragma once

#include <Windows.h>
#include <wincodec.h>
#include <vector>
#include <tuple>
#include "matrix.hpp"

class ZoneInfo
{
public:
    ZoneInfo();
    ~ZoneInfo();

    const UINT GetHeight() const { return _zoneMap.GetISize(); }
    const UINT GetWidth() const { return _zoneMap.GetJSize(); }
    const UINT GetColorCount() const { return _zoneCenters.size(); }
    const std::tuple<UINT, UINT> & GetZoneCenter(const UINT &zoneNumber) const;
    const int GetZoneNumber(UINT x, UINT y) const { return _zoneMap[y][x]; }
    const UINT GetColor(UINT i) const { return _colorOrder[i-1]; }

    HRESULT Build(IWICBitmapSource *source);

private:
    Matrix2D<int> _zoneMap;
    std::vector<std::tuple<UINT, UINT>> _zoneCenters;
    std::vector<UINT> _colorOrder;

private:
    struct ColorStats
    {
        ColorStats(UINT acolor, UINT width, UINT height) :
            color(acolor), count(0), hSum(0), wSum(0), left(width - 1), right(0), top(height - 1), bottom(0)
        { }

        // Color
        UINT color;

        // Number of pixels of this color
        UINT count;

        // Sums of h and w coordinates of pixels of this color
        UINT64 hSum;
        UINT64 wSum;

        // Borders of pixels of this color
        UINT left;
        UINT right;
        UINT top;
        UINT bottom;
    };
};