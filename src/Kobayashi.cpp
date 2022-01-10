#include "Kobayashi.h"

using namespace std;
using namespace DirectX;

Kobayashi::Kobayashi(int x, int y, float timeStep)
{
	_objectCount = { x, y };

	//
	_dx = 0.03f;
	_dy = 0.03f;
	_dt = timeStep;
	_tau = 0.0003f;
	_epsilonBar = 0.01f;		// Mean of epsilon. scaling factor that determines how much the microscopic front is magnified
	_mu = 1.0f;
	_K = 1.6f;				// Latent heat 
	_delta = 0.05f;			// Strength of anisotropy (speed of growth in preferred directions)
	_anisotropy = 6.0f;		// Degree of anisotropy
	_alpha = 0.9f;
	_gamma = 10.0f;
	_tEq = 1.0f;
	//

	size_t vSize = static_cast<size_t>(_objectCount.x) * static_cast<size_t>(_objectCount.y);
	_x.assign(vSize, 0.0f);
	_y.assign(vSize, 0.0f);
	_phi.assign(vSize, 0.0f);

	_t.assign(vSize, 0.0f);
	_gradPhiX.assign(vSize, 0.0f);
	_gradPhiY.assign(vSize, 0.0f);
	_lapPhi.assign(vSize, 0.0f);
	_lapT.assign(vSize, 0.0f);
	_angl.assign(vSize, 0.0f);
	_epsilon.assign(vSize, 0.0f);
	_epsilonDeriv.assign(vSize, 0.0f);

	// Set the position
	for (int i = 0; i < _objectCount.x; i++)
	{
		_x[i] = i - _objectCount.x / 2.0;
	}

	for (int j = 0; j < _objectCount.y; j++)
	{
		_y[j] = j - _objectCount.y / 2.0;
	}

	// Create the neuclei
	_createNucleus(_objectCount.x / 2 , _objectCount.y / 2);
}

Kobayashi::~Kobayashi()
{
}

void Kobayashi::_createNucleus(int x, int y)
{
	_phi[_INDEX(x, y)] = 1.0f;
	_phi[_INDEX(x - 1, y)] = 1.0f;
	_phi[_INDEX(x + 1, y)] = 1.0f;
	_phi[_INDEX(x, y - 1)] = 1.0f;
	_phi[_INDEX(x, y + 1)] = 1.0f;
}

void Kobayashi::_computeGradientLaplacian()
{

	for (int j = 0; j < _objectCount.y; j++)
	{
		for (int i = 0; i < _objectCount.x; i++)
		{

			int i_plus = (i + 1) % _objectCount.x;
			int i_minus = ((i - 1) + _objectCount.x) % _objectCount.x;
			int j_plus = (j + 1) % _objectCount.y;
			int j_minus = ((j - 1) + _objectCount.y) % _objectCount.y;


			_gradPhiX[_INDEX(i, j)] = (_phi[_INDEX(i_plus, j)] - _phi[_INDEX(i_minus, j)]) / _dx;
			_gradPhiY[_INDEX(i, j)] = (_phi[_INDEX(i, j_plus)] - _phi[_INDEX(i, j_minus)]) / _dy;

			_lapPhi[_INDEX(i, j)] = 
				(2.0f * (_phi[_INDEX(i_plus, j)] + _phi[_INDEX(i_minus, j)] + _phi[_INDEX(i, j_plus)] + _phi[_INDEX(i, j_minus)])
				+ _phi[_INDEX(i_plus, j_plus)] + _phi[_INDEX(i_minus, j_minus)] + _phi[_INDEX(i_minus, j_plus)] + _phi[_INDEX(i_plus, j_minus)]
				- 12.0f * _phi[_INDEX(i, j)])
				/ (3.0f * _dx * _dx);
			_lapT[_INDEX(i, j)] = 
				(2.0f * (_t[_INDEX(i_plus, j)] + _t[_INDEX(i_minus, j)] + _t[_INDEX(i, j_plus)] + _t[_INDEX(i, j_minus)])
				+ _t[_INDEX(i_plus, j_plus)] + _t[_INDEX(i_minus, j_minus)] + _t[_INDEX(i_minus, j_plus)] + _t[_INDEX(i_plus, j_minus)]
				- 12.0f * _t[_INDEX(i, j)])
				/ (3.0f * _dx * _dx);


			if (_gradPhiX[_INDEX(i, j)] <= +EPS_F && _gradPhiX[_INDEX(i, j)] >= -EPS_F) // _gradPhiX[i][j] == 0.0f
				if (_gradPhiY[_INDEX(i, j)] < -EPS_F)
					_angl[_INDEX(i, j)] = -0.5f * PI_F;
				else if (_gradPhiY[_INDEX(i, j)] > +EPS_F)
					_angl[_INDEX(i, j)] = 0.5f * PI_F;

			if (_gradPhiX[_INDEX(i, j)] > +EPS_F)
				if (_gradPhiY[_INDEX(i, j)] < -EPS_F)
					_angl[_INDEX(i, j)] = 2.0f * PI_F + atan(_gradPhiY[_INDEX(i, j)] / _gradPhiX[_INDEX(i, j)]);
				else if (_gradPhiY[_INDEX(i, j)] > +EPS_F)
					_angl[_INDEX(i, j)] = atan(_gradPhiY[_INDEX(i, j)] / _gradPhiX[_INDEX(i, j)]);

			if (_gradPhiX[_INDEX(i, j)] < -EPS_F)
				_angl[_INDEX(i, j)] = PI_F + atan(_gradPhiY[_INDEX(i, j)] / _gradPhiX[_INDEX(i, j)]);

			
			_epsilon[_INDEX(i, j)] = _epsilonBar * (1.0f + _delta * cos(_anisotropy * _angl[_INDEX(i, j)]));
			_epsilonDeriv[_INDEX(i, j)] = -_epsilonBar * _anisotropy * _delta * sin(_anisotropy * _angl[_INDEX(i, j)]);

		}
	}
}

