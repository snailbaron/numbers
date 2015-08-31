#pragma once

#include <Windows.h>
#include <d2d1.h>

namespace Event {

enum class Type { NONE, RENDER, PUSH };

struct Base
{
    virtual ~Base() { }
};

struct Render : public Base
{
    Render(ID2D1HwndRenderTarget *aRenderTarget) : renderTarget(aRenderTarget) { }

    ID2D1HwndRenderTarget *renderTarget;
};

struct Push : public Base
{
    Push(HWND aHwnd, ID2D1HwndRenderTarget *aRenderTarget, int ax, int ay) :
        hwnd(aHwnd), renderTarget(aRenderTarget), x(ax), y(ay) { }

    HWND hwnd;
    ID2D1HwndRenderTarget *renderTarget;
    int x, y;
};

struct Finish : public Base
{
    Finish() : Base() { }
};

} // namespace Evt