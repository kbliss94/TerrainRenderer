#ifndef _APPLICATION_
#define _APPLICATION_

//globals
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

const bool STITCHING_ENABLED = true;
const bool GENERATION_ENABLED = true;
const bool SCALING_ENABLED = true;
const bool RANDOM_GENERATION = false;
const bool QUADTREES_ENABLED = true;

const int HM_WIDTH = 128;
const int HM_HEIGHT = 128;

#include "Input.h"
#include "DirectX3D.h"
#include "Camera.h"
#include "TerrainManager.h"

#include "Timer.h"
#include "Position.h"
#include "FPS.h"
#include "CPU.h"
#include "FontShader.h"
#include "Text.h"
#include "HeightMap.h"
#include "TerrainShader.h"
#include "Light.h"
#include "Frustum.h"
#include "SkyDome.h"
#include "SkyDomeShader.h"
#include "noiseutils.h"

#include <random>
#include <chrono>
#include <noise.h>

namespace TerrainRenderer
{
	class Application
	{
	public:
		Application();
		Application(const Application& rhs);
		~Application();

		bool Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
		void Shutdown();
		bool Frame();

	private:
		bool HandleInput(float);
		bool RenderGraphics();

	private:
		Input* mInput;
		DirectX3D* mDirect3D;
		Camera* mCamera;
		Timer* mTimer;
		Position* mPosition;
		FPS* mFPS;
		CPU* mCPU;
		FontShader* mFontShader;
		Text* mText;
		TerrainManager* mTerrainManager;
		TerrainShader* mTerrainShader;
		Frustum* mFrustum;
		Light* mLight;
		SkyDome* mSkyDome;
		SkyDomeShader* mSkyDomeShader;

		const int mNumStartUpMaps = 9;

		vector<char*> mHeightMapFilenames;
		vector<char*> mScalingFilenames;
		char* mLargeScalingFilename;

		WCHAR* mGrassFilename = L"..//TerrainRenderer//data//grass1.dds";
		WCHAR* mSlopeFilename = L"..//TerrainRenderer//data//rock.dds";
		WCHAR* mRockFilename = L"..//TerrainRenderer//data//snow.dds";
	};
}

#endif