void Kobayashi::_evolution()
{
	for (int j = 0; j < _objectCount.y; j++)
	{
		for (int i = 0; i < _objectCount.x; i++)
		{

			int i_plus = (i + 1) % _objectCount.x;
			int i_minus = ((i - 1) + _objectCount.x) % _objectCount.x;
			int j_plus = (j + 1) % _objectCount.y;
			int j_minus = ((j - 1) + _objectCount.y) % _objectCount.y;


			float gradEpsPowX = 
				(_epsilon[_INDEX(i_plus, j)] * _epsilon[_INDEX(i_plus, j)] 
					- _epsilon[_INDEX(i_minus, j)] * _epsilon[_INDEX(i_minus, j)]) / _dx;
			float gradEpsPowY = 
				(_epsilon[_INDEX(i, j_plus)] * _epsilon[_INDEX(i, j_plus)] 
					- _epsilon[_INDEX(i, j_minus)] * _epsilon[_INDEX(i, j_minus)]) / _dy;

			float term1 = (_epsilon[_INDEX(i, j_plus)] * _epsilonDeriv[_INDEX(i, j_plus)] * _gradPhiX[_INDEX(i, j_plus)]
				- _epsilon[_INDEX(i, j_minus)] * _epsilonDeriv[_INDEX(i, j_minus)] * _gradPhiX[_INDEX(i, j_minus)])
				/ _dy;

			float term2 = -(_epsilon[_INDEX(i_plus, j)] * _epsilonDeriv[_INDEX(i_plus, j)] * _gradPhiY[_INDEX(i_plus, j)]
				- _epsilon[_INDEX(i_minus, j)] * _epsilonDeriv[_INDEX(i_minus, j)] * _gradPhiY[_INDEX(i_minus, j)])
				/ _dx;
			float term3 = gradEpsPowX * _gradPhiX[_INDEX(i, j)] + gradEpsPowY * _gradPhiY[_INDEX(i, j)];

			float m = _alpha / PI_F * atan(_gamma*(_tEq - _t[_INDEX(i, j)]));

			float oldPhi = _phi[_INDEX(i, j)];
			float oldT = _t[_INDEX(i, j)];

			_phi[_INDEX(i, j)] = _phi[_INDEX(i, j)] +
				(term1 + term2 + _epsilon[_INDEX(i, j)] * _epsilon[_INDEX(i, j)] * _lapPhi[_INDEX(i, j)]
					+ term3
					+ oldPhi * (1.0f - oldPhi)*(oldPhi - 0.5f + m))*_dt / _tau;
			_t[_INDEX(i, j)] = oldT + _lapT[_INDEX(i, j)] * _dt + _K * (_phi[_INDEX(i, j)] - oldPhi);


		}

	}
}

