#include "DirectX3D.h"

namespace TerrainRenderer
{
	DirectX3D::DirectX3D():
		mSwapChain(nullptr), mDevice(nullptr), mDeviceContext(nullptr), mRenderTargetView(nullptr), mDepthStencilBuffer(nullptr),
		mDepthStencilState(nullptr), mDepthStencilView(nullptr), mRasterState(nullptr)
	{

	}

	DirectX3D::DirectX3D(const DirectX3D& rhs)
	{

	}

	DirectX3D& DirectX3D::operator=(const DirectX3D& rhs)
	{
		return *this;
	}

	DirectX3D::~DirectX3D()
	{

	}

	bool DirectX3D::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear)
	{
		HRESULT result;
		IDXGIFactory* factory;
		IDXGIAdapter* adapter;
		IDXGIOutput* adapterOutput;
		DXGI_MODE_DESC* displayModeList;
		DXGI_ADAPTER_DESC adapterDesc;
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		D3D_FEATURE_LEVEL featureLevel;
		ID3D11Texture2D* backBufferPtr;
		D3D11_TEXTURE2D_DESC depthBufferDesc;
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		D3D11_RASTERIZER_DESC rasterDesc;
		D3D11_VIEWPORT viewport;

		int error;
		float fieldOfView, screenAspect;
		unsigned int numModes, numerator, denominator, stringLength;

		mVSyncEnabled = vsync;

		//getting the refresh rate from the video card/monitor
			//if this isn't done & the refresh rate is set to a default value that may not exist on all computers, DirectX will respond by performing a blit
			//instead of a buffer flip which will degrade performance & create errors in the debug output

		result = CreateDXGIFactory(_uuidof(IDXGIFactory), (void**)&factory);
		if (FAILED(result))
		{
			return false;
		}

		//using the factory to create an adapter for the primary graphics interface (video card)
		result = factory->EnumAdapters(0, &adapter);
		if (FAILED(result))
		{
			return false;
		}

		//enumerate the primary adapter output (monitor)
		result = adapter->EnumOutputs(0, &adapterOutput);
		if (FAILED(result))
		{
			return false;
		}

		//get the # of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor)
		result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
		if (FAILED(result))
		{
			return false;
		}

		//create a list to hold all the possible display modes for this monitor/video card combination
		displayModeList = new DXGI_MODE_DESC[numModes];
		if (!displayModeList)
		{
			return false;
		}

		//filling the display mode list structures
		result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
		if (FAILED(result))
		{
			return false;
		}

		unsigned int width;
		unsigned int height;

		//going through the display modes & finding the one that matches the screen width & height
		//when a match is found, stores the numerator & denominator of the refresh rate for that monitor
		for (unsigned int i = 0; i < numModes; ++i)
		{
			width = displayModeList[i].Width;
			height = displayModeList[i].Height;

			if (displayModeList[i].Width == static_cast<unsigned int>(screenWidth))
			{
				if (displayModeList[i].Height == static_cast<unsigned int>(screenHeight))
				{
					numerator = displayModeList[i].RefreshRate.Numerator;
					denominator = displayModeList[i].RefreshRate.Denominator;
				}
			}
		}

		//getting the adapter (video card description)
		result = adapter->GetDesc(&adapterDesc);
		if (FAILED(result))
		{
			return false;
		}

		//storing the dedicated video card memory in megabytes
		mVideoCardMemory = static_cast<int>((adapterDesc.DedicatedVideoMemory / 1024) / 1024);

		//converting the name of the video card to a char array & storing it
		error = wcstombs_s(&stringLength, mVideoCardDescription, mVCDescriptionSize, adapterDesc.Description, mVCDescriptionSize);
		if (error != 0)
		{
			return false;
		}

		//releasing the data used to get the numerator, denominator, and video card info
		delete[] displayModeList;
		displayModeList = nullptr;

		adapterOutput->Release();
		adapterOutput = nullptr;

		adapter->Release();
		adapter = nullptr;

		factory->Release();
		factory = nullptr;

		//initializing swap chain information
		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

		//setting up one back buffer
		swapChainDesc.BufferCount = 1;

		swapChainDesc.BufferDesc.Width = screenWidth;
		swapChainDesc.BufferDesc.Height = screenHeight;

		//setting regular 32-bit surface for the back buffer
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		//if vsync == true, the refresh rate will be locked to the system settings
		//if vsync == false, it will draw the screen as many times a second as it can

		if (mVSyncEnabled)
		{
			swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
		}
		else
		{
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		}

		//setting the usage of the back buffer
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

		//setting the handle for the window to render to
		swapChainDesc.OutputWindow = hwnd;

		//turning multisampling off
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;

		if (fullscreen)
		{
			swapChainDesc.Windowed = false;
		}
		else
		{
			swapChainDesc.Windowed = true;
		}

		//setting the scan line ordering & scaling to unspecified
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		//discard the back buffer contents after presenting
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		//don't set the advanced flags
		swapChainDesc.Flags = 0;

		//setting the feature level to DirectX 11
		featureLevel = D3D_FEATURE_LEVEL_11_0;

		result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &featureLevel, 1, D3D11_SDK_VERSION,
			&swapChainDesc, &mSwapChain, &mDevice, nullptr, &mDeviceContext);
		if (FAILED(result))
		{
			return false;
		}

		//getting pointer to the back buffer
		result = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
		if (FAILED(result))
		{
			return false;
		}

		//attaching the back buffer to the swap chain
		result = mDevice->CreateRenderTargetView(backBufferPtr, nullptr, &mRenderTargetView);
		if (FAILED(result))
		{
			return false;
		}

		backBufferPtr->Release();
		backBufferPtr = nullptr;

		//creating a depth buffer & attaching a stencil buffer to it
		ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

		depthBufferDesc.Width = screenWidth;
		depthBufferDesc.Height = screenHeight;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;

		//creating the depth/stencil buffer, which is a 2D texture (using CreateTexture2D)
		result = mDevice->CreateTexture2D(&depthBufferDesc, nullptr, &mDepthStencilBuffer);
		if (FAILED(result))
		{
			return false;
		}

		//setting up the depth stencil description; allows us to control what type of depth test Direct3D will do for each pixel
		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthStencilDesc.StencilEnable = true;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;

		//stencil operations if pixel is front-facing
		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		
		//stencil operations if pixel is back-facing
		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		result = mDevice->CreateDepthStencilState(&depthStencilDesc, &mDepthStencilState);
		if (FAILED(result))
		{
			return false;
		}

		mDeviceContext->OMSetDepthStencilState(mDepthStencilState, 1);

		//creating the description of the view of the depth stencil buffer
		//must be done so that Direct3D knows to use the depth buffer as a depth stencil texture

		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = mDevice->CreateDepthStencilView(mDepthStencilBuffer, &depthStencilViewDesc, &mDepthStencilView);
		if (FAILED(result))
		{
			return false;
		}

		//binding the render target view & the depth stencil buffer to the output render pipeline
		mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);

		//creating a rasterizer state (gives us control over how & what polygons are rendered

		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D11_CULL_BACK;
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0.0f;

		result = mDevice->CreateRasterizerState(&rasterDesc, &mRasterState);
		if (FAILED(result))
		{
			return false;
		}

		mDeviceContext->RSSetState(mRasterState);

		//setting up the viewport so that Direct3D can map clip space coordinates to the render target space
			//will be entire size of the window

		viewport.Width = static_cast<float>(screenWidth);
		viewport.Height = static_cast<float>(screenHeight);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;

		mDeviceContext->RSSetViewports(1, &viewport);

		//creating projection matrix; used to translate the 3D scene into the 2D viewport space 
		fieldOfView = static_cast<float>(D3DX_PI / 4.0f);
		screenAspect = static_cast<float>(screenWidth / screenHeight);

		D3DXMatrixPerspectiveFovLH(&mProjectionMatrix, fieldOfView, screenAspect, screenNear, screenDepth);

		//creating world matrix; used to convert vertices of objects into vertices in the 3D scene (rotate, translate, scale objects)
		D3DXMatrixIdentity(&mWorldMatrix);

		//creating orthographic projection matrix; used for rendering 2D elements (UI)
		D3DXMatrixOrthoLH(&mOrthoMatrix, static_cast<float>(screenWidth), static_cast<float>(screenHeight), screenNear, screenDepth);

		return true;
	}

	void DirectX3D::Shutdown()
	{
		if (mSwapChain)
		{
			mSwapChain->SetFullscreenState(FALSE, nullptr);
		}

		if (mRasterState)
		{
			mRasterState->Release();
			mRasterState = nullptr;
		}

		if (mDepthStencilView)
		{
			mDepthStencilView->Release();
			mDepthStencilView = nullptr;
		}

		if (mDepthStencilState)
		{
			mDepthStencilState->Release();
			mDepthStencilState = nullptr;
		}

		if (mDepthStencilBuffer)
		{
			mDepthStencilBuffer->Release();
			mDepthStencilBuffer = nullptr;
		}

		if (mRenderTargetView)
		{
			mRenderTargetView->Release();
			mRenderTargetView = nullptr;
		}

		if (mDeviceContext)
		{
			mDeviceContext->Release();
			mDeviceContext = nullptr;
		}

		if (mDevice)
		{
			mDevice->Release();
			mDevice = nullptr;
		}

		if (mSwapChain)
		{
			mSwapChain->Release();
			mSwapChain = nullptr;
		}
	}

	void DirectX3D::BeginScene(float r, float g, float b, float alpha)
	{
		float color[4] = { r, g, b, alpha};

		mDeviceContext->ClearRenderTargetView(mRenderTargetView, color);
		mDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	void DirectX3D::EndScene()
	{
		if (mVSyncEnabled)
		{
			mSwapChain->Present(1, 0);
		}
		else
		{
			mSwapChain->Present(0, 0);
		}
	}

	ID3D11Device* DirectX3D::GetDevice()
	{
		return mDevice;
	}

	ID3D11DeviceContext* DirectX3D::GetDeviceContext()
	{
		return mDeviceContext;
	}

	void DirectX3D::GetProjectionMatrix(D3DXMATRIX& matrix)
	{
		matrix = mProjectionMatrix;
	}

	void DirectX3D::GetWorldMatrix(D3DXMATRIX& matrix)
	{
		matrix = mWorldMatrix;
	}

	void DirectX3D::GetOrthoMatrix(D3DXMATRIX& matrix)
	{
		matrix = mOrthoMatrix;
	}

	void DirectX3D::GetVideoCardInfo(char* cardName, int& memory)
	{
		strcpy_s(cardName, mVCDescriptionSize, mVideoCardDescription);
		memory = mVideoCardMemory;
	}
}