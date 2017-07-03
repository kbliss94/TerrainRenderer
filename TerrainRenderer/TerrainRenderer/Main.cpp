#include "System.h"
#include <noise.h>
#include <random>
#include "noiseutils.h"
#include "HeightMap.h"

#include "EasyBMP.h"

using namespace std;
using namespace TerrainRenderer;

//!This terrain renderer will procedurally texture and render a large-scale terrain.
/*!*/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	//testing out modifying the bmp stuff using EasyBMP
	BMP testChunk;
	const char* testChunkFilename = "..//TerrainRenderer//data//HM0.bmp";
	const char* testOutputFilename = "..//TerrainRenderer//data//EasyBMPOutput.bmp";

	testChunk.ReadFromFile(testChunkFilename);

	//for (int j = 0; j < 2; ++j)
	//{
	//	for (int i = 0; i < testChunk.TellWidth(); ++i)
	//	{
	//		testChunk(j, i)->Red = 50;
	//		testChunk(j, i)->Green = 0;
	//		testChunk(j, i)->Blue = 50;
	//	}
	//}

	for (int i = 0; i < testChunk.TellWidth(); ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			testChunk(i, j)->Red = 50;
			testChunk(i, j)->Green = 0;
			testChunk(i, j)->Blue = 50;
		}
	}

	testChunk.WriteToFile(testOutputFilename);


	//Rendering the terrain mesh using the height map
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
	system = nullptr;

	return 0;
}
