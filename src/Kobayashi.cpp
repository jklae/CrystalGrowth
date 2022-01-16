#include "Kobayashi.h"

using namespace std;
using namespace DirectX;
using namespace DXViewer::xmfloat3;

Kobayashi::Kobayashi(int x, int y, float timeStep)
{
	_objectCount = { x, y };

	// The scroll position is stored separately as an integer due to the floating point precision.
	_crystalParameter.push_back(
		CrystalParameter(
										  //   value      min      max     stride
			ScrollParameter<float&, float>(		  _tau, 0.0001f, 0.0009f, 0.0001f), 
			ScrollParameter<int, int>	  (			 3,      1,       9,       1)));
	_crystalParameter.push_back(
		CrystalParameter(
			ScrollParameter<float&, float>(_epsilonBar,  0.006f,  0.015f,  0.001f), 
			ScrollParameter<int, int>	  (         10,      6,      15,       1)));
	_crystalParameter.push_back(
		CrystalParameter(
			ScrollParameter<float&, float>(		   _mu,    0.5f,    1.4f,    0.1f), 
			ScrollParameter<int, int>	  (			10,      5,      14,       1)));
	_crystalParameter.push_back(
		CrystalParameter(
			ScrollParameter<float&, float>(			_K,    1.0f,    1.9f,    0.1f), 
			ScrollParameter<int, int>	  (			16,    10,      19,        1)));
	_crystalParameter.push_back(
		CrystalParameter(
			ScrollParameter<float&, float>(		_delta,   0.01f,   0.09f,   0.01f), 
			ScrollParameter<int, int>	  (		     5,      1,       9,       1)));
	_crystalParameter.push_back(
		CrystalParameter(
			ScrollParameter<float&, float>(_anisotropy,    2.0f,    8.0f,      1.0f), 
			ScrollParameter<int, int>	  (          6,    2,       8,         1)));
	_crystalParameter.push_back(
		CrystalParameter(
			ScrollParameter<float&, float>(		_alpha,    0.7f,    1.2f,    0.1f), 
			ScrollParameter<int, int>	  (			 9,      7,     12,        1)));
	_crystalParameter.push_back(
		CrystalParameter(
			ScrollParameter<float&, float>(		_gamma,   10.0f,   20.0f,      1.0f), 
			ScrollParameter<int, int>	  (		    10,   10,      20,         1)));
	_crystalParameter.push_back(
		CrystalParameter(
			ScrollParameter<float&, float>(		  _tEq,    0.5f,    1.5f,    0.1f), 
			ScrollParameter<int, int>	  (		    10,      5,     15,        1)));

	_dx = 0.03f;
	_dy = 0.03f;
	_dt = timeStep;

	_parameterInit();
	_vectorInit();
}

Kobayashi::~Kobayashi()
{
}

void Kobayashi::_parameterInit()
{
	//
	_tau		= 0.0003f;
	_epsilonBar = 0.010f;		// Mean of epsilon. scaling factor that determines how much the microscopic front is magnified
	_mu		    = 1.0f;
	_K			= 1.6f;			// Latent heat 
	_delta		= 0.05f;		// Strength of anisotropy (speed of growth in preferred directions)
	_anisotropy = 6.0f;			// Degree of anisotropy
	_alpha		= 0.9f;
	_gamma		= 10.0f;
	_tEq		= 1.0f;
	//

	_crystalParameter[static_cast<int>(COM::TAU)].param_i.value			= 3;
	_crystalParameter[static_cast<int>(COM::EPLSILONBAR)].param_i.value = 10;
	_crystalParameter[static_cast<int>(COM::MU)].param_i.value			= 10;
	_crystalParameter[static_cast<int>(COM::K)].param_i.value			= 16;
	_crystalParameter[static_cast<int>(COM::DELTA)].param_i.value		= 5;
	_crystalParameter[static_cast<int>(COM::ANISOTROPY)].param_i.value  = 6;
	_crystalParameter[static_cast<int>(COM::ALPHA)].param_i.value		= 9;
	_crystalParameter[static_cast<int>(COM::GAMMA)].param_i.value		= 10;
	_crystalParameter[static_cast<int>(COM::TEQ)].param_i.value			= 10;
}

