#include "Graphics.h"

namespace TerrainRenderer
{
	Graphics::Graphics():
		mD3D(nullptr), mCamera(nullptr), mTextureShader(nullptr), mBitmap(nullptr)
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

		//setting up DirectX3D object
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

		//setting up Camera object
		mCamera = new Camera;
		if (!mCamera)
		{
			return false;
		}

		mCamera->SetPosition(0.0f, 0.0f, -10.0f);

		//setting up TextureShader object
		mTextureShader = new TextureShader;
		if (!mTextureShader)
		{
			return false;
		}

		result = mTextureShader->Initialize(mD3D->GetDevice(), hwnd);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
			return false;
		}

		//setting up the Bitmap object
		mBitmap = new Bitmap;
		if (!mBitmap)
		{
			return false;
		}

		result = mBitmap->Initialize(mD3D->GetDevice(), screenWidth, screenHeight, L"../TerrainRenderer/data/seafloor.dds", 256, 256);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
			return false;
		}

		return true;
	}

	void Graphics::Shutdown()
	{
		if (mBitmap)
		{
			mBitmap->Shutdown();
			delete mBitmap;
			mBitmap = nullptr;
		}

		if (mTextureShader)
		{
			mTextureShader->Shutdown();
			delete mTextureShader;
			mTextureShader = nullptr;
		}

		if (mCamera)
		{
			delete mCamera;
			mCamera = nullptr;
		}

		if (mD3D)
		{
			mD3D->Shutdown();
			delete mD3D;
			mD3D = nullptr;
		}
	}

	bool Graphics::Frame()
	{
		bool result;
		static float rotation = 0.0f;

		//updating the rotation variable each frame
		rotation += static_cast<float>(D3DX_PI * 0.01f);
		if (rotation > 360.0f)
		{
			rotation -= 360.0f;
		}

		//rendering the scene
		result = Render(rotation);
		if (!result)
		{
			return false;
		}

		return true;
	}

	bool Graphics::Render(float rotation)
	{
		D3DXMATRIX viewMatrix, projectionMatrix, worldMatrix, orthoMatrix;
		bool result;

		mD3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

		mCamera->Render();

		mCamera->GetViewMatrix(viewMatrix);
		mD3D->GetWorldMatrix(worldMatrix);
		mD3D->GetProjectionMatrix(projectionMatrix);

		//getting the ortho matrix for 2D rendering to pass in instead of the projection matrix
		mD3D->GetOrthoMatrix(orthoMatrix);

		mD3D->TurnZBufferOff();

		result = mBitmap->Render(mD3D->GetDeviceContext(), 100, 100);
		if (!result)
		{
			return false;
		}

		result = mTextureShader->Render(mD3D->GetDeviceContext(), mBitmap->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix, mBitmap->GetTexture());
		if (!result)
		{
			return false;
		}

		mD3D->TurnZBufferOn();

		//presenting the rendered scene to the screen
		mD3D->EndScene();

		return true;
	}
}