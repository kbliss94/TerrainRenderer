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

		//going through the display modes & finding the one that matches the screen width & height
		//when a match is found, stores the numerator & denominator of the refresh rate for that monitor
		for (int i = 0; i < numModes; ++i)
		{
			if (displayModeList[i].Width == static_cast<unsigned int>(screenHeight))
			{
				if (displayModeList[i].Height == static_cast<unsigned int>(screenWidth))
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

	}
}