#include "DX12DrawingApp.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

DX12DrawingApp::DX12DrawingApp(const int kWidth, const int kHeight, const HWND mhMainWnd)
	: DX12DefaultApp(kWidth, kHeight, mhMainWnd)
{	
}

DX12DrawingApp::~DX12DrawingApp()
{	
	// CreateVertexIndexBuffer
	VertexBufferUploader = nullptr;
	IndexBufferUploader = nullptr;


	// CreateConstantBuffer
	mMappedData = nullptr;

	if (mUploadBuffer != nullptr)
		mUploadBuffer->Unmap(0, nullptr);
}

void DX12DrawingApp::CreateObjects(const int count, const float scale)
{
	const int totalCount = static_cast<size_t>(count * count * count);
	constantBuffer.reserve(totalCount);
	mWorld.reserve(totalCount);

	const float stride = scale * 2.5f;
	const float offset = -(stride * count) / 2.0f;
	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < count; j++)
		{
			XMFLOAT3 pos = XMFLOAT3(
				offset + (float)i * stride,
				offset + (float)j * stride,
				0.0f);

			XMFLOAT4X4 world = TransformMatrix(pos.x, pos.y, pos.z, scale);
			mWorld.push_back(world);

			struct ConstantBuffer cb;
			cb.color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			cb.worldViewProj = TransformMatrix(0.0f, 0.0f, 0.0f);

			constantBuffer.push_back(cb);
			
		}
	}
}


bool DX12DrawingApp::Initialize(const int count, const float scale)
{
	DX12DefaultApp::Initialize();

	CreateObjects(count, scale);

	CreateProjMatrix();
	CreateVertexIndexBuffer();
	CreateConstantBuffer();
	CompileShader();
	CreatePSO();

	CloseCommandList();

	return 1;
}

#pragma region Initialization

void DX12DrawingApp::CreateProjMatrix()
{
	// Compute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * 3.14f, static_cast<float>(kWidth) / kHeight, 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void DX12DrawingApp::CreateVertexIndexBuffer()
{
	// 2, 3
	std::vector<Vertex> vertices =
	{
		Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f) }) //, XMFLOAT4(Colors::Black)
	};

	std::vector<std::uint16_t> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};


	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	VertexBufferGPU = CreateDefaultBuffer(vertices.data(), vbByteSize, VertexBufferUploader);
	IndexBufferGPU = CreateDefaultBuffer(indices.data(), ibByteSize, IndexBufferUploader);

	vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
	vbv.StrideInBytes = sizeof(Vertex);
	vbv.SizeInBytes = vbByteSize;

	ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
	ibv.Format = DXGI_FORMAT_R16_UINT;
	ibv.SizeInBytes = ibByteSize;

	IndexCount = (UINT)indices.size();
}

void DX12DrawingApp::CreateConstantBuffer()
{
	// 6
	CreateConstantBufferViewHeap();
	CreateUploadBuffer();
	CreateConstantBufferViews();
	CreateRootSignature();
}

void DX12DrawingApp::CreateConstantBufferViewHeap()
{
	// 6-4
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = mWorld.size();
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // The shader program will access this descriptor.
	cbvHeapDesc.NodeMask = 0;
	md3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mCbvHeap));
}

void DX12DrawingApp::CreateUploadBuffer()
{
	UINT mElementByteSize = ComputeBufferByteSize<ConstantBuffer>();

	md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * mWorld.size()), D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&mUploadBuffer));

	mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData));
}

void DX12DrawingApp::CreateConstantBufferViews()
{
	UINT objCBByteSize = ComputeBufferByteSize<ConstantBuffer>();
	for (int i = 0; i < mWorld.size(); i++)
	{
		//obj.CreateConstantBuffer(mCbvHeap, i, counts);
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(i, md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mUploadBuffer->GetGPUVirtualAddress();
		cbAddress += i * objCBByteSize;

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = objCBByteSize;

		md3dDevice->CreateConstantBufferView(&cbvDesc, handle);
	}
}

void DX12DrawingApp::CreateRootSignature()
{
	// 6-5
	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.  

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), 0);

	md3dDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));
}

