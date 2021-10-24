#pragma once
#include "Win32App.h"// This includes ISimulation.h.
					  // Win32App is required in main().

class Kobayashi : public ISimulation
{
public:
#pragma region Implementation
	// ################################## Implementation ####################################
	void iUpdate() override;
	void iResetSimulationState(std::vector<ConstantBuffer>& constantBuffer) override;

	std::vector<Vertex> iGetVertice() override;
	std::vector<unsigned int> iGetIndice() override;
	int iGetObjectCount() override;

	void iCreateObjectParticle(std::vector<ConstantBuffer>& constantBuffer) override;
	void iWMCreate(HWND hwnd, HINSTANCE hInstance) override;
	void iWMCommand(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance, bool& updateFlag, DX12App* dxapp) override;
	void iWMHScroll(HWND hwnd, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance, DX12App* dxapp) override;
	void iWMTimer(HWND hwnd) override;
	void iWMDestory(HWND hwnd) override;

	void iUpdateConstantBuffer(std::vector<ConstantBuffer>& constantBuffer, int i) override;
	void iDraw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& mCommandList, int size, UINT indexCount, int i) override;
	// #######################################################################################
#pragma endregion

	int _nx;
	int _ny;
	std::vector<double> _x;
	std::vector<double> _y;
	std::vector<std::vector<double>> _phi;

	Kobayashi(int nx, int ny, double spacing);

	void update();

private :
	float _objectCount = 60;

	const double pi = 3.1415926535;

	double _dx;
	double _dy;
	double _dt;
	double tau;
	double epsilonBar;
	double mu;
	double K;
	double delta;
	double anisotropy;
	double alpha;
	double gamma;
	double tEq;

	std::vector<std::vector<double>> _t;
	std::vector<std::vector<double>> _epsilon;
	std::vector<std::vector<double>> _epsilonDeriv;
	std::vector<std::vector<double>> _gradPhiX;
	std::vector<std::vector<double>> _gradPhiY;
	std::vector<std::vector<double>> _lapPhi;
	std::vector<std::vector<double>> _lapT;
	std::vector<std::vector<double>> _angl;

	void initVector2D(std::vector<std::vector<double>>& vec2D);
	void computeGradLap(int start, int end);
	void evolution();
	void printParam(std::vector<std::vector<double>>& vectemp, const char* a, bool exp) const;
	void createNuclei(int transX, int transY);
};
