#include "HeightMap.h"

namespace TerrainRenderer
{
	HeightMap::HeightMap():
		mFilename(""), mLowerXBound(2.0), mLowerZBound(1.0), mUpperXBound(6.0), mUpperZBound(5.0), mSeed(0)
	{

	}

	string HeightMap::DestinationFilename()
	{
		return mFilename;
	}

	void HeightMap::SetDestinationFilename(const string& filename)
	{
		mFilename = filename;
	}

	double HeightMap::LowerXBound()
	{
		return mLowerXBound;
	}

	void HeightMap::SetLowerXBound(double bound)
	{
		mLowerXBound = bound;
	}

	double HeightMap::LowerZBound()
	{
		return mLowerZBound;
	}

	void HeightMap::SetLowerZBound(double bound)
	{
		mLowerZBound = bound;
	}

	double HeightMap::UpperXBound()
	{
		return mUpperXBound;
	}

	void HeightMap::SetUpperXBound(double bound)
	{
		mUpperXBound = bound;
	}

	double HeightMap::UpperZBound()
	{
		return mUpperZBound;
	}

	void HeightMap::SetUpperZBound(double bound)
	{
		mUpperZBound = bound;
	}

	int HeightMap::Seed()
	{
		return mSeed;
	}

	void HeightMap::SetSeed(int seed)
	{
		mSeed = seed;
	}

	bool HeightMap::IsScaleMap()
	{
		return mIsScaleMap;
	}

	void HeightMap::SetIsScaleMap(bool isScale)
	{
		mIsScaleMap = isScale;
	}

	void HeightMap::Generate(const string& filename, int height, int width)
	{
		module::Perlin perlinModule;
		utils::NoiseMap heightMap;
		utils::NoiseMapBuilderPlane heightMapBuilder;
		
		mFilename = filename;
		mHeight = height;
		mWidth = width;
		perlinModule.SetSeed(mSeed);

		perlinModule.SetFrequency(perlinModule.GetFrequency() - .4);

		//testing scaling
		//module::Add scaling;
		
		if (mIsScaleMap)
		{
			perlinModule.SetFrequency(perlinModule.GetFrequency() - .3);
		}



		//building the height map
		heightMapBuilder.SetSourceModule(perlinModule);
		heightMapBuilder.SetDestNoiseMap(heightMap);
		heightMapBuilder.SetDestSize(mHeight, mWidth);
		heightMapBuilder.SetBounds(mLowerXBound, mUpperXBound, mLowerZBound, mUpperZBound);
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
		writer.SetDestFilename(mFilename);
		writer.WriteDestFile();
	}
}