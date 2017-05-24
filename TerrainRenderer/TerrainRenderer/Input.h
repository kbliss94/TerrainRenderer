#pragma once

using namespace std;

namespace TerrainRenderer
{
	class Input
	{
	public:
		void Initialize();
		void KeyDown(unsigned int key);
		void KeyUp(unsigned int key);
		bool IsKeyDown(unsigned int key);
	};
}
