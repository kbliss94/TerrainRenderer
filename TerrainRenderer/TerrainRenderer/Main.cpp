#include "System.h"
#include <noise.h>
#include "noiseutils.h"

using namespace std;
using namespace TerrainRenderer;

//!This terrain renderer will procedurally texture and render a large-scale terrain.
/*!*/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	//Generating the height map using Perlin noise
	const double LowerXBound = 2.0;
	const double LowerZBound = 1.0;
	const double UpperXBound = 6.0;
	const double UpperZBound = 5.0;
	const string HeightMapFilename = "..//TerrainRenderer//data//newHeightMap.bmp";

	module::Perlin perlinModule;
	utils::NoiseMap heightMap;
	utils::NoiseMapBuilderPlane heightMapBuilder;

	perlinModule.SetLacunarity(perlinModule.GetLacunarity() - .3);



	//building the height map
	heightMapBuilder.SetSourceModule(perlinModule);
	heightMapBuilder.SetDestNoiseMap(heightMap);
	heightMapBuilder.SetDestSize(64, 64);
	heightMapBuilder.SetBounds(LowerXBound, UpperXBound, LowerZBound, UpperZBound);
	heightMapBuilder.Build();

	int pSeed;
	pSeed = perlinModule.GetSeed();

	//setting up height map renderer
	utils::RendererImage renderer;
	utils::Image image;
	renderer.SetSourceNoiseMap(heightMap);
	renderer.SetDestImage(image);
	renderer.Render();

	//writing height map image to an output file
	utils::WriterBMP writer;
	writer.SetSourceImage(image);
	writer.SetDestFilename(HeightMapFilename);
	writer.WriteDestFile();


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