void Kobayashi::_vectorInit()
{
	size_t vSize = static_cast<size_t>(_objectCount.x) * static_cast<size_t>(_objectCount.y);
	_phi.assign(vSize, 0.0f);
	_t.assign(vSize, 0.0f);
	_gradPhiX.assign(vSize, 0.0f);
	_gradPhiY.assign(vSize, 0.0f);
	_lapPhi.assign(vSize, 0.0f);
	_lapT.assign(vSize, 0.0f);
	_angl.assign(vSize, 0.0f);
	_epsilon.assign(vSize, 0.0f);
	_epsilonDeriv.assign(vSize, 0.0f);


	// Create the neuclei
	_createNucleus(_objectCount.x / 2, _objectCount.y / 2);
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


			if (_gradPhiX[_INDEX(i, j)] <= +FLT_EPSILON && _gradPhiX[_INDEX(i, j)] >= -FLT_EPSILON) // _gradPhiX[i][j] == 0.0f
				if (_gradPhiY[_INDEX(i, j)] < -FLT_EPSILON)
					_angl[_INDEX(i, j)] = -0.5f * PI_F;
				else if (_gradPhiY[_INDEX(i, j)] > +FLT_EPSILON)
					_angl[_INDEX(i, j)] = 0.5f * PI_F;

			if (_gradPhiX[_INDEX(i, j)] > +FLT_EPSILON)
				if (_gradPhiY[_INDEX(i, j)] < -FLT_EPSILON)
					_angl[_INDEX(i, j)] = 2.0f * PI_F + atan(_gradPhiY[_INDEX(i, j)] / _gradPhiX[_INDEX(i, j)]);
				else if (_gradPhiY[_INDEX(i, j)] > +FLT_EPSILON)
					_angl[_INDEX(i, j)] = atan(_gradPhiY[_INDEX(i, j)] / _gradPhiX[_INDEX(i, j)]);

			if (_gradPhiX[_INDEX(i, j)] < -FLT_EPSILON)
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


#pragma region implementation
// ################################## implementation ####################################
// Simulation methods
void Kobayashi::iUpdate()
{
	clock_t startTime = clock();
	for (int i = 0; i < 10; i++)
	{
		_computeGradientLaplacian();
		_evolution();
	}
	clock_t endTime = clock();

	_simTime += endTime - startTime; // ms
	_simFrame++;
}

void Kobayashi::iResetSimulationState(std::vector<ConstantBuffer>& constantBuffer)
{
	_vectorInit();

	_dxapp->update();
	_dxapp->draw();
	_simTime = 0;
	_simFrame = 0;
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
	XMFLOAT3 color;

	XMFLOAT3 c0 = { 0.000'0000f, 0.000'0000f, 0.000'0000f };
	XMFLOAT3 c1 = { 0.250'5490f, 0.500'0000f, 0.988'2353f };
	XMFLOAT3 c2 = { 0.360'7843f, 1.000'0000f, 0.988'2353f };
	XMFLOAT3 c3 = { 0.900'5490f, 1.000'0000f, 0.988'2353f };

	float c1Boundary = 0.9f;
	float c2Boundary = 0.99f;
	float c3Boundary = 1.0f;
	float ratio;

	if (phi <= c1Boundary)
	{
		ratio = phi * (1.0f / c1Boundary);
		color = c0 * (1.0f - ratio) + c1 * ratio;
	}
	else if (phi > c1Boundary && phi <= c2Boundary)
	{
		ratio = (phi - c1Boundary) * (1.0f / (c2Boundary - c1Boundary));
		color = c1 * (1.0f - ratio) + c2 * ratio;
	}
	else //if (phi > c2Boundary && phi <= c3Boundary)
	{
		ratio = (phi - c2Boundary) * (1.0f / (c3Boundary - c2Boundary));
		color = c2 * (1.0f - ratio) + c3 * ratio;
	}

	constantBuffer[i].color = { color.x, color.y, color.z, 1.0f };
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
	return _updateFlag;
}

void Kobayashi::iWMCreate(HWND hwnd, HINSTANCE hInstance)
{
	CreateWindow(L"button", L"Reset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		93, 242, 95, 32, hwnd, reinterpret_cast<HMENU>(COM::RESET), hInstance, NULL);

	CreateWindow(L"button", _updateFlag ? L"¡«" : L"¢º", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		65, 305, 50, 25, hwnd, reinterpret_cast<HMENU>(COM::PLAY), hInstance, NULL);
	CreateWindow(L"button", L"¡á", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		115, 305, 50, 25, hwnd, reinterpret_cast<HMENU>(COM::STOP), hInstance, NULL);
	CreateWindow(L"button", L"¢ºl", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		165, 305, 50, 25, hwnd, reinterpret_cast<HMENU>(COM::NEXTSTEP), hInstance, NULL);

	CreateWindow(L"static", L"time :", WS_CHILD | WS_VISIBLE,
		95, 350, 40, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);
	CreateWindow(L"static", to_wstring(_simTime).c_str(), WS_CHILD | WS_VISIBLE,
		140, 350, 40, 20, hwnd, reinterpret_cast<HMENU>(COM::TIME_TEXT), hInstance, NULL);
	CreateWindow(L"static", L"frame :", WS_CHILD | WS_VISIBLE,
		86, 370, 45, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);
	CreateWindow(L"static", to_wstring(_simFrame).c_str(), WS_CHILD | WS_VISIBLE,
		140, 370, 40, 20, hwnd, reinterpret_cast<HMENU>(COM::FRAME_TEXT), hInstance, NULL);

	

	// tau
	CreateWindow(L"static", L"tau :", WS_CHILD | WS_VISIBLE,
		69, 45, 80, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);
	CreateWindow(L"static", to_wstring(_tau).c_str(), WS_CHILD | WS_VISIBLE,
		105, 45, 44, 20, hwnd, reinterpret_cast<HMENU>(COM::TAU), hInstance, NULL);
	_crystalParameter[static_cast<int>(COM::TAU)].scrollbar = 
		CreateWindow(L"scrollbar", NULL, WS_CHILD | WS_VISIBLE | SBS_HORZ,
			167, 45, 100, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);

	// epsilonBar
	CreateWindow(L"static", L"epsilonBar :", WS_CHILD | WS_VISIBLE,
		18, 65, 80, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);
	CreateWindow(L"static", to_wstring(_epsilonBar).c_str(), WS_CHILD | WS_VISIBLE,
		105, 65, 35, 20, hwnd, reinterpret_cast<HMENU>(COM::EPLSILONBAR), hInstance, NULL);
	_crystalParameter[static_cast<int>(COM::EPLSILONBAR)].scrollbar =
		CreateWindow(L"scrollbar", NULL, WS_CHILD | WS_VISIBLE | SBS_HORZ,
			167, 65, 100, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);

	// mu
	CreateWindow(L"static", L"mu :", WS_CHILD | WS_VISIBLE,
		69, 85, 80, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);
	CreateWindow(L"static", to_wstring(_mu).c_str(), WS_CHILD | WS_VISIBLE,
		105, 85, 20, 20, hwnd, reinterpret_cast<HMENU>(COM::MU), hInstance, NULL);
	_crystalParameter[static_cast<int>(COM::MU)].scrollbar =
		CreateWindow(L"scrollbar", NULL, WS_CHILD | WS_VISIBLE | SBS_HORZ,
			167, 85, 100, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);

	// K
	CreateWindow(L"static", L"K :", WS_CHILD | WS_VISIBLE,
		80, 105, 80, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);
	CreateWindow(L"static", to_wstring(_K).c_str(), WS_CHILD | WS_VISIBLE,
		105, 105, 20, 20, hwnd, reinterpret_cast<HMENU>(COM::K), hInstance, NULL);
	_crystalParameter[static_cast<int>(COM::K)].scrollbar =
		CreateWindow(L"scrollbar", NULL, WS_CHILD | WS_VISIBLE | SBS_HORZ,
			167, 105, 100, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);

	// delta
	CreateWindow(L"static", L"delta :", WS_CHILD | WS_VISIBLE,
		57, 125, 40, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);
	CreateWindow(L"static", to_wstring(_delta).c_str(), WS_CHILD | WS_VISIBLE,
		105, 126, 28, 20, hwnd, reinterpret_cast<HMENU>(COM::DELTA), hInstance, NULL);
	_crystalParameter[static_cast<int>(COM::DELTA)].scrollbar =
		CreateWindow(L"scrollbar", NULL, WS_CHILD | WS_VISIBLE | SBS_HORZ,
			167, 125, 100, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);

	// anisotropy
	CreateWindow(L"static", L"anisotropy :", WS_CHILD | WS_VISIBLE,
		20, 145, 80, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);
	CreateWindow(L"static", to_wstring(_anisotropy).c_str(), WS_CHILD | WS_VISIBLE,
		105, 146, 20, 20, hwnd, reinterpret_cast<HMENU>(COM::ANISOTROPY), hInstance, NULL);
	_crystalParameter[static_cast<int>(COM::ANISOTROPY)].scrollbar = 
		CreateWindow(L"scrollbar", NULL, WS_CHILD | WS_VISIBLE | SBS_HORZ,
			167, 145, 100, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);

	// alpha
	CreateWindow(L"static", L"alpha :", WS_CHILD | WS_VISIBLE,
		53, 165, 80, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);
	CreateWindow(L"static", to_wstring(_alpha).c_str(), WS_CHILD | WS_VISIBLE,
		105, 166, 20, 20, hwnd, reinterpret_cast<HMENU>(COM::ALPHA), hInstance, NULL);
	_crystalParameter[static_cast<int>(COM::ALPHA)].scrollbar =
		CreateWindow(L"scrollbar", NULL, WS_CHILD | WS_VISIBLE | SBS_HORZ,
			167, 165, 100, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);

	// gamma
	CreateWindow(L"static", L"gamma :", WS_CHILD | WS_VISIBLE,
		41, 185, 80, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);
	CreateWindow(L"static", to_wstring(_gamma).c_str(), WS_CHILD | WS_VISIBLE,
		105, 186, 28, 20, hwnd, reinterpret_cast<HMENU>(COM::GAMMA), hInstance, NULL);
	_crystalParameter[static_cast<int>(COM::GAMMA)].scrollbar =
		CreateWindow(L"scrollbar", NULL, WS_CHILD | WS_VISIBLE | SBS_HORZ,
			167, 185, 100, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);

	// tEq
	CreateWindow(L"static", L"tEq :", WS_CHILD | WS_VISIBLE,
		68, 205, 80, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);
	CreateWindow(L"static", to_wstring(_tEq).c_str(), WS_CHILD | WS_VISIBLE,
		105, 206, 20, 20, hwnd, reinterpret_cast<HMENU>(COM::TEQ), hInstance, NULL);
	_crystalParameter[static_cast<int>(COM::TEQ)].scrollbar =
		CreateWindow(L"scrollbar", NULL, WS_CHILD | WS_VISIBLE | SBS_HORZ,
			167, 205, 100, 20, hwnd, reinterpret_cast<HMENU>(-1), hInstance, NULL);


	if (_updateFlag)
	{
		EnableWindow(GetDlgItem(hwnd, static_cast<int>(COM::NEXTSTEP)), false);
	}
	
	for (int i = 0; i <= static_cast<int>(COM::TEQ); i++)
	{
		int minValue = _crystalParameter[i].param_i.minVal;
		int maxValue = _crystalParameter[i].param_i.maxVal;
		int value = _crystalParameter[i].param_i.value;
		HWND scrollbar = _crystalParameter[i].scrollbar;

		SetScrollRange(scrollbar, SB_CTL, minValue, maxValue, TRUE);
		SetScrollPos(scrollbar, SB_CTL, value, TRUE);
	}

	SetTimer(hwnd, 1, 10, NULL);
}

void Kobayashi::iWMCommand(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance)
{
	switch (LOWORD(wParam))
	{
		// ### Execution buttons ###
		case static_cast<int>(COM::RESET):
		{
			_parameterInit();

			for (int i = 0; i <= static_cast<int>(COM::TEQ); i++)
			{
				float& value = _crystalParameter[i].param_f.value;
				float stride = _crystalParameter[i].param_f.stride;
				int value_int = _crystalParameter[i].param_i.value;
				HWND scrollbar = _crystalParameter[i].scrollbar;

				SetScrollPos(scrollbar, SB_CTL, value_int, TRUE);
				SetDlgItemText(hwnd, i, to_wstring(value).c_str());
			}

			_dxapp->resetSimulationState();
		}
		break;
		case static_cast<int>(COM::PLAY):
		{
			_updateFlag = !_updateFlag;
			SetDlgItemText(hwnd, static_cast<int>(COM::PLAY), _updateFlag ? L"¡«" : L"¢º");

			EnableWindow(GetDlgItem(hwnd, static_cast<int>(COM::STOP)), true);
			EnableWindow(GetDlgItem(hwnd, static_cast<int>(COM::NEXTSTEP)), !_updateFlag);
		}
		break;
		case static_cast<int>(COM::STOP):
		{
			_dxapp->resetSimulationState();
		}
		break;
		case static_cast<int>(COM::NEXTSTEP):
		{
			iUpdate();
			_dxapp->update();
			_dxapp->draw();
		}
		break;
	// #####################
	}
}

void Kobayashi::iWMHScroll(HWND hwnd, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance)
{
	HWND iparam = reinterpret_cast<HWND>(lParam);

	int index = 0;
	if (iparam == _crystalParameter[static_cast<int>(COM::TAU)].scrollbar)
		index = static_cast<int>(COM::TAU);
	else if (iparam == _crystalParameter[static_cast<int>(COM::EPLSILONBAR)].scrollbar)
		index = static_cast<int>(COM::EPLSILONBAR);
	else if (iparam == _crystalParameter[static_cast<int>(COM::MU)].scrollbar)
		index = static_cast<int>(COM::MU);
	else if (iparam == _crystalParameter[static_cast<int>(COM::K)].scrollbar)
		index = static_cast<int>(COM::K);
	else if (iparam == _crystalParameter[static_cast<int>(COM::DELTA)].scrollbar)
		index = static_cast<int>(COM::DELTA);
	else if (iparam == _crystalParameter[static_cast<int>(COM::ANISOTROPY)].scrollbar)
		index = static_cast<int>(COM::ANISOTROPY);
	else if (iparam == _crystalParameter[static_cast<int>(COM::ALPHA)].scrollbar)
		index = static_cast<int>(COM::ALPHA);
	else if (iparam == _crystalParameter[static_cast<int>(COM::GAMMA)].scrollbar)
		index = static_cast<int>(COM::GAMMA);
	else
		index = static_cast<int>(COM::TEQ);

	float& value = _crystalParameter[index].param_f.value;
	int minValue_int = _crystalParameter[index].param_i.minVal;
	int maxValue_int = _crystalParameter[index].param_i.maxVal;
	float stride = _crystalParameter[index].param_f.stride;
	int& value_int = _crystalParameter[index].param_i.value;
	int stride_int = _crystalParameter[index].param_i.stride;

	switch (LOWORD(wParam))
	{
	case SB_THUMBTRACK:
		//scrollPos = HIWORD(wParam);
		break;

	case SB_LINELEFT:
	case SB_PAGELEFT:
		if (value_int - stride_int >= minValue_int)
		{
			value_int -= stride_int;
			value -= stride;
		}
		break;

	case SB_LINERIGHT:
	case SB_PAGERIGHT:
		if (value_int + stride_int <= maxValue_int)
		{
			value_int += stride_int;
			value += stride;
		}
		break;
	}

	SetScrollPos(iparam, SB_CTL, value_int, TRUE);
	SetDlgItemText(hwnd, index, to_wstring(value).c_str());

	_dxapp->resetSimulationState();
	
}

void Kobayashi::iWMTimer(HWND hwnd)
{
	SetDlgItemText(hwnd, static_cast<int>(COM::TIME_TEXT), to_wstring(_simTime).c_str());
	SetDlgItemText(hwnd, static_cast<int>(COM::FRAME_TEXT), to_wstring(_simFrame).c_str());
}

void Kobayashi::iWMDestory(HWND hwnd)
{
	KillTimer(hwnd, 1);
}

// #######################################################################################
#pragma endregion