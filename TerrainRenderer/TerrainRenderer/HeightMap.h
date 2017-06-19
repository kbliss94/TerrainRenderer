#pragma once
#include <noise.h>
#include "noiseutils.h"

using namespace std;

namespace TerrainRenderer
{
	class HeightMap
	{
	public:
		HeightMap();
		
		void Generate(const string& filename, int height, int width);

		string DestinationFilename();
		void SetDestinationFilename(const string& filename);

		double LowerXBound();
		void SetLowerXBound(double bound);

		double LowerZBound();
		void SetLowerZBound(double bound);

		double UpperXBound();
		void SetUpperXBound(double bound);

		double UpperZBound();
		void SetUpperZBound(double bound);

		int Seed();
		void SetSeed(int seed);

	private:
		string mFilename;
		double mLowerXBound;
		double mLowerZBound;
		double mUpperXBound;
		double mUpperZBound;
		int mHeight;
		int mWidth;
		int mSeed;
	};
}
