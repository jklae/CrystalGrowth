#pragma once
// Console window is displayed in debug mode.
#ifdef _DEBUG
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

#include "Kobayashi.h"

using namespace DXViewer::xmint3;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
    // Simulation init
    int x = 250;
    int y = x;
    float timeStep = 0.0001f;

    Kobayashi* crystalsim = new Kobayashi(x, y, timeStep);

    // DirectX init
    DX12App* dxapp = new DX12App();
    dxapp->setCameraProperties(
        PROJ::ORTHOGRAPHIC,
        static_cast<float>(max_element(crystalsim->iGetObjectCount())) * 0.0022f, // orthogonal distance
        2.0f, 0.0f, 0.0f                                                       // radius, theta, phi
    );
    //dxapp->setCameraProperties(PROJ::PERSPECTIVE, 1.3f, 0.0f, 0.0f);
    dxapp->setBackgroundColor(DirectX::Colors::Black);
    //dxapp->setLightPosition(1.0f, 1.0f, 1.0f);

    // Window init
    Win32App winApp(500, 500);
    winApp.setWinName(L"Crystal Simulation");
    winApp.setWinOffset(400, 300);
    winApp.initialize(hInstance, dxapp, crystalsim);

    return winApp.run();
}