#pragma once

#include "Terrain.h"
#include "ColorShader.h"
#include "Position.h"
#include "EasyBMP.h"
#include "HeightMap.h"

//below is used for cereal
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <fstream>

#include <vector>
#include <random>
#include <chrono>

using namespace std;

namespace TerrainRenderer
{
	class TerrainManager
	{
	public:
		//!Constructor
		TerrainManager();

		//!Copy constructor
		TerrainManager(const TerrainManager& rhs);

		//!Assignment operator
		TerrainManager& operator=(const TerrainManager& rhs);

		//!Destructor
		~TerrainManager();

		bool Initialize(ID3D11Device* device, vector<char*>* heightMapFilenames, vector<char*>* scalingFilenames, char* largeScalingFilename);
		void Shutdown();
		void Render(ID3D11DeviceContext* context, ColorShader* colorShader, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX projection);

		void GenerateChunks(Position* position);

		//!Serializes the terrain chunk param 
		/*
		Doesn't check to make sure that the data hasn't already been serialized
		*/
		void Serialize(std::shared_ptr<Terrain> terrainChunk);

		//!Pass in the position of the grid that you would like to get the height map for & the terrain chunk that should be populated
		/*
		\return true if the data was deserialized, false if the data has not been serialized yet
		*/
		bool Deserialize(int gridX, int gridY, std::shared_ptr<Terrain>& terrainChunk);

	private:
		void UpdateXPositionLeft();
		void UpdateXPositionRight();
		void UpdateZPositionUp();
		void UpdateZPositionDown();

		void UpdateChunkPositions();
		void UpdateCurrentChunk(float x, float z);

		void ResolveVerticalSeams();
		void ResolveHorizontalSeams();
		void ResolveVerticalSeam(const char* leftChunkFilename, const char* rightChunkFilename);
		void ResolveHorizontalSeam(const char* topChunkFilename, const char* bottomChunkFilename);
		void PartitionScalingMap();

		void GenerateNewHeightMap(char* filename);

		vector<char*> mHeightMapFilenames;
		vector<char*> mScalingFilenames;
		char* mLargeScalingMap;
		vector<std::shared_ptr<Terrain>> mGridBottomRow;
		vector<std::shared_ptr<Terrain>> mGridMiddleRow;
		vector<std::shared_ptr<Terrain>> mGridTopRow;

		vector<std::shared_ptr<Terrain>> mGridLeftColumn;
		vector<std::shared_ptr<Terrain>> mGridMiddleColumn;
		vector<std::shared_ptr<Terrain>> mGridRightColumn;

		struct ChunkOffset
		{
			int x, z;
		};

		vector<ChunkOffset>* mBottomRowOffsets;
		vector<ChunkOffset>* mMiddleRowOffsets;
		vector<ChunkOffset>* mTopRowOffsets;

		const int mNumGridRows = 3;
		const int mChunkOffset = 63;

		bool mUpdated;

		struct ChunkBorders
		{
			float minZ, maxZ, minX, maxX;
		};

		ChunkBorders mCurrentChunkBorders;
		const int mBorderWidth = 30;

		string mSerializationFilename = "..//TerrainRenderer//data//chunkData//chunk";
		vector<ChunkOffset>* mStartingGridPositions;
		HeightMap mHeightMapGenerator;
		//default_random_engine mRandomSeedGenerator;
		//uniform_int_distribution<int> mDistribution;
		const int mHMHeight = 64;
		const int mHMWidth = 64;
		const int mLeftColumnOffset = 0;
		const int mMiddleColumnOffset = 1;
		const int mRightColumnOffset = 2;
	};
}
