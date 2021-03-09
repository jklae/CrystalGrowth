#pragma once

#include <cassert>
#include <WindowsX.h>
#include <typeinfo>

#include "dx/DX12DrawingApp.h" // this includes IDX12App, DX12DefaultApp.h

class Win32App
{
private:
    static Win32App* instanceForProc;

    // Convenience overrides for handling mouse input.
    void OnMouseDown(WPARAM btnState, int x, int y);
    void OnMouseUp(WPARAM btnState, int x, int y);
    void OnMouseMove(WPARAM btnState, int x, int y);

    bool InitMainWindow(HINSTANCE hInstance);

    const int kWidth;
    const int kHeight;

    HWND mhMainWnd = nullptr; // main window handle

    void Update();
    void Draw();


    std::unique_ptr<IDX12App> dxApp = nullptr;


    POINT mLastMousePos;

public:
    Win32App(const int kWidth, const int KHeight);
    ~Win32App();

    static Win32App* GetinstanceForProc();
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool Initialize(HINSTANCE hInstance);
	int Run();
    void InitDirectX();
    void CreateObjects(const int count, const float scale);
};

