//#if defined(NDEBUG) || !defined(_DEBUG)
//#pragma comment( lib, "TktkMathLib_mt.lib" )
//#else
//#pragma comment( lib, "TktkMathLib_mtd.lib" )
//#endif
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include "MathHelper.h"
#include "Mesh.h"
#include "Skeleton.h"
#include "Animation.h"
#include "SkinnedMesh.h"
#include "SkinnedMeshShader.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hInstancePrev, LPSTR pCmdLine, int nCmdShow)
{
	const LPCSTR WindowName = "GameWindow";
	const int WindowWidth = 640;
	const int WindowHegint = 480;

	WNDCLASS wc{};

	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = WindowName;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClass(&wc);

	RECT rect{ 0, 0, WindowWidth, WindowHegint };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

	HWND hwnd = CreateWindow(
		WindowName,
		WindowName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (hwnd == NULL) return 0;

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	// Dx11 init
	IDXGISwapChain* pSwapChain;
	ID3D11Device* pDevice;
	ID3D11DeviceContext* pDeviceContext;
	DXGI_SWAP_CHAIN_DESC scd{};
	scd.BufferCount = 1;
	scd.BufferDesc.Width = WindowWidth;
	scd.BufferDesc.Height = WindowHegint;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hwnd;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.Windowed = TRUE;

	D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_11_0;

	D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		&fl,
		1,
		D3D11_SDK_VERSION,
		&scd,
		&pSwapChain,
		&pDevice,
		NULL,
		&pDeviceContext
	);

	ID3D11RenderTargetView* pBackBufferRenderTargetView;

	ID3D11Texture2D *pbbTex;
	pSwapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(LPVOID*)&pbbTex
	);

	pDevice->CreateRenderTargetView(
		pbbTex,
		NULL,
		&pBackBufferRenderTargetView
	);

	pbbTex->Release();

	// 深度バッファ
	ID3D11Texture2D* pDepthStencil;

	D3D11_TEXTURE2D_DESC dtd{};
	dtd.Width = WindowWidth;
	dtd.Height = WindowHegint;
	dtd.MipLevels = 1;
	dtd.ArraySize = 1;
	dtd.Format = DXGI_FORMAT_D32_FLOAT;
	dtd.SampleDesc.Count = 1;
	dtd.SampleDesc.Quality = 0;
	dtd.Usage = D3D11_USAGE_DEFAULT;
	dtd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dtd.CPUAccessFlags = 0;
	dtd.MiscFlags = 0;

	pDevice->CreateTexture2D(
		&dtd,
		NULL,
		&pDepthStencil
	);

	ID3D11DepthStencilView* pDepthStencilView{};

	pDevice->CreateDepthStencilView(
		pDepthStencil,
		NULL,
		&pDepthStencilView
	);

	pDeviceContext->OMSetRenderTargets(
		1,
		&pBackBufferRenderTargetView,
		pDepthStencilView
	);

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = (FLOAT)WindowWidth;
	vp.Height = (FLOAT)WindowHegint;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	pDeviceContext->RSSetViewports(1, &vp);

	Mesh mesh{ pDevice, pDeviceContext };
	mesh.load("res/Robot_Kyle/Robot_Kyle.meshs");

	Skeleton skeleton{};
	skeleton.load("Robot_Kyle.skls");

	Animation animation{};
	animation.load("Y_Bot@kicking_1.anms");

	SkinnedMesh skinnedMesh{ &mesh, &skeleton, &animation };

	SkinnedMeshShader skinnedMeshShader{ pDevice, pDeviceContext };
	skinnedMeshShader.load("SkinnedMeshBumpShader.hlsl");

	float animTimer{ 0.0f };

	float angle{ 0.0f };

	MSG msg{};

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			skinnedMesh.calculate(Matrix4::identity, animTimer);
			animTimer = std::fmod(animTimer + 0.5f, animation.endFrame());

			angle += 1.0f;

			const float clearColor[4] = { 0.5f, 0.5f, 1.0f, 1.0f };
			pDeviceContext->ClearRenderTargetView(pBackBufferRenderTargetView, clearColor);
			pDeviceContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

			Vector3 camera = Vector3(0.0f, 20.0f, -70.0f);// *Matrix4::createRotationY(angle);
			Matrix4 projection = Matrix4::createPerspectiveFieldOfViewLH(45.0f, 640.0f / 480.0f, 0.1f, 1000.0f);
			Matrix4 view = Matrix4::createLookAtLH(camera, { 0.0f, 20.0f, 0.0f }, Vector3::up);
			SkinnedMeshShader::Lignt light;
			light.ambient = Color(0.2f, 0.2f, 0.2f, 1.0f);
			light.diffuse = Color::white;
			light.specular = Color::white;
			light.position = Vector3(0.0f, 0.0f, -300.0f);

			skinnedMeshShader.projection(projection);
			skinnedMeshShader.view(view);
			skinnedMeshShader.world(Matrix4::createRotationY(angle));
			skinnedMeshShader.light(light);
			skinnedMesh.draw(skinnedMeshShader);

			pSwapChain->Present(1, 0);
		}
	}
	pDepthStencilView->Release();
	pDepthStencil->Release();
	pBackBufferRenderTargetView->Release();
	pDeviceContext->Release();
	pDevice->Release();
	pSwapChain->Release();
	return 0;
}