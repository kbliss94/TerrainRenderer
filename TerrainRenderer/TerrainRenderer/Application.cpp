#include "Application.h"

namespace TerrainRenderer
{
	Application::Application()
	{
		m_Input = 0;
		m_Direct3D = 0;
		m_Camera = 0;
		m_ColorShader = 0;
		m_Timer = 0;
		m_Position = 0;
		m_FPS = 0;
		m_CPU = 0;
		m_FontShader = 0;
		m_Text = 0;
		mTerrainManager = 0;
	}

	Application::Application(const Application& other)
	{

	}

	Application::~Application()
	{

	}


	bool Application::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
	{
		HeightMap heightMapGenerator;
		bool result;
		float cameraX, cameraY, cameraZ;
		D3DXMATRIX baseViewMatrix;
		char videoCard[128];
		int videoMemory;

		//Create the original 9 height maps for the starting 3x3 grid
		mHeightMapFilenames.push_back("..//TerrainRenderer//data//HM0.bmp");
		mHeightMapFilenames.push_back("..//TerrainRenderer//data//HM1.bmp");
		mHeightMapFilenames.push_back("..//TerrainRenderer//data//HM2.bmp");
		mHeightMapFilenames.push_back("..//TerrainRenderer//data//HM3.bmp");
		mHeightMapFilenames.push_back("..//TerrainRenderer//data//HM4.bmp");
		mHeightMapFilenames.push_back("..//TerrainRenderer//data//HM5.bmp");
		mHeightMapFilenames.push_back("..//TerrainRenderer//data//HM6.bmp");
		mHeightMapFilenames.push_back("..//TerrainRenderer//data//HM7.bmp");
		mHeightMapFilenames.push_back("..//TerrainRenderer//data//HM8.bmp");

		heightMapGenerator.SetIsScaleMap(false);

		for (int i = 0; i < mNumStartUpMaps; ++i)
		{
			heightMapGenerator.Generate(mHeightMapFilenames[i], mHMWidth, mHMHeight);

			//generate random int for the seed, set the seed
			heightMapGenerator.SetSeed(mDistribution(mRandomSeedGenerator));
		}

		//the filenames for the scaling chunks created from the big scaling map
		mScalingFilenames.push_back("..//TerrainRenderer//data//scaling0.bmp");
		mScalingFilenames.push_back("..//TerrainRenderer//data//scaling1.bmp");
		mScalingFilenames.push_back("..//TerrainRenderer//data//scaling2.bmp");
		mScalingFilenames.push_back("..//TerrainRenderer//data//scaling3.bmp");
		mScalingFilenames.push_back("..//TerrainRenderer//data//scaling4.bmp");
		mScalingFilenames.push_back("..//TerrainRenderer//data//scaling5.bmp");
		mScalingFilenames.push_back("..//TerrainRenderer//data//scaling6.bmp");
		mScalingFilenames.push_back("..//TerrainRenderer//data//scaling7.bmp");
		mScalingFilenames.push_back("..//TerrainRenderer//data//scaling8.bmp");

		//creating the big scaling map
		heightMapGenerator.SetIsScaleMap(true);
		char* mLargeScalingFilename = "..//TerrainRenderer//data//scalingMap.bmp";
		heightMapGenerator.SetSeed(70 /*Distribution(RandomSeedGenerator)*/);
		heightMapGenerator.Generate(mLargeScalingFilename, (mHMWidth * 3), (mHMHeight * 3));

		// Create the input object.  The input object will be used to handle reading the keyboard and mouse input from the user.
		m_Input = new Input;
		if (!m_Input)
		{
			return false;
		}

		// Initialize the input object.
		result = m_Input->Initialize(hinstance, hwnd, screenWidth, screenHeight);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the input object.", L"Error", MB_OK);
			return false;
		}

		// Create the Direct3D object.
		m_Direct3D = new DirectX3D;
		if (!m_Direct3D)
		{
			return false;
		}

