#include <noise.h>
#include "noiseutils.h"

using namespace std;
using namespace noise;

int main(int argc, char** argv)
{
	const double LowerXBound = 2.0;
	const double LowerZBound = 1.0;
	const double UpperXBound = 6.0;
	const double UpperZBound = 5.0;
	const string HeightMapFilename = "heightMap.bmp";		//default values
	//const string HeightMapFilename = "heightMapHigherFreq.bmp";	//added .5 to default freq
	//const string HeightMapFilename = "heightMapLowerFreq.bmp";	//subtracted .5 from default freq
	//const string HeightMapFilename = "heightMapHigherLac.bmp";		//added .5 to default lacunarity
	//const string HeightMapFilename = "heightMapLowerLac.bmp";		//subtracted .5 from default lacunarity
	//const string HeightMapFilename = "heightMapHigherOctCount.bmp";		//added 5 to default octave count
	//const string HeightMapFilename = "heightMapLowerOctCount.bmp";		//subtracted 5 from default octave count
	//const string HeightMapFilename = "heightMapHigherPers.bmp";		//added .5 to default persistence
	//const string HeightMapFilename = "heightMapLowerPers.bmp";		//subtracted .5 from default persistence

	module::Perlin perlinModule;
	utils::NoiseMap heightMap;
	utils::NoiseMapBuilderPlane heightMapBuilder;

	//building the height map
	heightMapBuilder.SetSourceModule(perlinModule);
	heightMapBuilder.SetDestNoiseMap(heightMap);
	heightMapBuilder.SetDestSize(256, 256);
	heightMapBuilder.SetBounds(LowerXBound, UpperXBound, LowerZBound, UpperZBound);
	heightMapBuilder.Build();

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

	return 0;
}