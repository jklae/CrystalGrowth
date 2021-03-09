#include "Win32App.h"


using namespace std;

// static variable is used to put the proc function into the class.
Win32App* Win32App::instanceForProc = nullptr;
Win32App* Win32App::GetinstanceForProc()
{
	return instanceForProc;
}
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Win32App::GetinstanceForProc()->WndProc(hwnd, msg, wParam, lParam);
}
//

Win32App::Win32App(const int kWidth, const int KHeight)
	:kWidth(kWidth), kHeight(KHeight)
{
	instanceForProc = this;
}

Win32App::~Win32App()
{
}

bool Win32App::Initialize(HINSTANCE hInstance)
{
	// Just call it once.
	assert(mhMainWnd == nullptr);

	if (!InitMainWindow(hInstance))
		return false;

	return true;
}


LRESULT Win32App::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}





bool Win32App::InitMainWindow(HINSTANCE hInstance)
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWnd";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}


	mhMainWnd = CreateWindow(L"MainWnd", L"d3d App",
		(WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX)), // disable resizing and maximzing 
		CW_USEDEFAULT, CW_USEDEFAULT, kWidth, kHeight,
		0, 0, hInstance, 0);

	if (!mhMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

	return true;
}


int Win32App::Run()
{
	assert(mhMainWnd != nullptr);

	MSG msg = {0};
	while(msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if(PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
		{
            TranslateMessage( &msg );
            DispatchMessage( &msg );
		}
		// Otherwise, do animation/game stuff.
		else
        {	
			Update();
			Draw();
        }
    }

	return (int)msg.wParam;
}


void Win32App::InitDirectX()
{
	// Call after window init
	assert(mhMainWnd != nullptr);

	// Just call it once.
	assert(dxApp == nullptr);

	dxApp = std::make_unique<DX12DefaultApp>(kWidth, kHeight, mhMainWnd);
	dxApp->Initialize();
}

void Win32App::CreateObjects(const int count, const float scale)
{
	// Call after directx init
	assert(dxApp != nullptr);

	dxApp.reset();
	dxApp = std::make_unique<DX12DrawingApp>(kWidth, kHeight, mhMainWnd);
	dxApp->Initialize(count, scale);

	
}

void Win32App::Update()
{
	if (dxApp)
	{
		DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		dxApp->Update(color);
	}
}

void Win32App::Draw()
{
	if (dxApp)
	{
		dxApp->Draw();
	}
}




void Win32App::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void Win32App::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Win32App::OnMouseMove(WPARAM btnState, int x, int y)
{
	if (dxApp)
	{
		if ((btnState & MK_LBUTTON) != 0)
		{
			dxApp->UpdateVirtualSphereAngles(mLastMousePos, x, y);
		}
		else if ((btnState & MK_RBUTTON) != 0)
		{
			dxApp->UpdateVirtualSphereRadius(mLastMousePos, x, y);
		}

		mLastMousePos.x = x;
		mLastMousePos.y = y;
	}
}

