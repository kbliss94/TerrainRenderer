#pragma once

#include "Terrain.h"
#include "TerrainShader.h"
#include "Position.h"
#include "EasyBMP.h"
#include "HeightMap.h"
#include "QuadTree.h"

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
			D3DXVECTOR4 diffuseColor, D3DXVECTOR3 lightDirection, Frustum* frustum, Position* position, D3DXVECTOR3 fogColor);

		//!Updates reserved data if necessary
		void Update();

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

		int GetTriDrawCounts();
		int GetTriTotalCounts();

	private:
		void UpdateXPositionLeft();
		void UpdateXPositionRight();
		void UpdateZPositionUp();
		void UpdateZPositionDown();

		void UpdateChunkPositions();
		void UpdateCurrentChunk(float x, float z);
		void UpdateQuadTrees();

		void ResolveMoveLeftSeams();
		void ResolveMoveRightSeams();
		void ResolveMoveUpSeams();
		void ResolveMoveDownSeams();

		//!Stitches chunks together while they're in .bmp form
		void ResolveVerticalSeam(const char* leftChunkFilename, const char* rightChunkFilename);
		void ResolveHorizontalSeam(const char* topChunkFilename, const char* bottomChunkFilename);

		//!Stitches chunks together after they've been loaded in & the data is in a vector
		void StitchVerticalSeam(vector<Terrain::HeightMapData>& leftChunk, vector<Terrain::HeightMapData>& rightChunk);
		void StitchHorizontalSeam(vector<Terrain::HeightMapData>& topChunk, vector<Terrain::HeightMapData>& bottomChunk);

		void PartitionScalingMap();
		void GenerateNewHeightMap(char* filename);
		void GenerateNewScalingMaps();

		void ApplyScalingMap(const char* heightMapFilename, const char* scalingMapFilename);

		//!The oldMapFilename will contain the map from the newMapFilename
		void SetNewHeightMap(const char* oldMapFilename, const char* newMapFilename);

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
		vector<ChunkOffset>* mStartingReservePositions;
		vector<char*> mHeightMapFilenames;
		vector<char*> mScalingFilenames;
		char* mLargeScalingMap;
		char* mScalingFilenameStart = "..//TerrainRenderer//data//scaling";
		char* mReserveMapFilename = "..//TerrainRenderer//data//reserveMap";

		vector<std::shared_ptr<Terrain>> mGridBottomRow;
		vector<std::shared_ptr<Terrain>> mGridMiddleRow;
		vector<std::shared_ptr<Terrain>> mGridTopRow;

		vector<std::shared_ptr<Terrain>> mGridLeftColumn;
		vector<std::shared_ptr<Terrain>> mGridMiddleColumn;
		vector<std::shared_ptr<Terrain>> mGridRightColumn;

		//pre-loaded data ready to be rendered
		vector<std::shared_ptr<Terrain>> mTopRowReserve;
		vector<std::shared_ptr<Terrain>> mRightColumnReserve;
		vector<std::shared_ptr<Terrain>> mBottomRowReserve;
		vector<std::shared_ptr<Terrain>> mLeftColumnReserve;
		//

		vector<std::shared_ptr<QuadTree>> mQuadBottomRow;
		vector<std::shared_ptr<QuadTree>> mQuadMiddleRow;
		vector<std::shared_ptr<QuadTree>> mQuadTopRow;

		vector<ChunkOffset>* mBottomRowOffsets;
		vector<ChunkOffset>* mMiddleRowOffsets;
		vector<ChunkOffset>* mTopRowOffsets;

		bool mUpdateTopReserveRow;
		bool mUpdateRightReserveColumn;
		bool mUpdateBottomReserveRow;
		bool mUpdateLeftReserveColumn;

		const int mBorderWidth = 20; //20;//10;
		const int mNumGridRows = 3;
		const int mSMHeight = HM_HEIGHT * mNumGridRows;
		const int mSMWidth = HM_WIDTH * mNumGridRows;
		const int mChunkOffset = HM_HEIGHT - 1;
	};
}
