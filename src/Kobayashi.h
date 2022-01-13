#pragma once
#include "Win32App.h"// This includes ISimulation.h.
					  // Win32App is required in main().

class Kobayashi : public ISimulation
{
public:
	Kobayashi(int x, int y, float timeStep);
	~Kobayashi() override;

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

private :
	enum class _COM
	{
		PLAY, STOP, NEXTSTEP,
		TIME_TEXT, FRAME_TEXT,
		ANISO_VALUE, ANISO_BAR
	};

	clock_t _simTime = 0;
	int _simFrame = 0;

	std::vector<Vertex> _vertices;
	std::vector<unsigned int> _indices;
	DirectX::XMINT2 _objectCount = { 0, 0 };

	DX12App* _dxapp = nullptr;

	float _updateFlag = true;

	wchar_t wBuffer[5];
	wchar_t* _int2wchar(int value);

	int _scrollPos = 6;

	//
	inline int _INDEX(int i, int j) { return (i + _objectCount.x * j); };

	float _tauValue = 0.0003f;
	float _epsilonBarValue = 0.01f;
	float _muValue = 1.0f;
	float _KValue = 1.6f;
	float _deltaValue = 0.05f;
	float _anisotropyValue = 6.0f;
	float _alphaValue = 0.9f;
	float _gammaValue = 10.0f;
	float _tEqValue = 1.0f;

	float _dx, _dy, _dt,
		_tau, _epsilonBar, _mu, _K, _delta, 
		_anisotropy, _alpha, _gamma, _tEq;

	std::vector<float> _phi;
	std::vector<float> _t;
	std::vector<float> _epsilon;
	std::vector<float> _epsilonDeriv;
	std::vector<float> _gradPhiX;
	std::vector<float> _gradPhiY;
	std::vector<float> _lapPhi;
	std::vector<float> _lapT;
	std::vector<float> _angl;
	
	void _initialize();
	void _createNucleus(int x, int y);
	void _computeGradientLaplacian();
	void _evolution();
};
