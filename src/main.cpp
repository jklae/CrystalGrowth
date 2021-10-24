#pragma once
// Console window is displayed in debug mode.
//#ifdef _DEBUG
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
//#endif

#include "Kobayashi.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
    // Simulation init
    int x = 100;
    int y = 0;
    float timeStep = 0.0001f;

    Kobayashi* fluidsim = new Kobayashi(x, y, timeStep);

    // DirectX init
    DX12App* dxapp = new DX12App();
    dxapp->setProjectionType(PROJ::ORTHOGRAPHIC);

    // Window init
    Win32App winApp(800, 800);
    winApp.initialize(hInstance, dxapp, fluidsim);

    return winApp.run();
}