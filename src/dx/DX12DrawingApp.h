#pragma once
#include "DX12DefaultApp.h"
#include "../sim/Kobayashi.h"

class DX12DrawingApp : public DX12DefaultApp
{
private:

    std::unique_ptr<Kobayashi> kob = nullptr;


    struct ConstantBuffer
    {
        DirectX::XMFLOAT4X4 worldViewProj;
        DirectX::XMFLOAT4 color;
    };

    std::vector<ConstantBuffer> constantBuffer;
    std::vector<DirectX::XMFLOAT4X4> mWorld;



    struct Vertex
    {
        DirectX::XMFLOAT3 Pos;
    };


    // CreateVertexIndexBuffer
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
        const void* initData, UINT64 byteSize,
        Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);

    Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

    D3D12_VERTEX_BUFFER_VIEW vbv;
    D3D12_INDEX_BUFFER_VIEW ibv;
    UINT IndexCount = 0;


    // CreateConstantBuffer
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer = nullptr;
    BYTE* mMappedData = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

    // CompileShader
    Microsoft::WRL::ComPtr<ID3DBlob> mvsByteCode = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> mpsByteCode = nullptr;
    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;


    DirectX::XMFLOAT4X4 mView = TransformMatrix(0.0f, 0.0f, 0.0f);
    DirectX::XMFLOAT4X4 mProj = TransformMatrix(0.0f, 0.0f, 0.0f);



    float mTheta = 1.5f * 3.14f;
    float mPhi = 3.14f / 2.0f;
    float mRadius = 5.0f;

    float Clamp(const float x, const float low, const float high);

    void CreateProjMatrix();
    void CreateVertexIndexBuffer();
    //
    void CreateConstantBuffer();
    void CreateConstantBufferViewHeap();
    void CreateUploadBuffer();
    void CreateConstantBufferViews();
    void CreateRootSignature();
    //
    void CompileShader();
    void CreatePSO();

public:
    DX12DrawingApp(const int kWidth, const int kHeight, const HWND mhMainWnd);
    ~DX12DrawingApp() final;

    bool Initialize(const int count = 0, const float scale = 0.0f) final;

    void Update(DirectX::XMFLOAT4 color) final;
    void Draw() final;

    void UpdateVirtualSphereAngles(const POINT mLastMousePos, const int x, const int y) final;
    void UpdateVirtualSphereRadius(const POINT mLastMousePos, const int x, const int y) final;


    void CreateObjects(const int count, const float scale);
};

