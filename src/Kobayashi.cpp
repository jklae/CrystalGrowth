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
	tau = 0.0003f;
	epsilonBar = 0.01f;		// mean of epsilon. scaling factor that determines how much the microscopic front is magnified
	mu = 1.0f;
	K = 1.6f;				// latent heat 
	delta = 0.05f;			// strength of anisotropy (speed of growth in preferred directions)
	anisotropy = 6.0f;		// degree of anisotropy
	alpha = 0.9f;
	gamma = 10.0f;
	tEq = 1.0f;
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

	// set the position
	for (int i = 0; i < _objectCount.x; i++)
	{
		_x[i] = i - _objectCount.x / 2.0;
	}

	for (int i = 0; i < _objectCount.y; i++)
	{
		_y[i] = i - _objectCount.y / 2.0;
	}

	// set the nuclei
	XMINT2 center = { _objectCount.x / 2 , _objectCount.y / 2 };
	_phi[_INDEX(center.x, center.y)] = 1.0f;
	_phi[_INDEX(center.x - 1, center.y)] = 1.0f;
	_phi[_INDEX(center.x + 1, center.y)] = 1.0f;
	_phi[_INDEX(center.x, center.y - 1)] = 1.0f;
	_phi[_INDEX(center.x, center.y + 1)] = 1.0f;
}

void Kobayashi::computeGradLap()
{

	for (int j = 0; j < _objectCount.y; j++)
	{
		for (int i = 0; i < _objectCount.x; i++)
		{

			int jp = j + 1;
			int jm = j - 1;
			int ip = i + 1;
			int im = i - 1;

			if (im == -1)
				im = _objectCount.x - 1;
			else if (ip == _objectCount.x)
				ip = 0;

			if (jm == -1)
				jm = _objectCount.y - 1;
			else if (jp == _objectCount.y)
				jp = 0;



			_gradPhiX[_INDEX(i, j)] = (_phi[_INDEX(ip, j)] - _phi[_INDEX(im, j)]) / _dx;
			_gradPhiY[_INDEX(i, j)] = (_phi[_INDEX(i, jp)] - _phi[_INDEX(i, jm)]) / _dy;

			_lapPhi[_INDEX(i, j)] = (2.0f * (_phi[_INDEX(ip, j)] + _phi[_INDEX(im, j)] + _phi[_INDEX(i, jp)] + _phi[_INDEX(i, jm)])
				+ _phi[_INDEX(ip, jp)] + _phi[_INDEX(im, jm)] + _phi[_INDEX(im, jp)] + _phi[_INDEX(ip, jm)]
				- 12.0f * _phi[_INDEX(i, j)])
				/ (3.0f * _dx * _dx);
			_lapT[_INDEX(i, j)] = (2.0f * (_t[_INDEX(ip, j)] + _t[_INDEX(im, j)] + _t[_INDEX(i, jp)] + _t[_INDEX(i, jm)])
				+ _t[_INDEX(ip, jp)] + _t[_INDEX(im, jm)] + _t[_INDEX(im, jp)] + _t[_INDEX(ip, jm)]
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

			
			_epsilon[_INDEX(i, j)] = epsilonBar * (1.0f + delta * cos(anisotropy * _angl[_INDEX(i, j)]));
			_epsilonDeriv[_INDEX(i, j)] = -epsilonBar * anisotropy * delta * sin(anisotropy * _angl[_INDEX(i, j)]);

		}
	}
}

void Kobayashi::evolution()
{
	for (int j = 0; j < _objectCount.y; j++)
	{
		for (int i = 0; i < _objectCount.x; i++)
		{

			int jp = j + 1;
			int jm = j - 1;
			int ip = i + 1;
			int im = i - 1;

			if (im == -1)
				im = _objectCount.x - 1;
			else if (ip == _objectCount.x)
				ip = 0;

			if (jm == -1)
				jm = _objectCount.y - 1;
			else if (jp == _objectCount.y)
				jp = 0;

			float gradEpsPowX = (_epsilon[_INDEX(ip, j)] * _epsilon[_INDEX(ip, j)] - _epsilon[_INDEX(im, j)] * _epsilon[_INDEX(im, j)]) / _dx;
			float gradEpsPowY = (_epsilon[_INDEX(i, jp)] * _epsilon[_INDEX(i, jp)] - _epsilon[_INDEX(i, jm)] * _epsilon[_INDEX(i, jm)]) / _dy;

			float term1 = (_epsilon[_INDEX(i, jp)] * _epsilonDeriv[_INDEX(i, jp)] * _gradPhiX[_INDEX(i, jp)]
				- _epsilon[_INDEX(i, jm)] * _epsilonDeriv[_INDEX(i, jm)] * _gradPhiX[_INDEX(i, jm)])
				/ _dy;

			float term2 = -(_epsilon[_INDEX(ip, j)] * _epsilonDeriv[_INDEX(ip, j)] * _gradPhiY[_INDEX(ip, j)]
				- _epsilon[_INDEX(im, j)] * _epsilonDeriv[_INDEX(im, j)] * _gradPhiY[_INDEX(im, j)])
				/ _dx;
			float term3 = gradEpsPowX * _gradPhiX[_INDEX(i, j)] + gradEpsPowY * _gradPhiY[_INDEX(i, j)];

			float m = alpha / PI_F * atan(gamma*(tEq - _t[_INDEX(i, j)]));

			float oldPhi = _phi[_INDEX(i, j)];
			float oldT = _t[_INDEX(i, j)];

			_phi[_INDEX(i, j)] = _phi[_INDEX(i, j)] +
				(term1 + term2 + _epsilon[_INDEX(i, j)] * _epsilon[_INDEX(i, j)] * _lapPhi[_INDEX(i, j)]
					+ term3
					+ oldPhi * (1.0f - oldPhi)*(oldPhi - 0.5f + m))*_dt / tau;
			_t[_INDEX(i, j)] = oldT + _lapT[_INDEX(i, j)] * _dt + K * (_phi[_INDEX(i, j)] - oldPhi);


		}

	}
}

void Kobayashi::update()
{
	computeGradLap();
	evolution();
}



#pragma region Implementation
// ################################## Implementation ####################################
// Simulation methods
void Kobayashi::iUpdate()
{
	update();
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

	constantBuffer[i].color = { (float)_phi[_INDEX(j, k)], (float)_phi[_INDEX(j, k)], (float)_phi[_INDEX(j, k)], 1.0f };
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