void DX12DrawingApp::CompileShader()
{
	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3DCompileFromFile(L"shader\\vertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &mvsByteCode, 0);
	D3DCompileFromFile(L"shader\\fragShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &mpsByteCode, 0);
}

void DX12DrawingApp::CreatePSO()
{
	// 8, 9
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()), mvsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()), mpsByteCode->GetBufferSize()
	};
	// 8
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = mDepthStencilFormat;
	md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO));
}


#pragma endregion


void DX12DrawingApp::Update()
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);

	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	UINT mElementByteSize = ComputeBufferByteSize<ConstantBuffer>();

	for (int i = 0; i < mWorld.size(); i++)
	{
		XMMATRIX world = XMLoadFloat4x4(&mWorld[i]);
		XMMATRIX worldViewProj = world * view * proj;

		// Update the constant buffer with the latest worldViewProj matrix.
		XMStoreFloat4x4(&constantBuffer[i].worldViewProj, XMMatrixTranspose(worldViewProj));
		memcpy(&mMappedData[i * mElementByteSize], &constantBuffer[i].worldViewProj, sizeof(ConstantBuffer));

	}
}

void DX12DrawingApp::Draw()
{
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());



	//
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	mCommandList->IASetVertexBuffers(0, 1, &vbv);
	mCommandList->IASetIndexBuffer(&ibv);
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	for (int i = 0; i < mWorld.size(); i++)
	{
		auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
		cbvHandle.Offset(i, md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

		mCommandList->SetGraphicsRootDescriptorTable(0, cbvHandle);
		mCommandList->DrawIndexedInstanced(IndexCount, 1, 0, 0, 0);
	}

	//




	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	CloseCommandList();


	// swap the back and front buffers
	mSwapChain->Present(0, 0);
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
}







void DX12DrawingApp::UpdateVirtualSphereAngles(const POINT mLastMousePos, const int x, const int y)
{
	// Make each pixel correspond to a quarter of a degree.
	float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
	float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

	// Update angles based on input to orbit camera around box.
	mTheta -= dx;
	mPhi -= dy;

	// Restrict the angle mPhi.
	mPhi = Clamp(mPhi, 0.1f, 3.14f - 0.1f);
}


void DX12DrawingApp::UpdateVirtualSphereRadius(const POINT mLastMousePos, const int x, const int y)
{
	// Make each pixel correspond to 0.005 unit in the scene.
	float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
	float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

	// Update the camera radius based on input.
	mRadius += dx - dy;

	// Restrict the radius.
	mRadius = Clamp(mRadius, 3.0f, 15.0f);
}


float DX12DrawingApp::Clamp(const float x, const float low, const float high)
{
	return x < low ? low : (x > high ? high : x);
}


ComPtr<ID3D12Resource> DX12DrawingApp::CreateDefaultBuffer(
	const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource>& uploadBuffer)
{
	ComPtr<ID3D12Resource> defaultBuffer;

	// Create the actual default buffer resource.
	md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize), D3D12_RESOURCE_STATE_COMMON,
		nullptr, IID_PPV_ARGS(defaultBuffer.GetAddressOf()));

	// In order to copy CPU memory data into our default buffer, we need to create
	// an intermediate """upload""" heap. 
	md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize), D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(uploadBuffer.GetAddressOf()));


	// Describe the data we want to copy into the default buffer.
	// Data material for copying?
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;


	// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
	// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
	// the intermediate upload heap data will be copied to mBuffer.

	// 1. transit defaultBuffer to copy mode
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	// 2. Copy
	UpdateSubresources<1>(mCommandList.Get(), defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
	// 3. reset mode
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));


	// Note: uploadBuffer has to be kept alive after the above function calls because
	// the command list has not been executed yet that performs the actual copy.
	// The caller can Release the uploadBuffer after it knows the copy has been executed.


	return defaultBuffer;
}