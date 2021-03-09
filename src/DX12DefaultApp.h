#pragma once

#include "dx12header.h"
#include "IDX12App.h"


class DX12DefaultApp : public IDX12App
{
protected:

    const int kWidth;
    const int kHeight;

    const HWND mhMainWnd; // main window handle



#pragma region Initialization
    // ########################################## Init ##########################################
    // CheckMSAA
    // Set true to use 4X MSAA (4.1.8).  The default is false.
    bool m4xMsaaState = false;    // 4X MSAA enabled
    UINT m4xMsaaQuality = 0;      // quality level of 4X MSAA

    // CreateDevice
    Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
    Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;

    // CreateFence
    Microsoft::WRL::ComPtr<ID3D12Fence> mFence;

    // CreateCommandQueueAllocatorList
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

    // CreateSwapChain
    const DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    static const int SwapChainBufferCount = 2; // 'static' is required when declared in a class.
    Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;

    // CreateDescriptorHeap
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;

    // CreateRTV
    UINT mRtvDescriptorSize;
    Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];

    // CreateDSV
    Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;
    DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT; // This is reused in pso.

    // SetViewport
    D3D12_VIEWPORT mScreenViewport;

    // SetScissorRectangle
    D3D12_RECT mScissorRect;

    void CheckMSAA();
    void CreateDevice();
    void CreateFence();
    void CreateCommandQueueAllocatorList();
    void CreateSwapChain();
    void CreateDescriptorHeap();
    void CreateRTV();
    void CreateDSV();
    void SetViewport();
    void SetScissorRectangle();

    // ##########################################################################################
#pragma endregion



    // ########################################## Draw ##########################################
    UINT64 mCurrentFence = 0;
    int mCurrBackBuffer = 0;

    // CreatePSO (DirectXDrawingApp's)
    Microsoft::WRL::ComPtr<ID3D12PipelineState> mPSO = nullptr;

    void FlushCommandQueue();
    void CloseCommandList();
    // ##########################################################################################


    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;
    ID3D12Resource* CurrentBackBuffer() const;
    D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;


public:
    DX12DefaultApp(const int kWidth, const int kHeight, const HWND mhMainWnd);
    ~DX12DefaultApp() override;

    bool Initialize(const int count = 0, const float scale = 0.0f) override;

    void Update() override {};
    void Draw() override;

    void UpdateVirtualSphereAngles(const POINT mLastMousePos, const int x, const int y) override {};
    void UpdateVirtualSphereRadius(const POINT mLastMousePos, const int x, const int y) override {};
};