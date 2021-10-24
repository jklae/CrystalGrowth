#include "Kobayashi.h"

using namespace std;
using namespace DirectX;

Kobayashi::Kobayashi(int nx, int ny, double spacing) :
	_nx(nx),
	_ny(ny)
{
	//
	_dx = 0.03;
	_dy = 0.03;
	_dt = 0.0001;
	tau = 0.0003;
	epsilonBar = 0.01;		// mean of epsilon. scaling factor that determines how much the microscopic front is magnified
	mu = 1.0;
	K = 1.6;				// latent heat 
	delta = 0.05;			// strength of anisotropy (speed of growth in preferred directions)
	anisotropy = 6.0;		// degree of anisotropy
	alpha = 0.9;
	gamma = 10.0;
	tEq = 1.0;
	//sdas

	_x.resize(nx);
	_y.resize(ny);

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
		_x[i] = i * spacing - _nx / 2.0 * spacing;
	}

	for (int i = 0; i < _ny; i++)
	{
		_y[i] = i * spacing - _ny / 2.0 * spacing;
	}

	for (int i = 0; i < _nx; i++)
	{
		for (int j = 0; j < _ny; j++)
		{
			_phi[i][j] = 0.0;
		}
	}

	// set the nuclei
	createNuclei(0, 0);

}

void Kobayashi::createNuclei(int transX, int transY)
{
	for (int i = 0; i < _nx; i++)
	{
		for (int j = 0; j < _ny; j++)
		{
			int iIdx = (i - (_nx / 2) + transX);
			int jIdx = (j - (_ny / 2) + transY);

			// circle equation
			if (iIdx * iIdx + jIdx * jIdx < 10)
				_phi[i][j] = 1.0;


		}
	}
}

void Kobayashi::initVector2D(vector<vector<double>>& vec2D)
{
	for (int i = 0; i < _nx; i++)
	{
		vector<double> tmp;
		tmp.resize(_ny);

		vec2D.push_back(tmp);
	}
}

void Kobayashi::computeGradLap(int start, int end)
{
#pragma omp parallel num_threads(24)
	{
#pragma omp for schedule(guided)
		for (int k = 0; k < _nx * _ny; k++)
		{
			int i = k / _nx;
			int j = k % _ny;

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

			_lapPhi[i][j] = (2.0 * (_phi[ip][j] + _phi[im][j] + _phi[i][jp] + _phi[i][jm])
				+ _phi[ip][jp] + _phi[im][jm] + _phi[im][jp] + _phi[ip][jm]
				- 12.0*_phi[i][j])
				/ (3.0*_dx*_dx);
			_lapT[i][j] = (2.0 * (_t[ip][j] + _t[im][j] + _t[i][jp] + _t[i][jm])
				+ _t[ip][jp] + _t[im][jm] + _t[im][jp] + _t[ip][jm]
				- 12.0*_t[i][j])
				/ (3.0*_dx*_dx);


			if (_gradPhiX[i][j] == 0)
				if (_gradPhiY[i][j] < 0)
					_angl[i][j] = -0.5*pi;
				else if (_gradPhiY[i][j] > 0)
					_angl[i][j] = 0.5*pi;

			if (_gradPhiX[i][j] > 0)
				if (_gradPhiY[i][j] < 0)
					_angl[i][j] = 2.0*pi + atan(_gradPhiY[i][j] / _gradPhiX[i][j]);
				else if (_gradPhiY[i][j] > 0)
					_angl[i][j] = atan(_gradPhiY[i][j] / _gradPhiX[i][j]);

			if (_gradPhiX[i][j] < 0)
				_angl[i][j] = pi + atan(_gradPhiY[i][j] / _gradPhiX[i][j]);




			_epsilon[i][j] = epsilonBar * (1.0 + delta * cos(anisotropy*_angl[i][j]));
			_epsilonDeriv[i][j] = -epsilonBar * anisotropy * delta * sin(anisotropy*_angl[i][j]);

		}

	}
	//printParam(_epsilonDeriv, "========_epsilonDeriv===========", true);
}

void Kobayashi::printParam(vector<vector<double>>& vectemp, const char* a, bool exp) const
{
	// column index
	printf("\n%s\n", a);
	for (int j = 0; j < _ny + 1; j++)
	{
		printf(exp ? "%11d  " : "%8d  ", j);
	}
	printf("\n");

	// row 
	for (int i = 0; i < _nx; i++)
	{
		// row index
		printf(exp ? "%11d  " : "%8d  ", i + 1);

		// -- contents
		for (int j = 0; j < _ny; j++)
		{
			printf(exp ? "%11.4e, " : "%8.4lf  ", vectemp[i][j]);
		}
		printf("\n");
		// --

	}
	printf("\n");
}

