#pragma once

#ifndef DX12HEADER_H
#define DX12HEADER_H


// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

#include <vector>
#include <memory>
#include <wrl.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXColors.h>

#include "../extern/d3dx12.h"


inline DirectX::XMFLOAT4X4 TransformMatrix(
    const float x, const float y, const float z, 
    const float scale = 1.0f)
{
    return DirectX::XMFLOAT4X4(
        scale, 0.0f, 0.0f, 0.0f,
        0.0f, scale, 0.0f, 0.0f,
        0.0f, 0.0f, scale, 0.0f,
        x, y, z, 1.0f);
}

template <typename T>
inline UINT ComputeBufferByteSize()
{
	// Constant buffers must be a multiple of the minimum hardware
	// allocation size (usually 256 bytes).  So round up to nearest
	// multiple of 256.  We do this by adding 255 and then masking off
	// the lower 2 bytes which store all bits < 256.
	// Example: Suppose byteSize = 300.
	// (300 + 255) & ~255
	// 555 & ~255
	// 0x022B & ~0x00ff
	// 0x022B & 0xff00
	// 0x0200
	// 512
	return (sizeof(T) + 255) & ~255;
}


#endif