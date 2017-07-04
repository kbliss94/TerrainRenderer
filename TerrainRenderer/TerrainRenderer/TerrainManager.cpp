#include "Application.h"
#include "TerrainManager.h"

namespace TerrainRenderer
{
	TerrainManager::TerrainManager(): 
		mHeightMapFilenames(), mGridBottomRow(), mGridMiddleRow(), mGridTopRow(), mUpdated(false), mScalingFilenames()
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

	bool TerrainManager::Initialize(ID3D11Device* device, vector<char*>* heightMapFilenames, vector<char*>* scalingFilenames, char* largeScalingFilename)
	{
		mHeightMapFilenames = *heightMapFilenames;
		mScalingFilenames = *scalingFilenames;
		mLargeScalingMap = largeScalingFilename;

		PartitionScalingMap();

		//seam together the height maps here
		if (STITCHING_ENABLED)
		{
			ResolveVerticalSeam(mHeightMapFilenames[0], mHeightMapFilenames[3]);
			ResolveVerticalSeam(mHeightMapFilenames[1], mHeightMapFilenames[4]);
			ResolveVerticalSeam(mHeightMapFilenames[2], mHeightMapFilenames[5]);
			ResolveVerticalSeam(mHeightMapFilenames[5], mHeightMapFilenames[8]);
			ResolveVerticalSeam(mHeightMapFilenames[4], mHeightMapFilenames[7]);
			ResolveVerticalSeam(mHeightMapFilenames[3], mHeightMapFilenames[6]);

			ResolveHorizontalSeam(mHeightMapFilenames[1], mHeightMapFilenames[0]);
			ResolveHorizontalSeam(mHeightMapFilenames[4], mHeightMapFilenames[3]);
			ResolveHorizontalSeam(mHeightMapFilenames[7], mHeightMapFilenames[6]);
			ResolveHorizontalSeam(mHeightMapFilenames[2], mHeightMapFilenames[1]);
			ResolveHorizontalSeam(mHeightMapFilenames[5], mHeightMapFilenames[4]);
			ResolveHorizontalSeam(mHeightMapFilenames[8], mHeightMapFilenames[7]);
		}

		PartitionScalingMap();

		int filenameIndex = 0;

		if (mHeightMapFilenames.size() == mNumGridRows*mNumGridRows)
		{
			for (int i = 0; i < mNumGridRows; ++i)
			{
				(mGridBottomRow[i])->Initialize(device, mHeightMapFilenames[filenameIndex], mScalingFilenames[filenameIndex], (*mBottomRowOffsets)[i].x, (*mBottomRowOffsets)[i].z);
				++filenameIndex;
				(mGridMiddleRow[i])->Initialize(device, mHeightMapFilenames[filenameIndex], mScalingFilenames[filenameIndex], (*mMiddleRowOffsets)[i].x, (*mMiddleRowOffsets)[i].z);
				++filenameIndex;
				(mGridTopRow[i])->Initialize(device, mHeightMapFilenames[filenameIndex], mScalingFilenames[filenameIndex], (*mTopRowOffsets)[i].x, (*mTopRowOffsets)[i].z);
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

	void TerrainManager::ResolveVerticalSeam(const char* leftChunkFilename, const char* rightChunkFilename)
	{
		BMP leftChunk, rightChunk;

		leftChunk.ReadFromFile(leftChunkFilename);
		rightChunk.ReadFromFile(rightChunkFilename);

		//adding a weighted average border on the left side of the right chunk
		int leftChunkBorder = 63;
		float weightChange = (float)(1.0f / mBorderWidth);
		float leftWeight = 1.0f;
		float rightWeight = 0.0f;

		for (int i = 0; i < mBorderWidth; ++i)
		{
			for (int j = 0; j < leftChunk.TellWidth(); ++j)
			{
				if (i == 0)
				{
					rightChunk(i, j)->Red = (leftChunk(leftChunkBorder, j)->Red * leftWeight) + (rightChunk(i, j)->Red * rightWeight);
					rightChunk(i, j)->Green = (leftChunk(leftChunkBorder, j)->Green * leftWeight) + (rightChunk(i, j)->Green * rightWeight);
					rightChunk(i, j)->Blue = (leftChunk(leftChunkBorder, j)->Blue * leftWeight) + (rightChunk(i, j)->Blue * rightWeight);
				}
				else
				{
					rightChunk(i, j)->Red = (rightChunk(i - 1, j)->Red * leftWeight) + (rightChunk(i, j)->Red * rightWeight);
					rightChunk(i, j)->Green = (rightChunk(i - 1, j)->Green * leftWeight) + (rightChunk(i, j)->Green * rightWeight);
					rightChunk(i, j)->Blue = (rightChunk(i - 1, j)->Blue * leftWeight) + (rightChunk(i, j)->Blue * rightWeight);
				}
			}

			leftWeight -= weightChange;
			rightWeight += weightChange;
		}

		//adding a weighted average border on the right side of the left chunk
		int rightChunkBorder = 0;
		leftWeight = 0.0f;
		rightWeight = 1.0f;

		for (int i = 63; i > (63 - mBorderWidth); --i)
		{
			for (int j = 0; j < leftChunk.TellWidth(); ++j)
			{
				if (i == 63)
				{
					leftChunk(i, j)->Red = (rightChunk(rightChunkBorder, j)->Red * rightWeight) + (leftChunk(i, j)->Red * leftWeight);
					leftChunk(i, j)->Green = (rightChunk(rightChunkBorder, j)->Green * rightWeight) + (leftChunk(i, j)->Green * leftWeight);
					leftChunk(i, j)->Blue = (rightChunk(rightChunkBorder, j)->Blue * rightWeight) + (leftChunk(i, j)->Blue * leftWeight);
				}
				else
				{
					leftChunk(i, j)->Red = (leftChunk(i + 1, j)->Red * rightWeight) + (leftChunk(i, j)->Red * leftWeight);
					leftChunk(i, j)->Green = (leftChunk(i + 1, j)->Green * rightWeight) + (leftChunk(i, j)->Green * leftWeight);
					leftChunk(i, j)->Blue = (leftChunk(i + 1, j)->Blue * rightWeight) + (leftChunk(i, j)->Blue * leftWeight);
				}
			}

			leftWeight += weightChange;
			rightWeight -= weightChange;
		}

		rightChunk.WriteToFile(rightChunkFilename);
		leftChunk.WriteToFile(leftChunkFilename);
	}

	void TerrainManager::ResolveHorizontalSeam(const char* topChunkFilename, const char* bottomChunkFilename)
	{
		BMP topChunk, bottomChunk;

		topChunk.ReadFromFile(topChunkFilename);
		bottomChunk.ReadFromFile(bottomChunkFilename);

		//adding a weighted average border on the top side of the bottom chunk
		int topChunkBorder = 63;
		float weightChange = (float)(1.0f / mBorderWidth);
		float topWeight = 1.0f;
		float bottomWeight = 0.0f;

		for (int j = 0; j < mBorderWidth; ++j)
		{
			for (int i = 0; i < topChunk.TellWidth(); ++i)
			{
				if (j == 0)
				{
					bottomChunk(i, j)->Red = (topChunk(i, topChunkBorder)->Red * topWeight) + (bottomChunk(i, j)->Red * bottomWeight);
					bottomChunk(i, j)->Green = (topChunk(i, topChunkBorder)->Green * topWeight) + (bottomChunk(i, j)->Green * bottomWeight);
					bottomChunk(i, j)->Blue = (topChunk(i, topChunkBorder)->Blue * topWeight) + (bottomChunk(i, j)->Blue * bottomWeight);
				}
				else
				{
					bottomChunk(i, j)->Red = (bottomChunk(i, j - 1)->Red * topWeight) + (bottomChunk(i, j)->Red * bottomWeight);
					bottomChunk(i, j)->Green = (bottomChunk(i, j - 1)->Green * topWeight) + (bottomChunk(i, j)->Green * bottomWeight);
					bottomChunk(i, j)->Blue = (bottomChunk(i, j - 1)->Blue * topWeight) + (bottomChunk(i, j)->Blue * bottomWeight);
				}
			}

			topWeight -= weightChange;
			bottomWeight += weightChange;
		}

		//adding a weighted average border on the bottom side of the top chunk
		int bottomChunkBorder = 0;
		topWeight = 0.0f;
		bottomWeight = 1.0f;

		for (int j = 63; j > (63 - mBorderWidth); --j)
		{
			for (int i = 0; i < topChunk.TellWidth(); ++i)
			{
				if (j == 63)
				{
					topChunk(i, j)->Red = (bottomChunk(i, bottomChunkBorder)->Red * bottomWeight) + (topChunk(i, j)->Red * topWeight);
					topChunk(i, j)->Green = (bottomChunk(i, bottomChunkBorder)->Green * bottomWeight) + (topChunk(i, j)->Green * topWeight);
					topChunk(i, j)->Blue = (bottomChunk(i, bottomChunkBorder)->Blue * bottomWeight) + (topChunk(i, j)->Blue * topWeight);
				}
				else
				{
					topChunk(i, j)->Red = (topChunk(i, j + 1)->Red * bottomWeight) + (topChunk(i, j)->Red * topWeight);
					topChunk(i, j)->Green = (topChunk(i, j + 1)->Green * bottomWeight) + (topChunk(i, j)->Green * topWeight);
					topChunk(i, j)->Blue = (topChunk(i, j + 1)->Blue * bottomWeight) + (topChunk(i, j)->Blue * topWeight);
				}
			}

			topWeight += weightChange;
			bottomWeight -= weightChange;
		}

		bottomChunk.WriteToFile(bottomChunkFilename);
		topChunk.WriteToFile(topChunkFilename);
	}
	
	//Partitioning the large scale map into 9 smaller 64x64 chunks
	void TerrainManager::PartitionScalingMap()
	{
		BMP largeMap, smallMap;

		largeMap.ReadFromFile(mLargeScalingMap);
		smallMap.SetSize(64, 64);

		int smallJIndex = 0;
		int smallIIndex = 0;
		int maxJ = 192;
		int minI = 0;
		const int width = 64;

		for (int k = 0; k < 9; ++k)
		{
			for (int i = minI; i < (minI + width); ++i)
			{
				for (int j = (maxJ - width); j < maxJ; ++j)
				{
					smallMap(smallIIndex, smallJIndex)->Red = largeMap(i, j)->Red;
					smallMap(smallIIndex, smallJIndex)->Green = largeMap(i, j)->Green;
					smallMap(smallIIndex, smallJIndex)->Blue = largeMap(i, j)->Blue;
					smallMap(smallIIndex, smallJIndex)->Alpha = largeMap(i, j)->Alpha;

					++smallJIndex;
				}

				smallJIndex = 0;
				++smallIIndex;
			}

			smallMap.WriteToFile(mScalingFilenames[k]);
			maxJ -= width;
			smallIIndex = 0;

			if (k == 2 || k == 5 || k == 8)
			{
				smallIIndex = 0;
				smallJIndex = 0;
				maxJ = 192;
				minI += width;
			}
		}
	}
}