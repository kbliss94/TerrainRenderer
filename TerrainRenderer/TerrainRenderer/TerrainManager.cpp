#include "Application.h"
#include "TerrainManager.h"

using namespace std;

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

		mStartingGridPositions = new vector<ChunkOffset>
		{
			{-1, -1},
			{-1, 0},
			{-1, 1},
			{0, -1},
			{0, 0},
			{0, 1},
			{1, -1},
			{1, 0},
			{1, 1}
		};

		mGridBottomRow.resize(mNumGridRows);
		mGridMiddleRow.resize(mNumGridRows);
		mGridTopRow.resize(mNumGridRows);

		for (int i = 0; i < mNumGridRows; ++i)
		{
			mGridBottomRow[i] = make_shared<Terrain>();
			mGridMiddleRow[i] = make_shared<Terrain>();
			mGridTopRow[i] = make_shared<Terrain>();
		}

		//sets the middle chunk as the current chunk on startup
		mCurrentChunkBorders = { (float)(*mMiddleRowOffsets)[1].z, (float)((*mMiddleRowOffsets)[1].z + mChunkOffset), (float)(*mMiddleRowOffsets)[1].x, (float)((*mMiddleRowOffsets)[1].x + mChunkOffset) };		
	
		//setting up the left column vector
		mGridLeftColumn.push_back(mGridTopRow[0]);
		mGridLeftColumn.push_back(mGridMiddleRow[0]);
		mGridLeftColumn.push_back(mGridBottomRow[0]);

		//setting up the middle column vector
		mGridMiddleColumn.push_back(mGridTopRow[1]);
		mGridMiddleColumn.push_back(mGridMiddleRow[1]);
		mGridMiddleColumn.push_back(mGridBottomRow[1]);

		//setting up the right column vector
		mGridRightColumn.push_back(mGridTopRow[2]);
		mGridRightColumn.push_back(mGridMiddleRow[2]);
		mGridRightColumn.push_back(mGridBottomRow[2]);
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
				mGridBottomRow[i]->SetGridPosition((*mStartingGridPositions)[filenameIndex].x, (*mStartingGridPositions)[filenameIndex].z);
				++filenameIndex;
				(mGridMiddleRow[i])->Initialize(device, mHeightMapFilenames[filenameIndex], mScalingFilenames[filenameIndex], (*mMiddleRowOffsets)[i].x, (*mMiddleRowOffsets)[i].z);
				mGridMiddleRow[i]->SetGridPosition((*mStartingGridPositions)[filenameIndex].x, (*mStartingGridPositions)[filenameIndex].z);
				++filenameIndex;
				(mGridTopRow[i])->Initialize(device, mHeightMapFilenames[filenameIndex], mScalingFilenames[filenameIndex], (*mTopRowOffsets)[i].x, (*mTopRowOffsets)[i].z);
				mGridTopRow[i]->SetGridPosition((*mStartingGridPositions)[filenameIndex].x, (*mStartingGridPositions)[filenameIndex].z);
				++filenameIndex;
			}
		}

		return true;
	}

	void TerrainManager::Shutdown()
	{
		//should delete all serialized files?
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

			//ResolveVerticalSeams();
			//ResolveHorizontalSeams();

			UpdateChunkPositions();


			//update current chunk
			UpdateCurrentChunk(xPos, zPos);
		}
		else if (xPos > mCurrentChunkBorders.maxX)
		{
			//move right
			UpdateXPositionRight();

			//ResolveVerticalSeams();
			//ResolveHorizontalSeams();

			UpdateChunkPositions();

			//update current chunk
			UpdateCurrentChunk(xPos, zPos);
		}
		else if (zPos > mCurrentChunkBorders.maxZ)
		{
			//move up
			UpdateZPositionUp();

			ResolveVerticalSeams();
			ResolveHorizontalSeams();


			//doing this updates the rows' maps to their new maps
			for (int i = 0; i < mNumGridRows; ++i)
			{
				mGridBottomRow[i]->UpdateHeightMap();
				mGridMiddleRow[i]->UpdateHeightMap();
				mGridTopRow[i]->UpdateHeightMap();
			}


			//moves the map upwards
			UpdateChunkPositions();


			//update current chunk information
			UpdateCurrentChunk(xPos, zPos);
		}
		else if (zPos < mCurrentChunkBorders.minZ)
		{
			//move down
			UpdateZPositionDown();

			//ResolveVerticalSeams();
			//ResolveHorizontalSeams();

			UpdateChunkPositions();

			//update current chunk
			UpdateCurrentChunk(xPos, zPos);
		}
	}

	void TerrainManager::Serialize(std::shared_ptr<Terrain> terrainChunk)
	{
		string filename = mSerializationFilename + to_string(terrainChunk->GetGridPositionX()) + to_string(terrainChunk->GetGridPositionY()) + ".bin";

		std::ofstream os(filename, std::ios::binary);

		if (static_cast<bool>(os))
		{
			cereal::BinaryOutputArchive archive(os);
			archive(terrainChunk);
		}
	}

	bool TerrainManager::Deserialize(int gridX, int gridY, std::shared_ptr<Terrain>& terrainChunk)
	{
		string filename = mSerializationFilename + to_string(gridX) + to_string(gridY) + ".bin";
		std::ifstream is(filename, std::ios::binary);

		//checking to make sure the data to retrieve has already been serialized (exists)
		if (is)
		{
			shared_ptr<Terrain> temp = make_shared<Terrain>();

			cereal::BinaryInputArchive archive(is);
			archive(temp);

			terrainChunk->SetHeightMapInfo(temp);

			return true;
		}

		return false;
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

		//serializing the right column & setting its height map info equal to the middle column height map info
		for (int i = 0; i < mNumGridRows; ++i)
		{
			Serialize(mGridRightColumn[i]);
			mGridRightColumn[i]->SetHeightMapInfo(mGridMiddleColumn[i]);

			//setting the middle column height map info equal to the left column height map info
			mGridMiddleColumn[i]->SetHeightMapInfo(mGridLeftColumn[i]);

			//updating the grid positions for the left column
			mGridLeftColumn[i]->SetGridPosition((mGridLeftColumn[0]->GetGridPositionX() - 1), mGridLeftColumn[0]->GetGridPositionY());
		}

		//deserializing saved height maps or generating new height maps for the left column
		for (int i = 0; i < mNumGridRows; ++i)
		{
			if (!Deserialize(mGridLeftColumn[i]->GetGridPositionX(), mGridLeftColumn[i]->GetGridPositionY(), mGridLeftColumn[i]))
			{
				GenerateNewHeightMap(mGridLeftColumn[i]->GetHeightMapFilename());
				mGridLeftColumn[i]->UpdateHeightMap();
			}
		}

		//ResolveVerticalSeams();
		//ResolveHorizontalSeams();

		//for (int i = 0; i < mNumGridRows; ++i)
		//{
		//	mGridLeftColumn[i]->UpdateHeightMap();
		//	mGridMiddleColumn[i]->UpdateHeightMap();
		//	mGridRightColumn[i]->UpdateHeightMap();
		//}
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

		for (int i = 0; i < mNumGridRows; ++i)
		{
			//serializing the left column & setting its height map info equal to the middle column height map info
			Serialize(mGridLeftColumn[i]);
			mGridLeftColumn[i]->SetHeightMapInfo(mGridMiddleColumn[i]);

			//setting the middle column height map info equal to the right column height map info
			mGridMiddleColumn[i]->SetHeightMapInfo(mGridRightColumn[i]);

			//updating the grid positions for the right column
			mGridRightColumn[i]->SetGridPosition((mGridRightColumn[i]->GetGridPositionX() + 1), mGridRightColumn[i]->GetGridPositionY());
		}

		//deserializing saved height maps or generating new height maps for the right column
		for (int i = 0; i < mNumGridRows; ++i)
		{
			if (!Deserialize(mGridRightColumn[i]->GetGridPositionX(), mGridRightColumn[i]->GetGridPositionY(), mGridRightColumn[i]))
			{
				GenerateNewHeightMap(mGridRightColumn[i]->GetHeightMapFilename());
				mGridRightColumn[i]->UpdateHeightMap();
			}
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

		for (int i = 0; i < mNumGridRows; ++i)
		{
			//serializing the bottom row & setting its height map info equal to the middle row height map info
			Serialize(mGridBottomRow[i]);
			mGridBottomRow[i]->SetHeightMapInfo(mGridMiddleRow[i]);

			//setting the middle row height map info equal to the top row height map info
			mGridMiddleRow[i]->SetHeightMapInfo(mGridTopRow[i]);

			//updating the grid positions for the top row
			mGridTopRow[i]->SetGridPosition(mGridTopRow[i]->GetGridPositionX(), (mGridTopRow[i]->GetGridPositionY() + 1));
		}

		//deserializing saved height maps or generating new height maps for the top row
		for (int i = 0; i < mNumGridRows; ++i)
		{
			//if height map data isn't saved for the top grid positions, generate 3 new height maps for the top row
			if (!Deserialize(mGridTopRow[i]->GetGridPositionX(), mGridTopRow[i]->GetGridPositionY(), mGridTopRow[i]))
			{
				GenerateNewHeightMap(mGridTopRow[i]->GetHeightMapFilename());
				//mGridTopRow[i]->UpdateHeightMap();
			}

			//mGridTopRow[i]->UpdateHeightMap();
		}

		//ResolveHorizontalSeams();
		//ResolveVerticalSeams();

		//for (int i = 0; i < mNumGridRows; ++i)
		//{
		//	mGridTopRow[i]->UpdateHeightMap();
		//	mGridMiddleRow[i]->UpdateHeightMap();
		//	mGridBottomRow[i]->UpdateHeightMap();
		//}
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

		for (int i = 0; i < mNumGridRows; ++i)
		{
			//serialize the top row & set its height map info equal to the middle row height map info
			Serialize(mGridTopRow[i]);
			mGridTopRow[i]->SetHeightMapInfo(mGridMiddleRow[i]);

			//setting the middle row height map info equal to the bottom row height map info
			mGridMiddleRow[i]->SetHeightMapInfo(mGridBottomRow[i]);

			//updating the grid positions for the bottom row
			mGridBottomRow[i]->SetGridPosition(mGridBottomRow[i]->GetGridPositionX(), (mGridBottomRow[i]->GetGridPositionY() - 1));
		}

		//deserializing saved height maps or generating new height maps for the bottom row
		for (int i = 0; i < mNumGridRows; ++i)
		{
			//if height map data isn't saved for the bottom grid positions, generate 3 new height maps for the bottom row
			if (!Deserialize(mGridBottomRow[i]->GetGridPositionX(), mGridBottomRow[i]->GetGridPositionY(), mGridBottomRow[i]))
			{
				GenerateNewHeightMap(mGridBottomRow[i]->GetHeightMapFilename());
				mGridBottomRow[i]->UpdateHeightMap();
			}
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

	void TerrainManager::ResolveVerticalSeams()
	{

		ResolveVerticalSeam(mGridBottomRow[0]->GetHeightMapFilename(), mGridBottomRow[1]->GetHeightMapFilename());
		ResolveVerticalSeam(mGridMiddleRow[0]->GetHeightMapFilename(), mGridMiddleRow[1]->GetHeightMapFilename());
		ResolveVerticalSeam(mGridTopRow[0]->GetHeightMapFilename(), mGridTopRow[1]->GetHeightMapFilename());

		ResolveVerticalSeam(mGridBottomRow[1]->GetHeightMapFilename(), mGridBottomRow[2]->GetHeightMapFilename());
		ResolveVerticalSeam(mGridMiddleRow[1]->GetHeightMapFilename(), mGridMiddleRow[2]->GetHeightMapFilename());
		ResolveVerticalSeam(mGridTopRow[1]->GetHeightMapFilename(), mGridTopRow[2]->GetHeightMapFilename());

		//ResolveVerticalSeam(mHeightMapFilenames[0], mHeightMapFilenames[3]);
		//ResolveVerticalSeam(mHeightMapFilenames[1], mHeightMapFilenames[4]);
		//ResolveVerticalSeam(mHeightMapFilenames[2], mHeightMapFilenames[5]);
		//ResolveVerticalSeam(mHeightMapFilenames[5], mHeightMapFilenames[8]);
		//ResolveVerticalSeam(mHeightMapFilenames[4], mHeightMapFilenames[7]);
		//ResolveVerticalSeam(mHeightMapFilenames[3], mHeightMapFilenames[6]);

	}

	void TerrainManager::ResolveHorizontalSeams()
	{

		ResolveHorizontalSeam(mGridMiddleRow[0]->GetHeightMapFilename(), mGridBottomRow[0]->GetHeightMapFilename());
		ResolveHorizontalSeam(mGridMiddleRow[1]->GetHeightMapFilename(), mGridBottomRow[1]->GetHeightMapFilename());
		ResolveHorizontalSeam(mGridMiddleRow[2]->GetHeightMapFilename(), mGridBottomRow[2]->GetHeightMapFilename());

		ResolveHorizontalSeam(mGridTopRow[0]->GetHeightMapFilename(), mGridMiddleRow[0]->GetHeightMapFilename());
		ResolveHorizontalSeam(mGridTopRow[1]->GetHeightMapFilename(), mGridMiddleRow[1]->GetHeightMapFilename());
		ResolveHorizontalSeam(mGridTopRow[2]->GetHeightMapFilename(), mGridMiddleRow[2]->GetHeightMapFilename());

		//ResolveHorizontalSeam(mHeightMapFilenames[1], mHeightMapFilenames[0]);
		//ResolveHorizontalSeam(mHeightMapFilenames[4], mHeightMapFilenames[3]);
		//ResolveHorizontalSeam(mHeightMapFilenames[7], mHeightMapFilenames[6]);
		//ResolveHorizontalSeam(mHeightMapFilenames[2], mHeightMapFilenames[1]);
		//ResolveHorizontalSeam(mHeightMapFilenames[5], mHeightMapFilenames[4]);
		//ResolveHorizontalSeam(mHeightMapFilenames[8], mHeightMapFilenames[7]);

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

	void TerrainManager::GenerateNewHeightMap(char* filename)
	{
		mHeightMapGenerator.SetIsScaleMap(false);

		//int seed = mDistribution(mRandomSeedGenerator);
		//int seed = mRandomSeedGenerator();
		unsigned timeSeed = std::chrono::system_clock::now().time_since_epoch().count();

		mHeightMapGenerator.SetSeed(timeSeed);
		mHeightMapGenerator.Generate(filename, mHMHeight, mHMWidth);
	}
}