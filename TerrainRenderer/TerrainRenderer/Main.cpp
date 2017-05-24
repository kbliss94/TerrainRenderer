#include "System.h"

using namespace std;

//!This terrain renderer will procedurally texture and render a large-scale terrain.
/*!*/

namespace TerrainRenderer
{
	int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
	{
		System* system;
		bool result;

		system = new System;
		if (!system)
		{
			return 0;
		}

		result = system->Initialize();
		if (result)
		{
			system->Run();
		}

		system->Shutdown();
		delete system;
		system = 0;

		return 0;
	}
}
