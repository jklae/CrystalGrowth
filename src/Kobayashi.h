#pragma once
#include "Win32App.h"// This includes ISimulation.h.
					  // Win32App is required in main().

class Kobayashi : public ISimulation
{
public:
#pragma region Implementation
	// ################################## Implementation ####################################
	// Simulation methods
	void iUpdate() override;
	void iResetSimulationState(std::vector<ConstantBuffer>& constantBuffer) override;

	// Mesh methods
	std::vector<Vertex>& iGetVertice() override;
	std::vector<unsigned int>& iGetIndice() override;
	UINT iGetVertexBufferSize() override;
	UINT iGetIndexBufferSize() override;

	// DirectX methods
	void iCreateObject(std::vector<ConstantBuffer>& constantBuffer) override;
	void iUpdateConstantBuffer(std::vector<ConstantBuffer>& constantBuffer, int i) override;
	void iDraw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& mCommandList, int size, UINT indexCount, int i) override;
	void iSetDXApp(DX12App* dxApp) override;
	UINT iGetConstantBufferSize() override;
	DirectX::XMINT3 iGetObjectCount() override;
	DirectX::XMFLOAT3 iGetObjectSize() override;
	DirectX::XMFLOAT3 iGetObjectPositionOffset() override;
	bool iIsUpdated() override;

	// WndProc methods
	void iWMCreate(HWND hwnd, HINSTANCE hInstance) override;
	void iWMCommand(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance) override;
	void iWMHScroll(HWND hwnd, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance) override;
	void iWMTimer(HWND hwnd) override;
	void iWMDestory(HWND hwnd) override;
	// #######################################################################################
#pragma endregion

	std::vector<float> _x;
	std::vector<float> _y;
	std::vector<float> _phi;

	Kobayashi(int x, int y, float timeStep);

	void update();

private :
	inline int _INDEX(int i, int j) { return (i + _objectCount.x * j); };

	std::vector<Vertex> _vertices;
	std::vector<unsigned int> _indices;

	DX12App* _dxapp = nullptr;

	DirectX::XMINT2 _objectCount = { 0, 0 };

	float _dx;
	float _dy;
	float _dt;
	float tau;
	float epsilonBar;
	float mu;
	float K;
	float delta;
	float anisotropy;
	float alpha;
	float gamma;
	float tEq;

	std::vector<float> _t;
	std::vector<float> _epsilon;
	std::vector<float> _epsilonDeriv;
	std::vector<float> _gradPhiX;
	std::vector<float> _gradPhiY;
	std::vector<float> _lapPhi;
	std::vector<float> _lapT;
	std::vector<float> _angl;
	
	void computeGradLap();
	void evolution();
	void createNuclei(int transX, int transY);
};
