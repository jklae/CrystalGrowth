#include "Kobayashi.h"

using namespace std;
using namespace DirectX;

Kobayashi::Kobayashi(int nx, int ny, float timeStep) :
	_nx(nx),
	_ny(ny)
{
	_objectCount = { _nx, _ny };

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
	//sdas

	_x.resize(_nx);
	_y.resize(_ny);

	initVector2D(_phi);
	initVector2D(_t);
	initVector2D(_gradPhiX);
	initVector2D(_gradPhiY);
	initVector2D(_lapPhi);
	initVector2D(_lapT);

	initVector2D(_angl);
	initVector2D(_epsilon);
	initVector2D(_epsilonDeriv);

	// set the position
	for (int i = 0; i < _nx; i++)
	{
		_x[i] = i - _nx / 2.0;
	}

	for (int i = 0; i < _ny; i++)
	{
		_y[i] = i - _ny / 2.0;
	}

	for (int j = 0; j < _ny; j++)
	{
		for (int i = 0; i < _nx; i++)
		{
			_phi[i][j] = 0.0;
		}
	}

	// set the nuclei
	createNuclei(0, 0);

}

void Kobayashi::createNuclei(int transX, int transY)
{
	for (int j = 0; j < _ny; j++)
	{
		for (int i = 0; i < _nx; i++)
		{
			int iIdx = (i - (_nx / 2) + transX);
			int jIdx = (j - (_ny / 2) + transY);

			// circle equation
			if (iIdx * iIdx + jIdx * jIdx < 10)
				_phi[i][j] = 1.0;


		}
	}
}

void Kobayashi::initVector2D(vector<vector<float>>& vec2D)
{
	for (int i = 0; i < _nx; i++)
	{
		vector<float> tmp;
		tmp.resize(_ny);

		vec2D.push_back(tmp);
	}
}

void Kobayashi::computeGradLap(int start, int end)
{

	for (int j = 0; j < _ny; j++)
	{
		for (int i = 0; i < _nx; i++)
		{

			int jp = j + 1;
			int jm = j - 1;
			int ip = i + 1;
			int im = i - 1;

			if (im == -1)
				im = _nx - 1;
			else if (ip == _nx)
				ip = 0;

			if (jm == -1)
				jm = _ny - 1;
			else if (jp == _ny)
				jp = 0;



			_gradPhiX[i][j] = (_phi[ip][j] - _phi[im][j]) / _dx;
			_gradPhiY[i][j] = (_phi[i][jp] - _phi[i][jm]) / _dy;

			_lapPhi[i][j] = (2.0f * (_phi[ip][j] + _phi[im][j] + _phi[i][jp] + _phi[i][jm])
				+ _phi[ip][jp] + _phi[im][jm] + _phi[im][jp] + _phi[ip][jm]
				- 12.0f * _phi[i][j])
				/ (3.0f * _dx * _dx);
			_lapT[i][j] = (2.0f * (_t[ip][j] + _t[im][j] + _t[i][jp] + _t[i][jm])
				+ _t[ip][jp] + _t[im][jm] + _t[im][jp] + _t[ip][jm]
				- 12.0f * _t[i][j])
				/ (3.0f * _dx * _dx);


			if (_gradPhiX[i][j] <= +EPS_F && _gradPhiX[i][j] >= -EPS_F) // _gradPhiX[i][j] == 0.0f
				if (_gradPhiY[i][j] < -EPS_F)
					_angl[i][j] = -0.5f * PI_F;
				else if (_gradPhiY[i][j] > +EPS_F)
					_angl[i][j] = 0.5f * PI_F;

			if (_gradPhiX[i][j] > +EPS_F)
				if (_gradPhiY[i][j] < -EPS_F)
					_angl[i][j] = 2.0f * PI_F + atan(_gradPhiY[i][j] / _gradPhiX[i][j]);
				else if (_gradPhiY[i][j] > +EPS_F)
					_angl[i][j] = atan(_gradPhiY[i][j] / _gradPhiX[i][j]);

			if (_gradPhiX[i][j] < -EPS_F)
				_angl[i][j] = PI_F + atan(_gradPhiY[i][j] / _gradPhiX[i][j]);




			_epsilon[i][j] = epsilonBar * (1.0f + delta * cos(anisotropy * _angl[i][j]));
			_epsilonDeriv[i][j] = -epsilonBar * anisotropy * delta * sin(anisotropy * _angl[i][j]);

		}
	}
}

void Kobayashi::evolution()
{
	for (int j = 0; j < _ny; j++)
	{
		for (int i = 0; i < _nx; i++)
		{

			int jp = j + 1;
			int jm = j - 1;
			int ip = i + 1;
			int im = i - 1;

			if (im == -1)
				im = _nx - 1;
			else if (ip == _nx)
				ip = 0;

			if (jm == -1)
				jm = _ny - 1;
			else if (jp == _ny)
				jp = 0;

			float gradEpsPowX = (_epsilon[ip][j] * _epsilon[ip][j] - _epsilon[im][j] * _epsilon[im][j]) / _dx;
			float gradEpsPowY = (_epsilon[i][jp] * _epsilon[i][jp] - _epsilon[i][jm] * _epsilon[i][jm]) / _dy;

			float term1 = (_epsilon[i][jp] * _epsilonDeriv[i][jp] * _gradPhiX[i][jp]
				- _epsilon[i][jm] * _epsilonDeriv[i][jm] * _gradPhiX[i][jm])
				/ _dy;

			float term2 = -(_epsilon[ip][j] * _epsilonDeriv[ip][j] * _gradPhiY[ip][j]
				- _epsilon[im][j] * _epsilonDeriv[im][j] * _gradPhiY[im][j])
				/ _dx;
			float term3 = gradEpsPowX * _gradPhiX[i][j] + gradEpsPowY * _gradPhiY[i][j];

			float m = alpha / PI_F * atan(gamma*(tEq - _t[i][j]));

			float oldPhi = _phi[i][j];
			float oldT = _t[i][j];

			_phi[i][j] = _phi[i][j] +
				(term1 + term2 + _epsilon[i][j] * _epsilon[i][j] * _lapPhi[i][j]
					+ term3
					+ oldPhi * (1.0f - oldPhi)*(oldPhi - 0.5f + m))*_dt / tau;
			_t[i][j] = oldT + _lapT[i][j] * _dt + K * (_phi[i][j] - oldPhi);


		}

	}
}

void Kobayashi::update()
{
	computeGradLap(0, 0);
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

	constantBuffer[i].color = { (float)_phi[j][k], (float)_phi[j][k], (float)_phi[j][k], 1.0f };
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