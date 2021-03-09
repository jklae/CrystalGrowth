#pragma once
#include "Win32App.h"



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
    Win32App winApp(800, 600);
    winApp.Initialize(hInstance);
    winApp.InitDirectX();
    winApp.CreateObjects(200, 0.01f);

    return winApp.Run();
}