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

		//create the system object
		system = new System;

		if (!system)
		{
			return 0;
		}

		//initialize & run the system object
		result = system->Initialize();

		if (result)
		{
			system->Run();
		}

		//shut down & release the system object
		system->Shutdown();
		delete system;
		system = 0;

		return 0;
	}
}