		// Initialize the Direct3D object.
		result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize DirectX 11.", L"Error", MB_OK);
			return false;
		}

		// Create the camera object.
		m_Camera = new Camera;
		if (!m_Camera)
		{
			return false;
		}

		// Initialize a base view matrix with the camera for 2D user interface rendering.
		m_Camera->SetPosition(0.0f, 0.0f, -1.0f);
		m_Camera->Render();
		m_Camera->GetViewMatrix(baseViewMatrix);

		// Set the initial position of the camera.
		//cameraX = 50.0f;
		//cameraY = 15.0f;
		//cameraZ = -7.0f;

		cameraX = 94.0f;
		cameraY = 90.0f;
		cameraZ = 94.0f;

		m_Camera->SetPosition(cameraX, cameraY, cameraZ);

		//setting up the terrain manager
		mTerrainManager = new TerrainManager;
		if (!mTerrainManager)
		{
			return false;
		}

		result = mTerrainManager->Initialize(m_Direct3D->GetDevice(), &mHeightMapFilenames, &mScalingFilenames, mLargeScalingFilename);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the terrain manager object.", L"Error", MB_OK);
			return false;
		}

		// Create the color shader object.
		m_ColorShader = new ColorShader;
		if (!m_ColorShader)
		{
			return false;
		}

		// Initialize the color shader object.
		result = m_ColorShader->Initialize(m_Direct3D->GetDevice(), hwnd);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
			return false;
		}

		// Create the timer object.
		m_Timer = new Timer;
		if (!m_Timer)
		{
			return false;
		}

		// Initialize the timer object.
		result = m_Timer->Initialize();
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the timer object.", L"Error", MB_OK);
			return false;
		}

		// Create the position object.
		m_Position = new Position;
		if (!m_Position)
		{
			return false;
		}

		// Set the initial position of the viewer to the same as the initial camera position.
		m_Position->SetPosition(cameraX, cameraY, cameraZ);

		// Create the fps object.
		m_FPS = new FPS;
		if (!m_FPS)
		{
			return false;
		}

		// Initialize the fps object.
		m_FPS->Initialize();

		// Create the cpu object.
		m_CPU = new CPU;
		if (!m_CPU)
		{
			return false;
		}

		// Initialize the cpu object.
		m_CPU->Initialize();

		// Create the font shader object.
		m_FontShader = new FontShader;
		if (!m_FontShader)
		{
			return false;
		}

		// Initialize the font shader object.
		result = m_FontShader->Initialize(m_Direct3D->GetDevice(), hwnd);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the font shader object.", L"Error", MB_OK);
			return false;
		}

		// Create the text object.
		m_Text = new Text;
		if (!m_Text)
		{
			return false;
		}

		// Initialize the text object.
		result = m_Text->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), hwnd, screenWidth, screenHeight, baseViewMatrix);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
			return false;
		}

		// Retrieve the video card information.
		m_Direct3D->GetVideoCardInfo(videoCard, videoMemory);

		// Set the video card information in the text object.
		result = m_Text->SetVideoCardInfo(videoCard, videoMemory, m_Direct3D->GetDeviceContext());
		if (!result)
		{
			MessageBox(hwnd, L"Could not set video card info in the text object.", L"Error", MB_OK);
			return false;
		}

		return true;
	}


	void Application::Shutdown()
	{
		// Release the text object.
		if (m_Text)
		{
			m_Text->Shutdown();
			delete m_Text;
			m_Text = 0;
		}

		// Release the font shader object.
		if (m_FontShader)
		{
			m_FontShader->Shutdown();
			delete m_FontShader;
			m_FontShader = 0;
		}

		// Release the cpu object.
		if (m_CPU)
		{
			m_CPU->Shutdown();
			delete m_CPU;
			m_CPU = 0;
		}

		// Release the fps object.
		if (m_FPS)
		{
			delete m_FPS;
			m_FPS = 0;
		}

		// Release the position object.
		if (m_Position)
		{
			delete m_Position;
			m_Position = 0;
		}

		// Release the timer object.
		if (m_Timer)
		{
			delete m_Timer;
			m_Timer = 0;
		}

		// Release the color shader object.
		if (m_ColorShader)
		{
			m_ColorShader->Shutdown();
			delete m_ColorShader;
			m_ColorShader = 0;
		}

		//release the terrain manager object
		if (mTerrainManager)
		{
			mTerrainManager->Shutdown();
			delete mTerrainManager;
			mTerrainManager = 0;
		}

		// Release the camera object.
		if (m_Camera)
		{
			delete m_Camera;
			m_Camera = 0;
		}

		// Release the Direct3D object.
		if (m_Direct3D)
		{
			m_Direct3D->Shutdown();
			delete m_Direct3D;
			m_Direct3D = 0;
		}

		// Release the input object.
		if (m_Input)
		{
			m_Input->Shutdown();
			delete m_Input;
			m_Input = 0;
		}

		return;
	}


	bool Application::Frame()
	{
		bool result;


		// Read the user input.
		result = m_Input->Frame();
		if (!result)
		{
			return false;
		}

		// Check if the user pressed escape and wants to exit the application.
		if (m_Input->IsEscapePressed() == true)
		{
			return false;
		}

		// Update the system stats.
		m_Timer->Frame();
		m_FPS->Frame();
		m_CPU->Frame();

		// Update the FPS value in the text object.
		result = m_Text->SetFps(m_FPS->GetFPS(), m_Direct3D->GetDeviceContext());
		if (!result)
		{
			return false;
		}

		// Update the CPU usage value in the text object.
		result = m_Text->SetCpu(m_CPU->GetCPUPercentage(), m_Direct3D->GetDeviceContext());
		if (!result)
		{
			return false;
		}

		// Do the frame input processing.
		result = HandleInput(m_Timer->GetTime());
		if (!result)
		{
			return false;
		}

		// Render the graphics.
		result = RenderGraphics();
		if (!result)
		{
			return false;
		}

		return result;
	}


	bool Application::HandleInput(float frameTime)
	{
		bool keyDown, result;
		float posX, posY, posZ, rotX, rotY, rotZ;


		// Set the frame time for calculating the updated position.
		m_Position->SetFrameTime(frameTime);

		// Handle the input.

		//turning left
		keyDown = m_Input->IsAPressed();
		m_Position->TurnLeft(keyDown);

		//turning right
		keyDown = m_Input->IsDPressed();
		m_Position->TurnRight(keyDown);

		//moving forward
		keyDown = m_Input->IsWPressed();
		m_Position->MoveForward(keyDown);

		if (keyDown && GENERATION_ENABLED)
		{
			mTerrainManager->GenerateChunks(m_Position);
		}

		//moving backward
		keyDown = m_Input->IsSPressed();
		m_Position->MoveBackward(keyDown);

		if (keyDown && GENERATION_ENABLED)
		{
			mTerrainManager->GenerateChunks(m_Position);
		}

		//moving up
		keyDown = m_Input->IsQPressed();
		m_Position->MoveUpward(keyDown);

		//moving down
		keyDown = m_Input->IsEPressed();
		m_Position->MoveDownward(keyDown);

		//looking up
		keyDown = m_Input->IsPgUpPressed();
		m_Position->LookUpward(keyDown);

		//looking down
		keyDown = m_Input->IsPgDownPressed();
		m_Position->LookDownward(keyDown);

		// Get the view point position/rotation.
		m_Position->GetPosition(posX, posY, posZ);
		m_Position->GetRotation(rotX, rotY, rotZ);

		// Set the position of the camera.
		m_Camera->SetPosition(posX, posY, posZ);
		m_Camera->SetRotation(rotX, rotY, rotZ);

		// Update the position values in the text object.
		result = m_Text->SetCameraPosition(posX, posY, posZ, m_Direct3D->GetDeviceContext());
		if (!result)
		{
			return false;
		}

		// Update the rotation values in the text object.
		result = m_Text->SetCameraRotation(rotX, rotY, rotZ, m_Direct3D->GetDeviceContext());
		if (!result)
		{
			return false;
		}

		return true;
	}


	bool Application::RenderGraphics()
	{
		D3DXMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
		bool result;


		// Clear the scene.
		m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

		// Generate the view matrix based on the camera's position.
		m_Camera->Render();

		// Get the world, view, projection, and orthographic matrices from the camera and Direct3D objects.
		m_Direct3D->GetWorldMatrix(worldMatrix);
		m_Camera->GetViewMatrix(viewMatrix);
		m_Direct3D->GetProjectionMatrix(projectionMatrix);
		m_Direct3D->GetOrthoMatrix(orthoMatrix);

		//rendering the terrain buffers
		mTerrainManager->Render(m_Direct3D->GetDeviceContext(), m_ColorShader, worldMatrix, viewMatrix, projectionMatrix);

		// Turn off the Z buffer to begin all 2D rendering.
		m_Direct3D->TurnZBufferOff();

		// Turn on the alpha blending before rendering the text.
		m_Direct3D->TurnOnAlphaBlending();

		// Render the text user interface elements.
		result = m_Text->Render(m_Direct3D->GetDeviceContext(), m_FontShader, worldMatrix, orthoMatrix);
		if (!result)
		{
			return false;
		}

		// Turn off alpha blending after rendering the text.
		m_Direct3D->TurnOffAlphaBlending();

		// Turn the Z buffer back on now that all 2D rendering has completed.
		m_Direct3D->TurnZBufferOn();

		// Present the rendered scene to the screen.
		m_Direct3D->EndScene();

		return true;
	}
}
