#ifndef _APPLICATION_
#define _APPLICATION_

//globals
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

#include "Input.h"
#include "DirectX3D.h"
#include "Camera.h"
#include "Terrain.h"
#include "ColorShader.h"
#include "Timer.h"
#include "Position.h"
#include "FPS.h"
#include "CPU.h"
#include "FontShader.h"
#include "Text.h"

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
		Input* m_Input;
		DirectX3D* m_Direct3D;
		Camera* m_Camera;
		Terrain* m_Terrain;
		ColorShader* m_ColorShader;
		Timer* m_Timer;
		Position* m_Position;
		FPS* m_FPS;
		CPU* m_CPU;
		FontShader* m_FontShader;
		Text* m_Text;
	};
}

#endif
