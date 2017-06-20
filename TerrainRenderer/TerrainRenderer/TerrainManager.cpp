#include "TerrainManager.h"

namespace TerrainRenderer
{
	TerrainManager::TerrainManager(): 
		mHeightMapFilenames(), mGridBottomRow(), mGridMiddleRow(), mGridTopRow(), mUpdated(false)//, mCurrentX(0), mCurrentZ(0)
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

		//sets the middle chunk as the current chunk on startup
		mCurrentChunkBorders = { (float)(*mMiddleRowOffsets)[1].z, (float)((*mMiddleRowOffsets)[1].z + mChunkOffset), (float)(*mMiddleRowOffsets)[1].x, (float)((*mMiddleRowOffsets)[1].x + mChunkOffset) };
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
		float xPos, yPos, zPos;
		
		position->GetPosition(xPos, yPos, zPos);

		if (xPos < mCurrentChunkBorders.minX)
		{
			//move left
			UpdateXPositionLeft();
			UpdateChunkPositions();
			//update current chunk
			UpdateCurrentChunk(xPos, zPos);
		}
		else if (xPos > mCurrentChunkBorders.maxX)
		{
			//move right
			UpdateXPositionRight();
			UpdateChunkPositions();
			//update current chunk
			UpdateCurrentChunk(xPos, zPos);
		}
		else if (zPos > mCurrentChunkBorders.maxZ)
		{
			//move up
			UpdateZPositionUp();
			UpdateChunkPositions();
			//update current chunk
			UpdateCurrentChunk(xPos, zPos);
		}
		else if (zPos < mCurrentChunkBorders.minZ)
		{
			//move down
			UpdateZPositionDown();
			UpdateChunkPositions();
			//update current chunk
			UpdateCurrentChunk(xPos, zPos);
		}
	}

	void TerrainManager::UpdateXPositionLeft()
	{
		for (int i = 0; i < mNumGridRows; ++i)
		{
			//update the row offset vectors
			(*mTopRowOffsets)[i].x = (*mTopRowOffsets)[i].x - mChunkOffset;
			(*mMiddleRowOffsets)[i].x = (*mMiddleRowOffsets)[i].x - mChunkOffset;
			(*mBottomRowOffsets)[i].x = (*mBottomRowOffsets)[i].x - mChunkOffset;
		}
	}

	void TerrainManager::UpdateXPositionRight()
	{
		for (int i = 0; i < mNumGridRows; ++i)
		{
			//update the row offset vectors
			(*mTopRowOffsets)[i].x = (*mTopRowOffsets)[i].x + mChunkOffset;
			(*mMiddleRowOffsets)[i].x = (*mMiddleRowOffsets)[i].x + mChunkOffset;
			(*mBottomRowOffsets)[i].x = (*mBottomRowOffsets)[i].x + mChunkOffset;
		}
	}

	void TerrainManager::UpdateZPositionUp()
	{
		for (int i = 0; i < mNumGridRows; ++i)
		{
			//update the row offset vectors
			(*mTopRowOffsets)[i].z = (*mTopRowOffsets)[i].z + mChunkOffset;
			(*mMiddleRowOffsets)[i].z = (*mMiddleRowOffsets)[i].z + mChunkOffset;
			(*mBottomRowOffsets)[i].z = (*mBottomRowOffsets)[i].z + mChunkOffset;
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
		}
	}

	void TerrainManager::UpdateChunkPositions()
	{
		for (int i = 0; i < mNumGridRows; ++i)
		{
			//update chunk positions
			(mGridTopRow[i])->UpdatePosition((*mTopRowOffsets)[i].x, (*mTopRowOffsets)[i].z);
			(mGridMiddleRow[i])->UpdatePosition((*mMiddleRowOffsets)[i].x, (*mMiddleRowOffsets)[i].z);
			(mGridBottomRow[i])->UpdatePosition((*mBottomRowOffsets)[i].x, (*mBottomRowOffsets)[i].z);
		}
	}

	void TerrainManager::UpdateCurrentChunk(float x, float z)
	{
		for (int i = 0; i < mNumGridRows; ++i)
		{
			if ((*mTopRowOffsets)[i].x < x && x < ((*mTopRowOffsets)[i].x + mChunkOffset)
				&& (*mTopRowOffsets)[i].z < z && z < ((*mTopRowOffsets)[i].z + mChunkOffset))
			{
				//if the camera position is within this chunk, then it is the current chunk

				mCurrentChunkBorders = { (float)(*mTopRowOffsets)[i].z, (float)((*mTopRowOffsets)[i].z + mChunkOffset), (float)(*mTopRowOffsets)[i].x, (float)((*mTopRowOffsets)[i].x + mChunkOffset) };
			}

			if ((*mMiddleRowOffsets)[i].x < x && x < ((*mMiddleRowOffsets)[i].x + mChunkOffset)
				&& (*mMiddleRowOffsets)[i].z < z && z < ((*mMiddleRowOffsets)[i].z + mChunkOffset))
			{
				//if the camera position is within this chunk, then it is the current chunk

				mCurrentChunkBorders = { (float)(*mMiddleRowOffsets)[i].z, (float)((*mMiddleRowOffsets)[i].z + mChunkOffset), (float)(*mMiddleRowOffsets)[i].x, (float)((*mMiddleRowOffsets)[i].x + mChunkOffset) };
			}

			if ((*mBottomRowOffsets)[i].x < x && x < ((*mBottomRowOffsets)[i].x + mChunkOffset)
				&& (*mBottomRowOffsets)[i].z < z && z < ((*mBottomRowOffsets)[i].z + mChunkOffset))
			{
				//if the camera position is within this chunk, then it is the current chunk

				mCurrentChunkBorders = { (float)(*mBottomRowOffsets)[i].z, (float)((*mBottomRowOffsets)[i].z + mChunkOffset), (float)(*mBottomRowOffsets)[i].x, (float)((*mBottomRowOffsets)[i].x + mChunkOffset) };
			}
		}
	}
}