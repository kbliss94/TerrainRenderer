#include "TerrainManager.h"

namespace TerrainRenderer
{
	TerrainManager::TerrainManager(): 
		mHeightMapFilenames(), mGridBottomRow(), mGridMiddleRow(), mGridTopRow(), mUpdated(false), mCurrentX(0), mCurrentZ(0)
	{
		mBottomRowOffsets = new vector<ChunkOffset>
		{
			{0, 0},
			{63, 0},
			{126, 0}
		};

		mMiddleRowOffsets = new vector<ChunkOffset>
		{
			{0, 63},
			{63, 63},
			{126, 63}
		};

		mTopRowOffsets = new vector<ChunkOffset>
		{
			{0, 126},
			{63, 126},
			{126, 126}
		};

		mGridBottomRow.resize(mNumGridRows);
		mGridMiddleRow.resize(mNumGridRows);
		mGridTopRow.resize(mNumGridRows);

		for (int i = 0; i < mNumGridRows; ++i)
		{
			mGridBottomRow[i] = new Terrain;
			mGridMiddleRow[i] = new Terrain;
			mGridTopRow[i] = new Terrain;
		}
	}

	TerrainManager::TerrainManager(const TerrainManager& rhs)
	{

	}

	TerrainManager& TerrainManager::operator=(const TerrainManager& rhs)
	{
		return *this;
	}

	TerrainManager::~TerrainManager()
	{

	}

	bool TerrainManager::Initialize(ID3D11Device* device, vector<char*>* heightMapFilenames)
	{
		mHeightMapFilenames = *heightMapFilenames;

		int filenameIndex = 0;

		if (mHeightMapFilenames.size() == mNumGridRows*mNumGridRows)
		{
			for (int i = 0; i < mNumGridRows; ++i)
			{
				(mGridBottomRow[i])->Initialize(device, mHeightMapFilenames[filenameIndex], (*mBottomRowOffsets)[i].x, (*mBottomRowOffsets)[i].z);
				++filenameIndex;
				(mGridMiddleRow[i])->Initialize(device, mHeightMapFilenames[filenameIndex], (*mMiddleRowOffsets)[i].x, (*mMiddleRowOffsets)[i].z);
				++filenameIndex;
				(mGridTopRow[i])->Initialize(device, mHeightMapFilenames[filenameIndex], (*mTopRowOffsets)[i].x, (*mTopRowOffsets)[i].z);
				++filenameIndex;
			}
		}

		return true;
	}

	void TerrainManager::Shutdown()
	{
		for (int i = 0; i < mNumGridRows; ++i)
		{
			if (mGridBottomRow[i])
			{
				(mGridBottomRow[i])->Shutdown();
				delete mGridBottomRow[i];
				mGridBottomRow[i] = 0;
			}

			if (mGridMiddleRow[i])
			{
				(mGridMiddleRow[i])->Shutdown();
				delete mGridMiddleRow[i];
				mGridMiddleRow[i] = 0;
			}

			if (mGridTopRow[i])
			{
				(mGridTopRow[i])->Shutdown();
				delete mGridTopRow[i];
				mGridTopRow[i] = 0;
			}
		}
	}

	void TerrainManager::Render(ID3D11DeviceContext* context, ColorShader* colorShader, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX projection)
	{
		for (int i = 0; i < mNumGridRows; ++i)
		{
			(mGridBottomRow[i])->Render(context);
			colorShader->Render(context, (mGridBottomRow[i])->GetIndexCount(), world, view, projection);

			(mGridMiddleRow[i])->Render(context);
			colorShader->Render(context, (mGridMiddleRow[i])->GetIndexCount(), world, view, projection);

			(mGridTopRow[i])->Render(context);
			colorShader->Render(context, (mGridTopRow[i])->GetIndexCount(), world, view, projection);
		}
	}

	void TerrainManager::GenerateChunks(Position* position)
	{
		//need to check to see if chunks need to be generated or not...divide them by something to get them to a base value that'll
		//be on the same scale for every chunk in order to check? idk
		///////for checking:
		//so if either x or z modded by 63 == 0, generate a chunk by offsetting x or z
		float flX, flY, flZ;
		position->GetPosition(flX, flY, flZ);

		int x = (int)flX;
		int z = (int)flZ;

		if (x % mChunkOffset == 0)
		{
			if (x < mCurrentX)
			{
				UpdateXPositionLeft();
				mCurrentX = x;
			}
			else if (x > mCurrentX)
			{
				UpdateXPositionRight();
				mCurrentX = x;
			}
			//so nothing should happen if x == mCurrentX
		}
		else if (z % mChunkOffset == 0)
		{
			if (z > mCurrentZ)
			{
				UpdateZPositionUp();
				mCurrentZ = z;
			}
			else if (z < mCurrentZ)
			{
				UpdateZPositionDown();
				mCurrentZ = z;
			}
			//so nothing should happen if z == mCurrentZ
		}
	}

	void TerrainManager::UpdateXPositionLeft()
	{

	}

	void TerrainManager::UpdateXPositionRight()
	{

	}

	void TerrainManager::UpdateZPositionUp()
	{
		for (int i = 0; i < mNumGridRows; ++i)
		{
			//update the row offset vectors
			(*mTopRowOffsets)[i].z = (*mTopRowOffsets)[i].z + mChunkOffset;
			(*mMiddleRowOffsets)[i].z = (*mMiddleRowOffsets)[i].z + mChunkOffset;
			(*mBottomRowOffsets)[i].z = (*mBottomRowOffsets)[i].z + mChunkOffset;

			//update chunk positions
			(mGridTopRow[i])->UpdatePosition((*mTopRowOffsets)[i].x, (*mTopRowOffsets)[i].z);
			(mGridMiddleRow[i])->UpdatePosition((*mMiddleRowOffsets)[i].x, (*mMiddleRowOffsets)[i].z);
			(mGridBottomRow[i])->UpdatePosition((*mBottomRowOffsets)[i].x, (*mBottomRowOffsets)[i].z);
		}
	}

	void TerrainManager::UpdateZPositionDown()
	{
		for (int i = 0; i < mNumGridRows; ++i)
		{
			//update the row offset vectors
			(*mTopRowOffsets)[i].z = (*mTopRowOffsets)[i].z - mChunkOffset;
			(*mMiddleRowOffsets)[i].z = (*mMiddleRowOffsets)[i].z - mChunkOffset;
			(*mBottomRowOffsets)[i].z = (*mBottomRowOffsets)[i].z - mChunkOffset;

			//update chunk positions
			(mGridTopRow[i])->UpdatePosition((*mTopRowOffsets)[i].x, (*mTopRowOffsets)[i].z);
			(mGridMiddleRow[i])->UpdatePosition((*mMiddleRowOffsets)[i].x, (*mMiddleRowOffsets)[i].z);
			(mGridBottomRow[i])->UpdatePosition((*mBottomRowOffsets)[i].x, (*mBottomRowOffsets)[i].z);
		}
	}
}