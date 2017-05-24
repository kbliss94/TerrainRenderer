#pragma once

using namespace std;

namespace TerrainRenderer
{
	//!Global
	const bool FULL_SCREEN = false;

	class Graphics
	{
	public:
		bool Initialize(int width, int height, HWND hwnd);
		void Shutdown();
		bool Frame();
	};
}
