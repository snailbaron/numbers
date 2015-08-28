#pragma once

#include <Windows.h>
#include <wincodec.h>
#include <vector>
#include <tuple>

class ZoneInfo
{
public:
    ZoneInfo();
    ~ZoneInfo();

    const UINT & GetHeight() const { return _zoneMapHeight; }
    const UINT & GetWidth() const { return _zoneMapWidth; }
    const UINT & GetColorCount() const { return _zoneCenters.size(); }
    const std::tuple<UINT, UINT> & GetZoneCenter(const UINT &zoneNumber) const { return _zoneCenters[_colorOrder[zoneNumber]]; }

    HRESULT Build(IWICBitmapSource *source);

private:
    UINT **_zoneMap;
    UINT _zoneMapHeight;
    UINT _zoneMapWidth;
    std::vector<std::tuple<UINT, UINT>> _zoneCenters;
    std::vector<UINT> _colorOrder;
};