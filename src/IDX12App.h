#pragma once

class IDX12App abstract
{
public:

	virtual bool Initialize(const int count = 0, const float scale = 0.0f) = 0;

    virtual void Update() = 0;
    virtual void Draw() = 0;

    virtual void UpdateVirtualSphereAngles(const POINT mLastMousePos, const int x, const int y) = 0;
    virtual void UpdateVirtualSphereRadius(const POINT mLastMousePos, const int x, const int y) = 0;

    virtual ~IDX12App() = 0 {};
};