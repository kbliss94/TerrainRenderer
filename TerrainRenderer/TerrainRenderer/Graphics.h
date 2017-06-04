#ifndef _GRAPHICS_
#define _GRAPHICS_

#pragma once
#include <windows.h>
#include "DirectX3D.h"
#include "Camera.h"
#include "Model.h"
//#include "ColorShader.h"
#include "TextureShader.h"

using namespace std;

namespace TerrainRenderer
{
	class Graphics final
	{
	public:
		//!Constructor
		Graphics();

		//!Copy constructor
		Graphics(const Graphics& rhs);

		//!Assignment operator
		Graphics& operator=(const Graphics& rhs);

		//!Destructor
		~Graphics();

		bool Initialize(int screenWidth, int screenHeight, HWND hwnd);

		void Shutdown();

		//!Calls Render() each frame
		/*!
		\return true if able to render & false otherwise
		*/
		bool Frame();

	private:
		//!Creates a view matrix based on the camera's location & draws to the back buffer in order to render to the screen
		bool Render();

		DirectX3D* mD3D;
		Camera* mCamera;
		Model* mModel;
		//ColorShader* mColorShader;
		TextureShader* mTextureShader;
	};

	//!Globals
	const bool FULL_SCREEN = false;
	const bool VSYNC_ENABLED = false;
	const float SCREEN_DEPTH = 1000.0f;
	const float SCREEN_NEAR = 0.1f;
}

#endif
