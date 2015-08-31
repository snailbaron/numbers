#pragma once

#include <Windows.h>
#include <d2d1.h>
#include "event_queue.hpp"

class GfxWindow
{
public:
    GfxWindow(HINSTANCE hInstance, LPCWSTR name, ID2D1Factory *d2dFactory);
    ~GfxWindow();

    const HINSTANCE & GetInstance() const { return _hInstance; }
    const HWND & GetHwnd() const { return _hwnd; }
    const WCHAR * GetName() const { return _name; }
    ID2D1HwndRenderTarget * GetRenderTarget() { return _renderTarget; }

    static HRESULT Register();
    HRESULT Initialize();

    void RegisterEventQueue(EventQueue *queue);

private:
    static const WCHAR _WND_CLASS_NAME[];

    static LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    LRESULT HandleMessage(UINT msg, WPARAM wparam, LPARAM lparam);
    HRESULT CreateRenderTarget();

private:
    static ATOM _wndClassAtom;

    HINSTANCE _hInstance;
    WCHAR *_name;
    ID2D1Factory *_d2dFactory;
    HWND _hwnd;
    ID2D1HwndRenderTarget *_renderTarget;
    EventQueue *_evtQueue;
};

