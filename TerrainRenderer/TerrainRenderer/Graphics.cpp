#include "Graphics.h"

namespace TerrainRenderer
{
	Graphics::Graphics():
		mD3D(nullptr)
	{
		//call initialize?
	}

	Graphics::Graphics(const Graphics& rhs)
	{

	}

	Graphics& Graphics::operator=(const Graphics& rhs)
	{
		return *this;
	}

	Graphics::~Graphics()
	{

	}

	bool Graphics::Initialize(int screenWidth, int screenHeight, HWND hwnd)
	{
		bool result;

		mD3D = new DirectX3D;
		if (!mD3D)
		{
			return false;
		}

		result = mD3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
			return false;
		}

		return true;
	}

	void Graphics::Shutdown()
	{
		if (mD3D)
		{
			mD3D->Shutdown();
			delete mD3D;
			mD3D = nullptr;
		}
	}

	bool Graphics::Frame()
	{
		return Render();
	}

	bool Graphics::Render()
	{
		//clearing buffers to begin the scene
		mD3D->BeginScene(0.5f, 0.5f, 0.5f, 1.0f);

		//presents the rendered scene to the screen
		mD3D->EndScene();

		return true;
	}
}