void Kobayashi::_update()
{
	_computeGradientLaplacian();
	_evolution();
}



#pragma region implementation
// ################################## implementation ####################################
// Simulation methods
void Kobayashi::iUpdate()
{
	_update();
}

void Kobayashi::iResetSimulationState(std::vector<ConstantBuffer>& constantBuffer)
{
}


// Mesh methods
std::vector<Vertex>& Kobayashi::iGetVertice()
{
	_vertices.clear();

	_vertices.push_back(Vertex({ DirectX::XMFLOAT3(-0.5f, -0.5f, -0.0f) }));
	_vertices.push_back(Vertex({ DirectX::XMFLOAT3(-0.5f, +0.5f, -0.0f) }));
	_vertices.push_back(Vertex({ DirectX::XMFLOAT3(+0.5f, +0.5f, -0.0f) }));
	_vertices.push_back(Vertex({ DirectX::XMFLOAT3(+0.5f, -0.5f, -0.0f) }));

	return _vertices;
}

std::vector<unsigned int>& Kobayashi::iGetIndice()
{
	_indices.clear();

	_indices.push_back(0); _indices.push_back(1); _indices.push_back(2);
	_indices.push_back(0); _indices.push_back(2); _indices.push_back(3);

	return _indices;
}

UINT Kobayashi::iGetVertexBufferSize()
{
	return 4;
}

UINT Kobayashi::iGetIndexBufferSize()
{
	return 6;
}


// DirectX methods
void Kobayashi::iCreateObject(std::vector<ConstantBuffer>& constantBuffer)
{
	for (int j = 0; j < _objectCount.y; j++)
	{
		for (int i = 0; i < _objectCount.x; i++)
		{
			// Position
			XMFLOAT2 pos = XMFLOAT2(
				(float)i,
				(float)j);

			struct ConstantBuffer objectCB;
			objectCB.world = DXViewer::util::transformMatrix(pos.x, pos.y, 0.0f, 1.0f);
			objectCB.worldViewProj = DXViewer::util::transformMatrix(0.0f, 0.0f, 0.0f);
			objectCB.transInvWorld = DXViewer::util::transformMatrix(0.0f, 0.0f, 0.0f);
			objectCB.color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

			constantBuffer.push_back(objectCB);
		}
	}
}

void Kobayashi::iUpdateConstantBuffer(std::vector<ConstantBuffer>& constantBuffer, int i)
{
	int size = constantBuffer.size();
	int j = i / (int)(sqrt(size));
	int k = i % (int)(sqrt(size));

	float phi = _phi[_INDEX(j, k)];
	constantBuffer[i].color = { phi, phi, phi, 1.0f };
}

void Kobayashi::iDraw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& mCommandList, int size, UINT indexCount, int i)
{
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Kobayashi::iSetDXApp(DX12App* dxApp)
{
	_dxapp = dxApp;
}

UINT Kobayashi::iGetConstantBufferSize()
{
	return _objectCount.x * _objectCount.y * 2;
}

XMINT3 Kobayashi::iGetObjectCount()
{
	return { _objectCount.x, _objectCount.y, 0 };
}

XMFLOAT3 Kobayashi::iGetObjectSize()
{
	return { 1.0f, 1.0f, 0.0f };
}

XMFLOAT3 Kobayashi::iGetObjectPositionOffset()
{
	return { 0.0f, 0.0f, 0.0f };
}

bool Kobayashi::iIsUpdated()
{
	return true;
}

void Kobayashi::iWMCreate(HWND hwnd, HINSTANCE hInstance)
{
}

void Kobayashi::iWMCommand(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance)
{
}

void Kobayashi::iWMHScroll(HWND hwnd, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance)
{
}

void Kobayashi::iWMTimer(HWND hwnd)
{
}

void Kobayashi::iWMDestory(HWND hwnd)
{
}

// #######################################################################################
#pragma endregion