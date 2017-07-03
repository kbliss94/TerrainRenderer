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


		//testing out weighted average borders
		int test = 63;

		float leftWeight = 1.0f;
		float rightWeight = 0.0f;

		//used to be j < 11
		//then was j < 21
		for (int j = 0; j < 30; ++j)
		{
			for (int i = 0; i < leftChunk.TellWidth(); ++i)
			{
				//weighting average
				if (j == 0)
				{
					//rightChunk(j, i)->Red = (leftChunk(test, i)->Red * leftWeight) + (rightChunk(j, i)->Red * rightWeight);
					//rightChunk(j, i)->Green = (leftChunk(test, i)->Green * leftWeight) + (rightChunk(j, i)->Green * rightWeight);
					//rightChunk(j, i)->Blue = (leftChunk(test, i)->Blue * leftWeight) + (rightChunk(j, i)->Blue * rightWeight);

					//trying to not use weighted average
					//rightChunk(j, i)->Red = ((leftChunk(test, i)->Red) + (rightChunk(j, i)->Red)) / 2;
					//rightChunk(j, i)->Green = ((leftChunk(test, i)->Green) + (rightChunk(j, i)->Green)) / 2;
					//rightChunk(j, i)->Blue = ((leftChunk(test, i)->Blue) + (rightChunk(j, i)->Blue)) / 2;

					rightChunk(j, i)->Red = (leftChunk(test, i)->Red);
					rightChunk(j, i)->Green = (leftChunk(test, i)->Green);
					rightChunk(j, i)->Blue = (leftChunk(test, i)->Blue);

				}
				else
				{
					//rightChunk(j, i)->Red = (rightChunk(j - 1, i)->Red * leftWeight) + (rightChunk(j, i)->Red * rightWeight);
					//rightChunk(j, i)->Green = (rightChunk(j - 1, i)->Green * leftWeight) + (rightChunk(j, i)->Green * rightWeight);
					//rightChunk(j, i)->Blue = (rightChunk(j - 1, i)->Blue * leftWeight) + (rightChunk(j, i)->Blue * rightWeight);

					//trying to not use weighted average
					rightChunk(j, i)->Red = ((rightChunk(j - 1, i)->Red) + (rightChunk(j, i)->Red)) / 2;
					rightChunk(j, i)->Green = ((rightChunk(j - 1, i)->Green) + (rightChunk(j, i)->Green)) / 2;
					rightChunk(j, i)->Blue = ((rightChunk(j - 1, i)->Blue) + (rightChunk(j, i)->Blue)) / 2;
				}

				//incrementing weight values & x values of left-hand border
				leftWeight -= -.1f;		//0.025f;	//0.05f;	//0.1f;
				rightWeight += 0.1f;		//0.025f;			//0.05f;			//0.1f;
			}
		}


		//below is what i'm currently using
		//int test = 61;

		//for (int j = 0; j < 3; ++j)
		//{
		//	for (int i = 0; i < leftChunk.TellWidth(); ++i)
		//	{
		//		//averaging the values for the border/seam
		//		if (j == 0)
		//		{
		//			rightChunk(j, i)->Red = leftChunk(test, i)->Red;
		//			rightChunk(j, i)->Green = leftChunk(test, i)->Green;
		//			rightChunk(j, i)->Blue = leftChunk(test, i)->Blue;
		//		}

		//		if (j == 1)
		//		{
		//			rightChunk(j, i)->Red = (rightChunk(j, i)->Red + leftChunk(test, i)->Red) / 2;
		//			rightChunk(j, i)->Green = (rightChunk(j, i)->Green + leftChunk(test, i)->Green) / 2;
		//			rightChunk(j, i)->Blue = (rightChunk(j, i)->Blue + leftChunk(test, i)->Blue) / 2;

		//			leftChunk(test, i)->Red = (rightChunk(j, i)->Red + leftChunk(test, i)->Red) / 2;
		//			leftChunk(test, i)->Green = (rightChunk(j, i)->Green + leftChunk(test, i)->Green) / 2;
		//			leftChunk(test, i)->Blue = (rightChunk(j, i)->Blue + leftChunk(test, i)->Blue) / 2;
		//		}

		//		if (j == 2)
		//		{
		//			rightChunk(j, i)->Red = (rightChunk(j, i)->Red + rightChunk(1, i)->Red) / 2;
		//			rightChunk(j, i)->Green = (rightChunk(j, i)->Green + rightChunk(1, i)->Green) / 2;
		//			rightChunk(j, i)->Blue = (rightChunk(j, i)->Blue + rightChunk(1, i)->Blue) / 2;

		//			leftChunk(test, i)->Red = (leftChunk(test, i)->Red + leftChunk(test - 1, i)->Red) / 2;
		//			leftChunk(test, i)->Green = (leftChunk(test, i)->Green + leftChunk(test - 1, i)->Green) / 2;
		//			leftChunk(test, i)->Blue = (leftChunk(test, i)->Blue + leftChunk(test - 1, i)->Blue) / 2;
		//		}
		//	}

		//	++test;
		//}

		rightChunk.WriteToFile(rightChunkFilename);
	}

	void TerrainManager::ResolveHorizontalSeam(const char* topChunkFilename, const char* bottomChunkFilename)
	{
		BMP topChunk, bottomChunk;

		topChunk.ReadFromFile(topChunkFilename);
		bottomChunk.ReadFromFile(bottomChunkFilename);

		////testing out weighted average borders
		//int test = 63;

		////unsigned char LRed, LGreen, LBlue, RRed, RGreen, RBlue;

		////used to be j < 11
		////then was j < 21
		//for (int j = 0; j < 30; ++j)
		//{
		//	for (int i = 0; i < topChunk.TellWidth(); ++i)
		//	{
		//		//LRed = 


		//		//weighting average
		//		if (j == 0)
		//		{
		//			bottomChunk(j, i)->Red = (topChunk(test, i)->Red);
		//			bottomChunk(j, i)->Green = (topChunk(test, i)->Green);
		//			bottomChunk(j, i)->Blue = (topChunk(test, i)->Blue);

		//		}
		//		else
		//		{
		//			bottomChunk(j, i)->Red = ((bottomChunk(j - 1, i)->Red) + (bottomChunk(j, i)->Red)) / 2;
		//			bottomChunk(j, i)->Green = ((bottomChunk(j - 1, i)->Green) + (bottomChunk(j, i)->Green)) / 2;
		//			bottomChunk(j, i)->Blue = ((bottomChunk(j - 1, i)->Blue) + (bottomChunk(j, i)->Blue)) / 2;
		//		}
		//	}
		//}


		//previous stuff
		int test = 61;

		for (int i = 0; i < topChunk.TellWidth(); ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				//averaging the values for the border/seam
				if (j == 0)
				{
					bottomChunk(i, j)->Red = topChunk(i, test)->Red;
					bottomChunk(i, j)->Green = topChunk(i, test)->Green;
					bottomChunk(i, j)->Blue = topChunk(i, test)->Blue;
				}

				if (j == 1)
				{
					bottomChunk(i, j)->Red = (bottomChunk(j, i)->Red + topChunk(test, i)->Red) / 2;
					bottomChunk(i, j)->Green = (bottomChunk(j, i)->Green + topChunk(test, i)->Green) / 2;
					bottomChunk(i, j)->Blue = (bottomChunk(j, i)->Blue + topChunk(test, i)->Blue) / 2;
				}

				if (j == 2)
				{
					bottomChunk(i, j)->Red = (bottomChunk(j, i)->Red + bottomChunk(1, i)->Red) / 2;
					bottomChunk(i, j)->Green = (bottomChunk(j, i)->Green + bottomChunk(1, i)->Green) / 2;
					bottomChunk(i, j)->Blue = (bottomChunk(j, i)->Blue + bottomChunk(1, i)->Blue) / 2;
				}
			}

			++test;
		}

		bottomChunk.WriteToFile(bottomChunkFilename);
	}
	
	//partitioning the large scale map into 9 smaller 64x64 chunks
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