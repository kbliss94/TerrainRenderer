#ifndef _DIRECTX3D_
#define _DIRECTX3D_

#pragma once

//linking
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3dx10math.h>

using namespace std;

namespace TerrainRenderer
{
	class DirectX3D
	{
	public:
		//!Constructor
		DirectX3D();

		//!Copy constructor
		DirectX3D(const DirectX3D& rhs);

		//!Assignment operator
		DirectX3D& operator=(const DirectX3D& rhs);

		//!Destructor
		~DirectX3D();

		//!Sets up Direct3D for DirectX11
		/*!
		\param vsync indicates whether rendering should be done according to the user's monitor refresh rate or as fast as possible
		\param hwnd handle to the window
		*/
		bool Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear);
		
		//!Releases and nulls out all member pointers
		void Shutdown();

		//!Initializes buffers so they are ready to be drawn to
		void BeginScene(float r, float g, float b, float alpha);

		//!Tells swap chain to display 3D scene once all drawing has completed at the end of each frame
		void EndScene();

		ID3D11Device* GetDevice();
		ID3D11DeviceContext* GetDeviceContext();
		void GetProjectionMatrix(D3DXMATRIX& matrix);
		void GetWorldMatrix(D3DXMATRIX& matrix);
		void GetOrthoMatrix(D3DXMATRIX& matrix);
		void GetVideoCardInfo(char* cardName, int& memory);

		void TurnZBufferOn();
		void TurnZBufferOff();

	private:
		const static int mVCDescriptionSize = 128;
		bool mVSyncEnabled;
		int mVideoCardMemory;
		char mVideoCardDescription[mVCDescriptionSize];
		IDXGISwapChain* mSwapChain;
		ID3D11Device* mDevice;
		ID3D11DeviceContext* mDeviceContext;
		ID3D11RenderTargetView* mRenderTargetView;
		ID3D11Texture2D* mDepthStencilBuffer;
		ID3D11DepthStencilState* mDepthStencilState;
		ID3D11DepthStencilView* mDepthStencilView;
		ID3D11RasterizerState* mRasterState;
		D3DXMATRIX mProjectionMatrix;
		D3DXMATRIX mWorldMatrix;
		D3DXMATRIX mOrthoMatrix;
		ID3D11DepthStencilState* mDepthDisabledStencilState;
	};
}

#endif
