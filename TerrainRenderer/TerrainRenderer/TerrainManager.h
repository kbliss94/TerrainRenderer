#pragma once

#include "Terrain.h"
#include "ColorShader.h"
#include "Position.h"

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

		bool Initialize(ID3D11Device* device, vector<char*>* heightMapFilenames);
		void Shutdown();
		void Render(ID3D11DeviceContext* context, ColorShader* colorShader, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX projection);

		void GenerateChunks(Position* position);

	private:
		void UpdateXPositionLeft();
		void UpdateXPositionRight();
		void UpdateZPositionUp();
		void UpdateZPositionDown();

		void UpdateChunkPositions();
		void UpdateCurrentChunk(float x, float z);

		vector<char*> mHeightMapFilenames;
		vector<Terrain*> mGridBottomRow;
		vector<Terrain*> mGridMiddleRow;
		vector<Terrain*> mGridTopRow;

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

		//int mCurrentX;
		//int mCurrentZ;

		struct ChunkBorders
		{
			float minZ, maxZ, minX, maxX;
		};

		ChunkBorders mCurrentChunkBorders;
	};
}
