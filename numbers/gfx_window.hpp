#pragma once

#include <Windows.h>
#include <d2d1.h>
#include <string>

class GfxWindow
{
public:
    GfxWindow(HINSTANCE hInstance, LPCWSTR name, ID2D1Factory *d2dFactory);
    ~GfxWindow();

    static HRESULT Register();
    HRESULT Initialize();

private:
    static const WCHAR _WND_CLASS_NAME[];
    static bool _classRegistered;

    static LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    HRESULT CreateRenderTarget();

    HINSTANCE _hInstance;
    std::wstring _name;
    ID2D1Factory *_d2dFactory;
    HWND _hwnd;
    ID2D1HwndRenderTarget *_renderTarget;
    
};
