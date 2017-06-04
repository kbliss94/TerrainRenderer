#include "System.h"

namespace TerrainRenderer
{
	System::System() :
		mInput(nullptr), mGraphics(nullptr), mFPS(nullptr), mCPU(nullptr), mTimer(nullptr)
	{
		//call initialize?
	}

	System::System(const System& rhs)
	{

	}

	System& System::operator=(const System& rhs)
	{
		return *this;
	}

	System::~System()
	{
		//call shutdown windows?
	}

	bool System::Initialize()
	{
		int screenWidth = 0;
		int screenHeight = 0;
		bool result;

		InitializeWindows(screenWidth, screenHeight);

		//setting up the Input object
		mInput = new Input;
		if (!mInput)
		{
			return false;
		}

		mInput->Initialize();

		//setting up the Graphics object
		mGraphics = new Graphics;
		if (!mGraphics)
		{
			return false;
		}

		result = mGraphics->Initialize(screenWidth, screenHeight, mHWND);
		if (!result)
		{
			return false;
		}

		//setting up the FPS object
		mFPS = new FPS;
		if (!mFPS)
		{
			return false;
		}

		mFPS->Initialize();

		//setting up the CPU object
		mCPU = new CPU;
		if (!mCPU)
		{
			return false;
		}

		mCPU->Initialize();

		//setting up the Timer object
		mTimer = new Timer;
		if (!mTimer)
		{
			return false;
		}

		result = mTimer->Initialize();
		if (!result)
		{
			MessageBox(mHWND, L"Could not initialize the Timer object.", L"Error", MB_OK);
			return false;
		}

		return true;
	}

	void System::Shutdown()
	{
		if (mTimer)
		{
			delete mTimer;
			mTimer = nullptr;
		}

		if (mCPU)
		{
			mCPU->Shutdown();
			delete mCPU;
			mCPU = nullptr;
		}

		if (mFPS)
		{
			delete mFPS;
			mFPS = nullptr;
		}

		if (mGraphics)
		{
			mGraphics->Shutdown();
			delete mGraphics;
			mGraphics = nullptr;
		}

		if (mInput)
		{
			delete mInput;
			mInput = nullptr;
		}

		ShutdownWindows();
	}

	void System::Run()
	{
		MSG msg;
		bool done = false;
		bool result;

		//initializing the message structure
		ZeroMemory(&msg, sizeof(MSG));

		//looping until there's a quit message from the window or the user
		while (!done)
		{
			//handling windows messages
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			//exit if windows signals to end the application
			if (msg.message == WM_QUIT)
			{
				done = true;
			}
			else
			{
				//frame processing
				result = Frame();
				if (!result)
				{
					done = true;
				}
			}
		}
	}

	bool System::Frame()
	{
		bool result;

		//updating the system stats
		mTimer->Frame();
		mFPS->Frame();
		mCPU->Frame();

		if (mInput->IsKeyDown(VK_ESCAPE))
		{
			return false;
		}

		result = mGraphics->Frame();
		if (!result)
		{
			return false;
		}

		return true;
	}

	LRESULT CALLBACK System::MessageHandler(HWND hwnd, UINT uint, WPARAM wparam, LPARAM lparam)
	{
		switch (uint)
		{
		case WM_KEYDOWN:
			mInput->KeyDown(static_cast<unsigned int>(wparam));
			return 0;
			break;
		case WM_KEYUP:
			mInput->KeyUp(static_cast<unsigned int>(wparam));
			return 0;
			break;
		default:
			return DefWindowProc(hwnd, uint, wparam, lparam);
		}
	}

	void System::InitializeWindows(int& screenWidth, int& screenHeight)
	{
		DEVMODE screenSettings;
		WNDCLASSEX window;
		int posX, posY;

		ApplicationHandle = this;
		mHInstance = GetModuleHandle(nullptr);
		mApplicationName = L"Engine";

		//setting up window with default settings
		window.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		window.lpfnWndProc = WndProc;
		window.cbClsExtra = 0;
		window.cbWndExtra = 0;
		window.hInstance = mHInstance;
		window.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
		window.hIconSm = window.hIcon;
		window.hCursor = LoadCursor(nullptr, IDC_ARROW);
		window.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		window.lpszMenuName = nullptr;
		window.lpszClassName = mApplicationName;
		window.cbSize = sizeof(WNDCLASSEX);

		RegisterClassEx(&window);

		//determining the resolution of the desktop screen
		screenWidth = GetSystemMetrics(SM_CXSCREEN);
		screenHeight = GetSystemMetrics(SM_CYSCREEN);

		if (FULL_SCREEN)
		{
			memset(&screenSettings, 0, sizeof(screenSettings));
			screenSettings.dmSize = sizeof(screenSettings);
			screenSettings.dmPelsWidth = static_cast<unsigned long>(screenWidth);
			screenSettings.dmPelsHeight = static_cast<unsigned long>(screenHeight);
			screenSettings.dmBitsPerPel = 32;
			screenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

			ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN);

			//setting the position of the window to the top left corner
			posX = posY = 0;
		}
		else
		{
			screenWidth = 800;
			screenHeight = 600;

			//placing the window in the center of the screen
			posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
			posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
		}

		mHWND = CreateWindowEx(WS_EX_APPWINDOW, mApplicationName, mApplicationName, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP, 
			posX, posY, screenWidth, screenHeight, nullptr, nullptr, mHInstance, nullptr);

		//bringing the window up on the screen & setting it as the main focus
		ShowWindow(mHWND, SW_SHOW);
		SetForegroundWindow(mHWND);
		SetFocus(mHWND);

		ShowCursor(false);
	}

	void System::ShutdownWindows()
	{
		ShowCursor(true);

		if (FULL_SCREEN)
		{
			ChangeDisplaySettings(nullptr, 0);
		}

		DestroyWindow(mHWND);
		mHWND = nullptr;

		UnregisterClass(mApplicationName, mHInstance);
		mHInstance = nullptr;

		ApplicationHandle = nullptr;
	}

	LRESULT CALLBACK WndProc(HWND hwnd, UINT uint, WPARAM wparam, LPARAM lparam)
	{
		switch (uint)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
			break;
		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;
			break;
		default:
			return ApplicationHandle->MessageHandler(hwnd, uint, wparam, lparam);
		}
	}
}