void Kobayashi::evolution()
{
#pragma omp parallel num_threads(24)
	{
#pragma omp for schedule(guided)
		for (int k = 0; k < _nx * _ny; k++)
		{
			int i = k / _nx;
			int j = k % _ny;

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

			double gradEpsPowX = (_epsilon[ip][j] * _epsilon[ip][j] - _epsilon[im][j] * _epsilon[im][j]) / _dx;
			double gradEpsPowY = (_epsilon[i][jp] * _epsilon[i][jp] - _epsilon[i][jm] * _epsilon[i][jm]) / _dy;

			double term1 = (_epsilon[i][jp] * _epsilonDeriv[i][jp] * _gradPhiX[i][jp]
				- _epsilon[i][jm] * _epsilonDeriv[i][jm] * _gradPhiX[i][jm])
				/ _dy;

			double term2 = -(_epsilon[ip][j] * _epsilonDeriv[ip][j] * _gradPhiY[ip][j]
				- _epsilon[im][j] * _epsilonDeriv[im][j] * _gradPhiY[im][j])
				/ _dx;
			double term3 = gradEpsPowX * _gradPhiX[i][j] + gradEpsPowY * _gradPhiY[i][j];

			double m = alpha / pi * atan(gamma*(tEq - _t[i][j]));

			double oldPhi = _phi[i][j];
			double oldT = _t[i][j];

			_phi[i][j] = _phi[i][j] +
				(term1 + term2 + _epsilon[i][j] * _epsilon[i][j] * _lapPhi[i][j]
					+ term3
					+ oldPhi * (1.0 - oldPhi)*(oldPhi - 0.5 + m))*_dt / tau;
			_t[i][j] = oldT + _lapT[i][j] * _dt + K * (_phi[i][j] - oldPhi);


		}

	}
	//printParam(_t, "========t===========", true);
}

void Kobayashi::update()
{
	clock_t start, finish;
	/*double duration;

	start = clock();*/
	computeGradLap(0, 0);
	evolution();
	/*finish = clock();

	duration = (double)(finish - start) / CLOCKS_PER_SEC;

	cout << duration << "ÃÊ\n";*/

	/*static int step = 1;
	printf("step %d\n", step);
	step++;*/
}



#pragma region Implementation
void Kobayashi::iUpdate()
{
	update();
}

void Kobayashi::iResetSimulationState(std::vector<ConstantBuffer>& constantBuffer)
{
}

std::vector<Vertex> Kobayashi::iGetVertice()
{
	vector<Vertex> vertices =
	{
		Vertex({ XMFLOAT3(-0.5f, -0.5f, 0.0f) }),
		Vertex({ XMFLOAT3(-0.5f, +0.5f, 0.0f) }),
		Vertex({ XMFLOAT3(+0.5f, +0.5f, 0.0f) }),
		Vertex({ XMFLOAT3(+0.5f, -0.5f, 0.0f) })
	};

	return vertices;
}

std::vector<unsigned int> Kobayashi::iGetIndice()
{
	vector<unsigned int> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,
	};

	return indices;
}

int Kobayashi::iGetObjectCount()
{
	return _objectCount;
}

void Kobayashi::iCreateObjectParticle(std::vector<ConstantBuffer>& constantBuffer)
{
	for (int i = 0; i < _objectCount; i++)
	{
		for (int j = 0; j < _objectCount; j++)
		{
			// Position
			XMFLOAT2 pos = XMFLOAT2(
				(float)j,    // "j"
				(float)i);   // "i"

			struct ConstantBuffer objectCB;
			// Multiply by a specific value to make a stripe
			objectCB.world = transformMatrix(pos.x, pos.y, 0.0f, 0.8f);
			objectCB.worldViewProj = transformMatrix(0.0f, 0.0f, 0.0f);
			objectCB.color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

			constantBuffer.push_back(objectCB);
		}
	}
}

void Kobayashi::iWMCreate(HWND hwnd, HINSTANCE hInstance)
{
}

void Kobayashi::iWMCommand(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance, bool& updateFlag, DX12App* dxapp)
{
}

void Kobayashi::iWMHScroll(HWND hwnd, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance, DX12App* dxapp)
{
}

void Kobayashi::iWMTimer(HWND hwnd)
{
}

void Kobayashi::iWMDestory(HWND hwnd)
{
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
#pragma endregion