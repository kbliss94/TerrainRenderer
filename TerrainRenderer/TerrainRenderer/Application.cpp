#include "Application.h"

namespace TerrainRenderer
{
	Application::Application()
	{
		mInput = 0;
		mDirect3D = 0;
		mCamera = 0;
		mTimer = 0;
		mPosition = 0;
		mFPS = 0;
		mCPU = 0;
		mFontShader = 0;
		mText = 0;
		mTerrainManager = 0;
		mTerrainShader = 0;
		mLight = 0;
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
		unsigned timeSeed = std::chrono::system_clock::now().time_since_epoch().count();
		//int testSeed = 0;

		for (int i = 0; i < mNumStartUpMaps; ++i)
		{
			heightMapGenerator.Generate(mHeightMapFilenames[i], mHMWidth, mHMHeight);

			//using the time as a seed for the height map generator
			heightMapGenerator.SetSeed(timeSeed);
			timeSeed = std::chrono::system_clock::now().time_since_epoch().count();

			//heightMapGenerator.SetSeed(testSeed);
			//++testSeed;
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
		mInput = new Input;
		if (!mInput)
		{
			return false;
		}

		// Initialize the input object.
		result = mInput->Initialize(hinstance, hwnd, screenWidth, screenHeight);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the input object.", L"Error", MB_OK);
			return false;
		}

		// Create the Direct3D object.
		mDirect3D = new DirectX3D;
		if (!mDirect3D)
		{
			return false;
		}

		// Initialize the Direct3D object.
		result = mDirect3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize DirectX 11.", L"Error", MB_OK);
			return false;
		}

		// Create the camera object.
		mCamera = new Camera;
		if (!mCamera)
		{
			return false;
		}

		// Initialize a base view matrix with the camera for 2D user interface rendering.
		mCamera->SetPosition(0.0f, 0.0f, -1.0f);
		mCamera->Render();
		mCamera->GetViewMatrix(baseViewMatrix);

		// Set the initial position of the camera.
		//cameraX = 50.0f;
		//cameraY = 15.0f;
		//cameraZ = -7.0f;

		cameraX = 94.0f;
		cameraY = 90.0f;
		cameraZ = 94.0f;

		mCamera->SetPosition(cameraX, cameraY, cameraZ);

		//setting up the terrain manager
		mTerrainManager = new TerrainManager;
		if (!mTerrainManager)
		{
			return false;
		}

