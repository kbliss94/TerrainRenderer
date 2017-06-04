#include "Graphics.h"

namespace TerrainRenderer
{
	Graphics::Graphics():
		mD3D(nullptr), mCamera(nullptr), mModel(nullptr), mColorShader(nullptr)
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

		//setting up Model object
		mModel = new Model;
		if (!mModel)
		{
			return false;
		}

		result = mModel->Initialize(mD3D->GetDevice());
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}

		//setting up the color shader object
		mColorShader = new ColorShader;
		if (!mColorShader)
		{
			return false;
		}

		result = mColorShader->Initialize(mD3D->GetDevice(), hwnd);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
			return false;
		}

		return true;
	}

	void Graphics::Shutdown()
	{
		if (mColorShader)
		{
			mColorShader->Shutdown();
			delete mColorShader;
			mColorShader = nullptr;
		}

		if (mModel)
		{
			mModel->Shutdown();
			delete mModel;
			mModel = nullptr;
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
		return Render();
	}

	bool Graphics::Render()
	{
		D3DXMATRIX viewMatrix, projectionMatrix, worldMatrix;
		bool result;

		mD3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

		mCamera->Render();

		mCamera->GetViewMatrix(viewMatrix);
		mD3D->GetWorldMatrix(worldMatrix);
		mD3D->GetProjectionMatrix(projectionMatrix);

		//putting the model vertex/index buffers on the graphics pipeline to prepare them for drawing
		mModel->Render(mD3D->GetDeviceContext());

		//rendering the model using the color shader
		result = mColorShader->Render(mD3D->GetDeviceContext(), mModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		if (!result)
		{
			return false;
		}

		//presents the rendered scene to the screen
		mD3D->EndScene();

		return true;
	}
}