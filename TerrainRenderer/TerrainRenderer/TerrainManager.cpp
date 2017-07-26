#include "Application.h"
#include "TerrainManager.h"

using namespace std;

namespace TerrainRenderer
{
	TerrainManager::TerrainManager(): 
		mHeightMapFilenames(), mGridBottomRow(), mGridMiddleRow(), mGridTopRow(), mScalingFilenames()
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

		//setting up quad trees
		mQuadBottomRow.resize(mNumGridRows);
		mQuadMiddleRow.resize(mNumGridRows);
		mQuadTopRow.resize(mNumGridRows);

		for (int i = 0; i < mNumGridRows; ++i)
		{
			mQuadBottomRow[i] = make_shared<QuadTree>();
			mQuadMiddleRow[i] = make_shared<QuadTree>();
			mQuadTopRow[i] = make_shared<QuadTree>();
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

	bool TerrainManager::Initialize(ID3D11Device* device, vector<char*>* heightMapFilenames, vector<char*>* scalingFilenames, char* largeScalingFilename,
		WCHAR* grassTextureFilename, WCHAR* slopeTextureFilename, WCHAR* rockTextureFilename)
	{
		mHeightMapFilenames = *heightMapFilenames;
		mScalingFilenames = *scalingFilenames;
		mLargeScalingMap = largeScalingFilename;

		//applying scaling maps to the height maps here
		if (SCALING_ENABLED)
		{
			PartitionScalingMap();

			if (mHeightMapFilenames.size() == mScalingFilenames.size())
			{
				for (int i = 0; i < mHeightMapFilenames.size(); ++i)
				{
					ApplyScalingMap(mHeightMapFilenames[i], mScalingFilenames[i]);
				}
			}

			//getting rid of the used scaling maps
			mScalingFilenames.clear();
		}

		//*then the scaling maps wouldn't need to be stored in Terrain

		//stitch together the height maps here
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

		int filenameIndex = 0;

		//initializing the terrain chunks
		if (mHeightMapFilenames.size() == mNumGridRows*mNumGridRows)
		{
			for (int i = 0; i < mNumGridRows; ++i)
			{
				(mGridBottomRow[i])->Initialize(device, mHeightMapFilenames[filenameIndex], grassTextureFilename, slopeTextureFilename, rockTextureFilename, 
					(*mBottomRowOffsets)[i].x, (*mBottomRowOffsets)[i].z);
				mGridBottomRow[i]->SetGridPosition((*mStartingGridPositions)[filenameIndex].x, (*mStartingGridPositions)[filenameIndex].z);
				++filenameIndex;
				(mGridMiddleRow[i])->Initialize(device, mHeightMapFilenames[filenameIndex], grassTextureFilename, slopeTextureFilename, rockTextureFilename, 
					(*mMiddleRowOffsets)[i].x, (*mMiddleRowOffsets)[i].z);
				mGridMiddleRow[i]->SetGridPosition((*mStartingGridPositions)[filenameIndex].x, (*mStartingGridPositions)[filenameIndex].z);
				++filenameIndex;
				(mGridTopRow[i])->Initialize(device, mHeightMapFilenames[filenameIndex], grassTextureFilename, slopeTextureFilename, rockTextureFilename, 
					(*mTopRowOffsets)[i].x, (*mTopRowOffsets)[i].z);
				mGridTopRow[i]->SetGridPosition((*mStartingGridPositions)[filenameIndex].x, (*mStartingGridPositions)[filenameIndex].z);
				++filenameIndex;
			}
		}

		//initializing the quad trees
		if (QUADTREES_ENABLED)
		{
			for (int i = 0; i < mNumGridRows; ++i)
			{
				(mQuadBottomRow[i])->Initialize(mGridBottomRow[i], device);
				(mQuadMiddleRow[i])->Initialize(mGridMiddleRow[i], device);
				(mQuadTopRow[i])->Initialize(mGridTopRow[i], device);
			}
		}

		return true;
	}

	void TerrainManager::Shutdown()
	{
		//should delete all serialized files?
	}

	void TerrainManager::Render(ID3D11DeviceContext* context, TerrainShader* terrainShader, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX projection,
		D3DXVECTOR4 ambientColor, D3DXVECTOR4 diffuseColor, D3DXVECTOR3 lightDirection, Frustum* frustum, Position* position, D3DXVECTOR4 fogColor)
	{
		//send camera position & fog color in render call
		float posX, posY, posZ;
		position->GetPosition(posX, posY, posZ);
		D3DXVECTOR3 cameraPosition = D3DXVECTOR3(posX, posY, posZ);
		D3DXVECTOR4 skyDomeColor = D3DXVECTOR4(0.20f, 0.65f, 0.90f, 1.0f);

		if (QUADTREES_ENABLED)
		{
			for (int i = 0; i < mNumGridRows; ++i)
			{
				
				terrainShader->SetShaderParameters(context, world, view, projection, ambientColor, diffuseColor, lightDirection, cameraPosition, skyDomeColor,
					mGridBottomRow[i]->GetGrassTexture(), mGridBottomRow[i]->GetSlopeTexture(), mGridBottomRow[i]->GetRockTexture());
				mQuadBottomRow[i]->Render(frustum, context, terrainShader);

				terrainShader->SetShaderParameters(context, world, view, projection, ambientColor, diffuseColor, lightDirection, cameraPosition, skyDomeColor,
					mGridMiddleRow[i]->GetGrassTexture(), mGridMiddleRow[i]->GetSlopeTexture(), mGridMiddleRow[i]->GetRockTexture());
				mQuadMiddleRow[i]->Render(frustum, context, terrainShader);

				terrainShader->SetShaderParameters(context, world, view, projection, ambientColor, diffuseColor, lightDirection, cameraPosition, skyDomeColor,
					mGridTopRow[i]->GetGrassTexture(), mGridTopRow[i]->GetSlopeTexture(), mGridTopRow[i]->GetRockTexture());
				mQuadTopRow[i]->Render(frustum, context, terrainShader);
			}
		}
		else
		{
			for (int i = 0; i < mNumGridRows; ++i)
			{
				(mGridBottomRow[i])->Render(context);
				terrainShader->Render(context, mGridBottomRow[i]->GetIndexCount(), world, view, projection, ambientColor, diffuseColor, lightDirection, 
					cameraPosition, skyDomeColor, mGridBottomRow[i]->GetGrassTexture(), mGridBottomRow[i]->GetSlopeTexture(), mGridBottomRow[i]->GetRockTexture());

				(mGridMiddleRow[i])->Render(context);
				terrainShader->Render(context, mGridMiddleRow[i]->GetIndexCount(), world, view, projection, ambientColor, diffuseColor, lightDirection, 
					cameraPosition, skyDomeColor, mGridMiddleRow[i]->GetGrassTexture(), mGridMiddleRow[i]->GetSlopeTexture(), mGridMiddleRow[i]->GetRockTexture());

				(mGridTopRow[i])->Render(context);
				terrainShader->Render(context, mGridTopRow[i]->GetIndexCount(), world, view, projection, ambientColor, diffuseColor, lightDirection, 
					cameraPosition, skyDomeColor, mGridTopRow[i]->GetGrassTexture(), mGridTopRow[i]->GetSlopeTexture(), mGridTopRow[i]->GetRockTexture());
			}
		}
	}

	void TerrainManager::GenerateChunks(Position* position)
	{
		float xPos, yPos, zPos;
		position->GetPosition(xPos, yPos, zPos);
		bool chunksGenerated = false;

		if (xPos < mCurrentChunkBorders.minX)
		{
			//move left
			UpdateXPositionLeft();

			//stitches together chunks that have been added
			ResolveMoveLeftSeams();

			chunksGenerated = true;
		}
		else if (xPos > mCurrentChunkBorders.maxX)
		{
			//move right
			UpdateXPositionRight();

			//stitches together chunks that have been added
			ResolveMoveRightSeams();

			chunksGenerated = true;
		}
		else if (zPos > mCurrentChunkBorders.maxZ)
		{
			//move up
			UpdateZPositionUp();

			//stitches together chunks that have been added
			ResolveMoveUpSeams();

			chunksGenerated = true;
		}
		else if (zPos < mCurrentChunkBorders.minZ)
		{
			//move down
			UpdateZPositionDown();

			//stitches together chunks that have been added
			ResolveMoveDownSeams();

			chunksGenerated = true;
		}

		if (chunksGenerated)
		{
			//moving map
			UpdateChunkPositions();

			if (QUADTREES_ENABLED)
			{
				UpdateQuadTrees();
			}

			//updating current chunk
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
		if (static_cast<bool>(is))
		{
			shared_ptr<Terrain> temp = make_shared<Terrain>();

			cereal::BinaryInputArchive archive(is);
			archive(temp);

			terrainChunk->SetHeightMapInfo(temp);

			return true;
		}

		return false;
	}

	int TerrainManager::GetTriDrawCounts()
	{
		int drawCountTotal = 0;

		for (int i = 0; i < mNumGridRows; ++i)
		{
			drawCountTotal += mQuadBottomRow[i]->GetTriDrawCount();
			drawCountTotal += mQuadMiddleRow[i]->GetTriDrawCount();
			drawCountTotal += mQuadTopRow[i]->GetTriDrawCount();
		}

		return drawCountTotal;
	}

	int TerrainManager::GetTriTotalCounts()
	{
		int totalTriCount = 0;

		for (int i = 0; i < mNumGridRows; ++i)
		{
			totalTriCount += mQuadBottomRow[i]->GetTriTotalCount();
			totalTriCount += mQuadMiddleRow[i]->GetTriTotalCount();
			totalTriCount += mQuadTopRow[i]->GetTriTotalCount();
		}

		return totalTriCount;
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

		for (int i = 0; i < mNumGridRows; ++i)
		{
			//serializing the right column & setting its height map & grid position equal to the middle column's height map & grid position
			Serialize(mGridRightColumn[i]);
			mGridRightColumn[i]->SetHeightMapInfo(mGridMiddleColumn[i]);

			//setting the middle column height map & grid position equal to the left column height map & grid position
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

				//loading in the data for the new height map
				mGridLeftColumn[i]->UpdateHeightMap();
			}
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

		for (int i = 0; i < mNumGridRows; ++i)
		{
			//serializing the left column & setting its height map & grid position equal to the middle column height map & grid position
			Serialize(mGridLeftColumn[i]);
			mGridLeftColumn[i]->SetHeightMapInfo(mGridMiddleColumn[i]);

			//setting the middle column height map & grid position equal to the right column height map & grid position
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

				//loading in the new height map data
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
			//serializing the bottom row & setting its height map & grid position equal to the middle row height map & grid position
			Serialize(mGridBottomRow[i]);
			mGridBottomRow[i]->SetHeightMapInfo(mGridMiddleRow[i]);

			//setting the middle row height map & grid position equal to the top row height map & grid position
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

				//loading in the new height map
				mGridTopRow[i]->UpdateHeightMap();
			}
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

		for (int i = 0; i < mNumGridRows; ++i)
		{
			//serialize the top row & set its height map & grid position equal to the middle row height map & grid position
			Serialize(mGridTopRow[i]);
			mGridTopRow[i]->SetHeightMapInfo(mGridMiddleRow[i]);

			//setting the middle row height map & grid position equal to the bottom row height map & grid position
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

				//need to load in the data for the new height map
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

	void TerrainManager::UpdateQuadTrees()
	{
		for (int i = 0; i < mNumGridRows; ++i)
		{
			mQuadBottomRow[i]->UpdateTerrainData();
			mQuadMiddleRow[i]->UpdateTerrainData();
			mQuadTopRow[i]->UpdateTerrainData();
		}
	}

	void TerrainManager::ResolveMoveLeftSeams()
	{
		StitchHorizontalSeam(mGridMiddleRow[0]->GetHeightMap(), mGridBottomRow[0]->GetHeightMap());
		StitchHorizontalSeam(mGridTopRow[0]->GetHeightMap(), mGridMiddleRow[0]->GetHeightMap());

		StitchVerticalSeam(mGridBottomRow[0]->GetHeightMap(), mGridBottomRow[1]->GetHeightMap());
		StitchVerticalSeam(mGridMiddleRow[0]->GetHeightMap(), mGridMiddleRow[1]->GetHeightMap());
		StitchVerticalSeam(mGridTopRow[0]->GetHeightMap(), mGridTopRow[1]->GetHeightMap());
	}

	void TerrainManager::ResolveMoveRightSeams()
	{
		StitchVerticalSeam(mGridBottomRow[1]->GetHeightMap(), mGridBottomRow[2]->GetHeightMap());
		StitchVerticalSeam(mGridMiddleRow[1]->GetHeightMap(), mGridMiddleRow[2]->GetHeightMap());
		StitchVerticalSeam(mGridTopRow[1]->GetHeightMap(), mGridTopRow[2]->GetHeightMap());

		StitchHorizontalSeam(mGridMiddleRow[2]->GetHeightMap(), mGridBottomRow[2]->GetHeightMap());
		StitchHorizontalSeam(mGridTopRow[2]->GetHeightMap(), mGridMiddleRow[2]->GetHeightMap());
	}

	void TerrainManager::ResolveMoveUpSeams()
	{
		StitchVerticalSeam(mGridTopRow[0]->GetHeightMap(), mGridTopRow[1]->GetHeightMap());
		StitchVerticalSeam(mGridTopRow[1]->GetHeightMap(), mGridTopRow[2]->GetHeightMap());

		StitchHorizontalSeam(mGridTopRow[0]->GetHeightMap(), mGridMiddleRow[0]->GetHeightMap());
		StitchHorizontalSeam(mGridTopRow[1]->GetHeightMap(), mGridMiddleRow[1]->GetHeightMap());
		StitchHorizontalSeam(mGridTopRow[2]->GetHeightMap(), mGridMiddleRow[2]->GetHeightMap());
	}

	void TerrainManager::ResolveMoveDownSeams()
	{
		StitchVerticalSeam(mGridBottomRow[0]->GetHeightMap(), mGridBottomRow[1]->GetHeightMap());
		StitchVerticalSeam(mGridBottomRow[1]->GetHeightMap(), mGridBottomRow[2]->GetHeightMap());

		StitchHorizontalSeam(mGridMiddleRow[0]->GetHeightMap(), mGridBottomRow[0]->GetHeightMap());
		StitchHorizontalSeam(mGridMiddleRow[1]->GetHeightMap(), mGridBottomRow[1]->GetHeightMap());
		StitchHorizontalSeam(mGridMiddleRow[2]->GetHeightMap(), mGridBottomRow[2]->GetHeightMap());
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

	void TerrainManager::StitchVerticalSeam(vector<Terrain::HeightMapData>& leftChunk, vector<Terrain::HeightMapData>& rightChunk)
	{
		//building the border for the right hand side
		float weightChange = (float)(1.0f / mBorderWidth);
		float leftWeight = 1.0f;
		float rightWeight = 0.0f;

		int leftOffset = 63;

		for (int i = 0; i < mBorderWidth; ++i)
		{
			for (int j = 0; j < mHMHeight * mHMHeight; j += mHMHeight)
			{
				if (i == 0)
				{
					rightChunk[i + j].y = leftChunk[leftOffset + j].y;
				}
				else
				{
					rightChunk[i + j].y = (rightChunk[(i - 1) + j].y * leftWeight) + (rightChunk[i + j].y * rightWeight);
				}

			}

			leftWeight -= weightChange;
			rightWeight += weightChange;
		}

		//building the border for the left hand side
		leftWeight = 0.0f;
		rightWeight = 1.0f;

		int rightOffset = 0;

		for (int i = (mHMHeight - 1); i >= (mHMHeight - mBorderWidth); --i)
		{
			for (int j = 0; j < mHMHeight * mHMHeight; j += mHMHeight)
			{
				if (i == (mHMHeight - 1))
				{
					leftChunk[i + j].y = rightChunk[rightOffset + j].y;
				}
				else
				{
					leftChunk[i + j].y = (leftChunk[i + j].y * leftWeight) + (leftChunk[(i + 1) + j].y * rightWeight);
				}

			}

			leftWeight += weightChange;
			rightWeight -= weightChange;
		}
	}

	void TerrainManager::StitchHorizontalSeam(vector<Terrain::HeightMapData>& topChunk, vector<Terrain::HeightMapData>& bottomChunk)
	{
		//building the border for the top chunk
		float weightChange = (float)(1.0f / mBorderWidth);
		float bottomWeight = 1.0f;
		float topWeight = 0.0f;

		int bottomOffset = mHMHeight * (mHMHeight - 1);	

		for (int i = 0; i < mHMHeight; ++i)
		{
			topChunk[i].y = bottomChunk[bottomOffset].y;
			++bottomOffset;
		}

		for (int i = mHMHeight; i < mHMHeight * mBorderWidth; ++i)
		{
			if (i % mHMHeight == 0)
			{
				bottomWeight -= weightChange;
				topWeight += weightChange;
			}

			topChunk[i].y = (topChunk[i - mHMHeight].y * bottomWeight) + (topChunk[i].y * topWeight);
		}

		//building the border for the bottom chunk
		bottomWeight = 0.0f;
		topWeight = 1.0f;

		int topOffset = 0;

		for (int i = mHMHeight * (mHMHeight - 1); i < (mHMHeight * mHMHeight); ++i)
		{
			bottomChunk[i].y = topChunk[topOffset].y;
			++topOffset;
		}

		bottomWeight += weightChange;
		topWeight -= weightChange;

		for (int i = (mHMHeight * (mHMHeight - 1) - 1); i >= (mHMHeight * (mHMHeight - mBorderWidth)); --i)
		{
			if (i % mHMHeight == 0)
			{
				bottomWeight += weightChange;
				topWeight -= weightChange;
			}
			
			bottomChunk[i].y = (bottomChunk[i].y * bottomWeight) + (bottomChunk[i + mHMHeight].y * topWeight);
		}
	}
	
	//Partitioning the large scale map into 9 smaller 64x64 chunks
	void TerrainManager::PartitionScalingMap()
	{
		if (mScalingFilenames.empty())
		{
			for (int i = 0; i < mNumGridRows * mNumGridRows; ++i)
			{
				string filename = mScalingFilenameStart;
				filename += to_string(i) + ".bmp";
				mScalingFilenames.emplace_back(const_cast<char*>(filename.c_str()));
			}
		}

		BMP largeMap, smallMap;

		largeMap.ReadFromFile(mLargeScalingMap);
		smallMap.SetSize(64, 64);

		int smallJIndex = 0;
		int smallIIndex = 0;
		int maxJ = 192;
		int minI = 0;
		const int width = 64;

		for (int k = 0; k < mNumGridRows * mNumGridRows; ++k)
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
		unsigned timeSeed = std::chrono::system_clock::now().time_since_epoch().count();

		mHeightMapGenerator.SetIsScaleMap(false);
		mHeightMapGenerator.SetSeed(timeSeed);
		mHeightMapGenerator.Generate(filename, mHMHeight, mHMWidth);

		//apply scaling map here
		if (mScalingFilenames.empty())
		{
			//generate a new large scaling map & partition it
			GenerateNewScalingMaps();
		}
		
		//apply scaling map at the end of the vector
		ApplyScalingMap(filename, mScalingFilenames[mScalingFilenames.size() - 1]);

		//pop the used scaling map off the end of the vector
		mScalingFilenames.pop_back();
	}

	void TerrainManager::GenerateNewScalingMaps()
	{
		//generate new scaling map
		unsigned timeSeed = std::chrono::system_clock::now().time_since_epoch().count();

		mHeightMapGenerator.SetIsScaleMap(true);
		mHeightMapGenerator.SetSeed(timeSeed);
		mHeightMapGenerator.Generate(mLargeScalingMap, mSMHeight, mSMWidth);

		//partition new scaling map
		PartitionScalingMap();
	}

	void TerrainManager::ApplyScalingMap(const char* heightMapFilename, const char* scalingMapFilename)
	{
		BMP heightMap, scalingMap;

		heightMap.ReadFromFile(heightMapFilename);
		scalingMap.ReadFromFile(scalingMapFilename);

		if (heightMap.TellWidth() == scalingMap.TellWidth() && heightMap.TellHeight() == scalingMap.TellHeight())
		{
			for (int i = 0; i < heightMap.TellWidth(); ++i)
			{
				for (int j = 0; j < heightMap.TellHeight(); ++j)
				{
					//find the scaling factor for the pixel from the scaling map pixel
					//float scalingFactor = 1 / scalingMap(i, j)->Red;
					float scalingFactor = scalingMap(i, j)->Red / 256.0f;
					//256 is the max color value, so scaling the red color to being between 0 & 1

					//multiply all the color values for the height map by the scaling factor?
					heightMap(i, j)->Red *= scalingFactor;
					heightMap(i, j)->Green *= scalingFactor;
					heightMap(i, j)->Blue *= scalingFactor;
					//heightMap(i, j)->Red /= scalingMap(i, j)->Red;
					//heightMap(i, j)->Green /= scalingMap(i, j)->Green;
					//heightMap(i, j)->Blue /= scalingMap(i, j)->Blue;

					//r, g, b, alpha
				}
			}
		}

		heightMap.WriteToFile(heightMapFilename);
	}

	void TerrainManager::SetNewHeightMap(const char* oldMapFilename, const char* newMapFilename)
	{
		BMP newMap;
		newMap.ReadFromFile(newMapFilename);
		newMap.WriteToFile(oldMapFilename);
	}
}