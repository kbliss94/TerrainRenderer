#ifndef _GRAPHICSCLASS_
#define _GRAPHICSCLASS_

#pragma once
#include <windows.h>

using namespace std;

namespace TerrainRenderer
{
	class Graphics
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

		bool Initialize(int width, int height, HWND hwnd);
		void Shutdown();
		bool Frame();

	private:
		bool Render();
	};

	//!Globals
	const bool FULL_SCREEN = false;
	const bool VSYNC_ENABLED = true;
	const float SCREEN_DEPTH = 1000.0f;
	const float SCREEN_NEAR = 0.1f;
}

#endif
