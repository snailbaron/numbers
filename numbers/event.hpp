#pragma once

#include <Windows.h>
#include <d2d1.h>

namespace Event {

enum class Type { NONE, RENDER, PUSH };

struct Base
{
    Base(const Type &atype = Type::NONE) : type(atype) { }
    virtual ~Base() { }
    Event::Type type;
};

struct Render : public Event::Base
{
    Render(ID2D1HwndRenderTarget *aRenderTarget) : Base(Type::RENDER), renderTarget(aRenderTarget) { }
    ID2D1HwndRenderTarget *renderTarget;
};

struct Push : public Event::Base
{
    Push(HWND aHwnd, ID2D1HwndRenderTarget *aRenderTarget, int ax, int ay) :
        Base(Type::PUSH), hwnd(aHwnd), renderTarget(aRenderTarget), x(ax), y(ay) { }

    HWND hwnd;
    ID2D1HwndRenderTarget *renderTarget;
    int x, y;
};

} // namespace Event