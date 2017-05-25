#ifndef _GRAPHICSCLASS_
#define _GRAPHICSCLASS_

#pragma once
#include <windows.h>
#include "DirectX3D.h"

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
		//!Clears the screen to a gray color
		bool Render();

		DirectX3D* mD3D;
	};

	//!Globals
	const bool FULL_SCREEN = false;
	const bool VSYNC_ENABLED = true;
	const float SCREEN_DEPTH = 1000.0f;
	const float SCREEN_NEAR = 0.1f;
}

#endif