		result = mTerrainManager->Initialize(mDirect3D->GetDevice(), &mHeightMapFilenames, &mScalingFilenames, mLargeScalingFilename, 
			mGrassFilename, mSlopeFilename, mRockFilename);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the terrain manager object.", L"Error", MB_OK);
			return false;
		}

		//// Create the color shader object.
		//m_ColorShader = new ColorShader;
		//if (!m_ColorShader)
		//{
		//	return false;
		//}

		//// Initialize the color shader object.
		//result = m_ColorShader->Initialize(mDirect3D->GetDevice(), hwnd);
		//if (!result)
		//{
		//	MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
		//	return false;
		//}

		// Create the timer object.
		mTimer = new Timer;
		if (!mTimer)
		{
			return false;
		}

		// Initialize the timer object.
		result = mTimer->Initialize();
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the timer object.", L"Error", MB_OK);
			return false;
		}

		// Create the position object.
		mPosition = new Position;
		if (!mPosition)
		{
			return false;
		}

		// Set the initial position of the viewer to the same as the initial camera position.
		mPosition->SetPosition(cameraX, cameraY, cameraZ);

		// Create the fps object.
		mFPS = new FPS;
		if (!mFPS)
		{
			return false;
		}

		// Initialize the fps object.
		mFPS->Initialize();

		// Create the cpu object.
		mCPU = new CPU;
		if (!mCPU)
		{
			return false;
		}

		// Initialize the cpu object.
		mCPU->Initialize();

		// Create the font shader object.
		mFontShader = new FontShader;
		if (!mFontShader)
		{
			return false;
		}

		// Initialize the font shader object.
		result = mFontShader->Initialize(mDirect3D->GetDevice(), hwnd);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the font shader object.", L"Error", MB_OK);
			return false;
		}

		// Create the text object.
		mText = new Text;
		if (!mText)
		{
			return false;
		}

		// Initialize the text object.
		result = mText->Initialize(mDirect3D->GetDevice(), mDirect3D->GetDeviceContext(), hwnd, screenWidth, screenHeight, baseViewMatrix);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
			return false;
		}

		// Retrieve the video card information.
		mDirect3D->GetVideoCardInfo(videoCard, videoMemory);

		// Set the video card information in the text object.
		result = mText->SetVideoCardInfo(videoCard, videoMemory, mDirect3D->GetDeviceContext());
		if (!result)
		{
			MessageBox(hwnd, L"Could not set video card info in the text object.", L"Error", MB_OK);
			return false;
		}

		//create the terrain shader object
		mTerrainShader = new TerrainShader;
		if (!mTerrainShader)
		{
			return false;
		}

		//initialize the terrain shader object
		result = mTerrainShader->Initialize(mDirect3D->GetDevice(), hwnd);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the terrain shader object.", L"Error", MB_OK);
			return false;
		}

		//create the light object
		mLight = new Light;
		if (!mLight)
		{
			return false;
		}

		// Initialize the light object.
		//mLight->SetAmbientColor(0.05f, 0.05f, 0.05f, 1.0f);
		//mLight->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		//mLight->SetDirection(-0.5f, -1.0f, 0.0f);
		mLight->SetAmbientColor(1.0f, 1.0f, 1.0f, 1.0f);
		mLight->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		mLight->SetDirection(-0.5f, -1.0f, 0.0f);

		return true;
	}


	void Application::Shutdown()
	{
		// Release the light object.
		if (mLight)
		{
			delete mLight;
			mLight = 0;
		}

		// Release the terrain shader object.
		if (mTerrainShader)
		{
			mTerrainShader->Shutdown();
			delete mTerrainShader;
			mTerrainShader = 0;
		}

		// Release the text object.
		if (mText)
		{
			mText->Shutdown();
			delete mText;
			mText = 0;
		}

		// Release the font shader object.
		if (mFontShader)
		{
			mFontShader->Shutdown();
			delete mFontShader;
			mFontShader = 0;
		}

		// Release the cpu object.
		if (mCPU)
		{
			mCPU->Shutdown();
			delete mCPU;
			mCPU = 0;
		}

		// Release the fps object.
		if (mFPS)
		{
			delete mFPS;
			mFPS = 0;
		}

		// Release the position object.
		if (mPosition)
		{
			delete mPosition;
			mPosition = 0;
		}

		// Release the timer object.
		if (mTimer)
		{
			delete mTimer;
			mTimer = 0;
		}

		//// Release the color shader object.
		//if (m_ColorShader)
		//{
		//	m_ColorShader->Shutdown();
		//	delete m_ColorShader;
		//	m_ColorShader = 0;
		//}

		//release the terrain manager object
		if (mTerrainManager)
		{
			mTerrainManager->Shutdown();
			delete mTerrainManager;
			mTerrainManager = 0;
		}

		// Release the camera object.
		if (mCamera)
		{
			delete mCamera;
			mCamera = 0;
		}

		// Release the Direct3D object.
		if (mDirect3D)
		{
			mDirect3D->Shutdown();
			delete mDirect3D;
			mDirect3D = 0;
		}

		// Release the input object.
		if (mInput)
		{
			mInput->Shutdown();
			delete mInput;
			mInput = 0;
		}
	}


	bool Application::Frame()
	{
		bool result;


		// Read the user input.
		result = mInput->Frame();
		if (!result)
		{
			return false;
		}

		// Check if the user pressed escape and wants to exit the application.
		if (mInput->IsEscapePressed() == true)
		{
			return false;
		}

		// Update the system stats.
		mTimer->Frame();
		mFPS->Frame();
		mCPU->Frame();

		// Update the FPS value in the text object.
		result = mText->SetFps(mFPS->GetFPS(), mDirect3D->GetDeviceContext());
		if (!result)
		{
			return false;
		}

		// Update the CPU usage value in the text object.
		result = mText->SetCpu(mCPU->GetCPUPercentage(), mDirect3D->GetDeviceContext());
		if (!result)
		{
			return false;
		}

		// Do the frame input processing.
		result = HandleInput(mTimer->GetTime());
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
		mPosition->SetFrameTime(frameTime);

		// Handle the input.

		//toggling generation
		keyDown = mInput->IsGPressed();

		//turning left
		keyDown = mInput->IsAPressed();
		mPosition->TurnLeft(keyDown);

		//turning right
		keyDown = mInput->IsDPressed();
		mPosition->TurnRight(keyDown);

		//moving forward
		keyDown = mInput->IsWPressed();
		mPosition->MoveForward(keyDown);

		if (keyDown && GENERATION_ENABLED)
		{
			mTerrainManager->GenerateChunks(mPosition);
		}

		//moving backward
		keyDown = mInput->IsSPressed();
		mPosition->MoveBackward(keyDown);

		if (keyDown && GENERATION_ENABLED)
		{
			mTerrainManager->GenerateChunks(mPosition);
		}

		//moving up
		keyDown = mInput->IsQPressed();
		mPosition->MoveUpward(keyDown);

		//moving down
		keyDown = mInput->IsEPressed();
		mPosition->MoveDownward(keyDown);

		//looking up
		keyDown = mInput->IsPgUpPressed();
		mPosition->LookUpward(keyDown);

		//looking down
		keyDown = mInput->IsPgDownPressed();
		mPosition->LookDownward(keyDown);

		// Get the view point position/rotation.
		mPosition->GetPosition(posX, posY, posZ);
		mPosition->GetRotation(rotX, rotY, rotZ);

		// Set the position of the camera.
		mCamera->SetPosition(posX, posY, posZ);
		mCamera->SetRotation(rotX, rotY, rotZ);

		// Update the position values in the text object.
		result = mText->SetCameraPosition(posX, posY, posZ, mDirect3D->GetDeviceContext());
		if (!result)
		{
			return false;
		}

		// Update the rotation values in the text object.
		result = mText->SetCameraRotation(rotX, rotY, rotZ, mDirect3D->GetDeviceContext());
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
		mDirect3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

		// Generate the view matrix based on the camera's position.
		mCamera->Render();

		// Get the world, view, projection, and orthographic matrices from the camera and Direct3D objects.
		mDirect3D->GetWorldMatrix(worldMatrix);
		mCamera->GetViewMatrix(viewMatrix);
		mDirect3D->GetProjectionMatrix(projectionMatrix);
		mDirect3D->GetOrthoMatrix(orthoMatrix);

		//rendering the terrain buffers
		mTerrainManager->Render(mDirect3D->GetDeviceContext(), mTerrainShader, worldMatrix, viewMatrix, projectionMatrix,
			mLight->GetAmbientColor(), mLight->GetDiffuseColor(), mLight->GetDirection());

		// Turn off the Z buffer to begin all 2D rendering.
		mDirect3D->TurnZBufferOff();

		// Turn on the alpha blending before rendering the text.
		mDirect3D->TurnOnAlphaBlending();

		// Render the text user interface elements.
		result = mText->Render(mDirect3D->GetDeviceContext(), mFontShader, worldMatrix, orthoMatrix);
		if (!result)
		{
			return false;
		}

		// Turn off alpha blending after rendering the text.
		mDirect3D->TurnOffAlphaBlending();

		// Turn the Z buffer back on now that all 2D rendering has completed.
		mDirect3D->TurnZBufferOn();

		// Present the rendered scene to the screen.
		mDirect3D->EndScene();

		return true;
	}
}
