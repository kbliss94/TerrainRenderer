#pragma once

#include "Terrain.h"
//#include "ColorShader.h"
#include "TerrainShader.h"
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

		bool Initialize(ID3D11Device* device, vector<char*>* heightMapFilenames, vector<char*>* scalingFilenames, char* largeScalingFilename, 
			WCHAR* grassTextureFilename, WCHAR* slopeTextureFilename, WCHAR* rockTextureFilename);
		void Shutdown();
		void Render(ID3D11DeviceContext* context, TerrainShader* terrainShader, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX projection, D3DXVECTOR4 ambientColor,
			D3DXVECTOR4 diffuseColor, D3DXVECTOR3 lightDirection);

		//!Generates new chunks or deserializes chunks as the camera moves around
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

		void ResolveMoveLeftSeams();
		void ResolveMoveRightSeams();
		void ResolveMoveUpSeams();
		void ResolveMoveDownSeams();

		void ResolveVerticalSeam(const char* leftChunkFilename, const char* rightChunkFilename);
		void ResolveHorizontalSeam(const char* topChunkFilename, const char* bottomChunkFilename);

		void PartitionScalingMap();
		void GenerateNewHeightMap(char* filename);
		void GenerateNewScalingMaps();

		void ApplyScalingMap(const char* heightMapFilename, const char* scalingMapFilename);

	private:
		struct ChunkOffset
		{
			int x, z;
		};

		struct ChunkBorders
		{
			float minZ, maxZ, minX, maxX;
		};

		HeightMap mHeightMapGenerator;
		ChunkBorders mCurrentChunkBorders;

		string mSerializationFilename = "..//TerrainRenderer//data//chunkData//chunk";
		vector<ChunkOffset>* mStartingGridPositions;
		vector<char*> mHeightMapFilenames;
		vector<char*> mScalingFilenames;
		char* mLargeScalingMap;
		char* mScalingFilenameStart = "..//TerrainRenderer//data//scaling";

		vector<std::shared_ptr<Terrain>> mGridBottomRow;
		vector<std::shared_ptr<Terrain>> mGridMiddleRow;
		vector<std::shared_ptr<Terrain>> mGridTopRow;

		vector<std::shared_ptr<Terrain>> mGridLeftColumn;
		vector<std::shared_ptr<Terrain>> mGridMiddleColumn;
		vector<std::shared_ptr<Terrain>> mGridRightColumn;

		vector<ChunkOffset>* mBottomRowOffsets;
		vector<ChunkOffset>* mMiddleRowOffsets;
		vector<ChunkOffset>* mTopRowOffsets;

		const int mBorderWidth = 10; //20;//10;
		const int mHMHeight = 64;
		const int mHMWidth = 64;
		const int mSMHeight = 192;
		const int mSMWidth = 192;
		const int mNumGridRows = 3;
		const int mChunkOffset = 63;
	};
}
