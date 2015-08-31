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
    const UINT GetZoneNumber(UINT x, UINT y) const { return _zoneMap[y][x]; }

    HRESULT Build(IWICBitmapSource *source);

private:
    Matrix2D<int> _zoneMap;
    std::vector<std::tuple<UINT, UINT>> _zoneCenters;
    std::vector<UINT> _colorOrder;
};