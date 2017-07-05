#pragma once

#include "Terrain.h"
#include "ColorShader.h"
#include "Position.h"
#include "EasyBMP.h"

//below is used for cereal
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <fstream>

#include <vector>

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

		void Serialize(std::shared_ptr<Terrain> terrainChunk);
		bool Deserialize(int gridX, int gridY, std::shared_ptr<Terrain>& terrainChunk);

	private:
		void UpdateXPositionLeft();
		void UpdateXPositionRight();
		void UpdateZPositionUp();
		void UpdateZPositionDown();

		void UpdateChunkPositions();
		void UpdateCurrentChunk(float x, float z);

		void ResolveVerticalSeam(const char* leftChunkFilename, const char* rightChunkFilename);
		void ResolveHorizontalSeam(const char* topChunkFilename, const char* bottomChunkFilename);
		void PartitionScalingMap();

		vector<char*> mHeightMapFilenames;
		vector<char*> mScalingFilenames;
		char* mLargeScalingMap;
		vector<std::shared_ptr<Terrain>> mGridBottomRow;
		vector<std::shared_ptr<Terrain>> mGridMiddleRow;
		vector<std::shared_ptr<Terrain>> mGridTopRow;

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
		const int mBorderWidth = 10;

		string mSerializationFilename = "..//TerrainRenderer//data//chunkData//chunk";
	};
}
