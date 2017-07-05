#include "Terrain.h"

namespace TerrainRenderer 
{
	Terrain::Terrain(): 
		mVertexBuffer(nullptr), mIndexBuffer(nullptr), mXOffset(0), mZOffset(0)
	{

	}

	Terrain::Terrain(const Terrain& rhs)
	{

	}

	Terrain& Terrain::operator=(const Terrain& rhs)
	{
		return *this;
	}

	Terrain::~Terrain()
	{

	}

	bool Terrain::Initialize(ID3D11Device* device, char* heightMapFilename, char* scalingFilename, int xOffset, int zOffset)
	{
		bool result;
		mXOffset = xOffset;
		mZOffset = zOffset;
		mHeightScalingMap = scalingFilename;
		mHeightMapFilename = heightMapFilename;

		// Load in the height map for the terrain.
		result = LoadHeightMap(mHeightMapFilename);
		if (!result)
		{
			return false;
		}

		// Normalize the height of the height map.
		NormalizeHeightMap();

		//// Initialize the vertex and index buffer that hold the geometry for the terrain.
		//result = InitializeBuffers(device);
		//if (!result)
		//{
		//	return false;
		//}

		//testing scaling
		//loading in the height map for scaling the terrain
		result = LoadScalingMap(scalingFilename);
		if (!result)
		{
			return false;
		}

		//normalizing the height of the scaling map
		NormalizeScalingMap();

		// Initialize the vertex and index buffer that hold the geometry for the terrain.
		result = InitializeBuffers(device);
		if (!result)
		{
			return false;
		}

		return true;
	}


	void Terrain::Shutdown()
	{
		// Release the vertex and index buffer.
		ShutdownBuffers();

		// Release the height map data.
		ShutdownHeightMap();

		return;
	}


	void Terrain::Render(ID3D11DeviceContext* context)
	{
		// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
		RenderBuffers(context);

		return;
	}

	int Terrain::GetIndexCount()
	{
		return mIndexCount;
	}

	void Terrain::UpdatePosition(int xUpdate, int zUpdate)
	{
		mXOffset = xUpdate;
		mZOffset = zUpdate;

		InitializeBuffers(mDevice);
	}

	void Terrain::UpdateHeightMap()
	{
		//loading in the height map
		LoadHeightMap(mHeightMapFilename);

		//normalizing the height of the height map
		NormalizeHeightMap();
	}

	int Terrain::GetGridPositionX()
	{
		return mGridPositionX;
	}

	int Terrain::GetGridPositionY()
	{
		return mGridPositionY;
	}

	void Terrain::SetGridPosition(int x, int y)
	{
		mGridPositionX = x;
		mGridPositionY = y;
	}

	char* Terrain::GetHeightMapFilename()
	{
		return mHeightMapFilename;
	}

	void Terrain::SetHeightMapInfo(std::shared_ptr<Terrain> terrain)
	{
		mHeightMap = terrain->mHeightMap;
		mScalingMap = terrain->mScalingMap;
		mGridPositionX = terrain->mGridPositionX;
		mGridPositionY = terrain->mGridPositionY;
	}

	bool Terrain::LoadHeightMap(char* filename)
	{
		FILE* filePtr;
		int error;
		unsigned int count;
		BITMAPFILEHEADER bitmapFileHeader;
		BITMAPINFOHEADER bitmapInfoHeader;
		int imageSize, i, j, k, index;
		unsigned char* bitmapImage;
		unsigned char height;


		// Open the height map file in binary.
		error = fopen_s(&filePtr, filename, "rb");
		if (error != 0)
		{
			return false;
		}

		// Read in the file header.
		count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
		if (count != 1)
		{
			return false;
		}

		// Read in the bitmap info header.
		count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
		if (count != 1)
		{
			return false;
		}

		// Save the dimensions of the terrain.
		mTerrainWidth = bitmapInfoHeader.biWidth;
		mTerrainHeight = bitmapInfoHeader.biHeight;

		// Calculate the size of the bitmap image data.
		imageSize = mTerrainWidth * mTerrainHeight * 3;

		// Allocate memory for the bitmap image data.
		bitmapImage = new unsigned char[imageSize];
		if (!bitmapImage)
		{
			return false;
		}

		// Move to the beginning of the bitmap data.
		fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

		// Read in the bitmap image data.
		count = fread(bitmapImage, 1, imageSize, filePtr);
		if (count != imageSize)
		{
			return false;
		}

		// Close the file.
		error = fclose(filePtr);
		if (error != 0)
		{
			return false;
		}

		// Create the structure to hold the height map data.
		//mHeightMap = new HeightMapData[mTerrainWidth * mTerrainHeight];
		mHeightMap.resize(mTerrainWidth * mTerrainHeight);

		//if (!mHeightMap)
		//{
		//	return false;
		//}

		// Initialize the position in the image data buffer.
		k = 0;

		// Read the image data into the height map.
		for (j = 0; j < mTerrainHeight; j++)
		{
			for (i = 0; i < mTerrainWidth; i++)
			{
				height = bitmapImage[k];

				index = (mTerrainHeight * j) + i;

				mHeightMap[index].x = (float)i;
				mHeightMap[index].y = (float)height;
				mHeightMap[index].z = (float)j;

				k += 3;
			}
		}

		// Release the bitmap image data.
		delete[] bitmapImage;
		bitmapImage = 0;

		return true;
	}

	bool Terrain::LoadScalingMap(char* filename)
	{
		FILE* filePtr;
		int error;
		unsigned int count;
		BITMAPFILEHEADER bitmapFileHeader;
		BITMAPINFOHEADER bitmapInfoHeader;
		int imageSize, i, j, k, index;
		unsigned char* bitmapImage;
		unsigned char height;

		// Open the height map file in binary.
		error = fopen_s(&filePtr, filename, "rb");
		if (error != 0)
		{
			return false;
		}

		// Read in the file header.
		count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
		if (count != 1)
		{
			return false;
		}

		// Read in the bitmap info header.
		count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
		if (count != 1)
		{
			return false;
		}

		// Save the dimensions of the terrain.
		mTerrainWidth = bitmapInfoHeader.biWidth;
		mTerrainHeight = bitmapInfoHeader.biHeight;

		// Calculate the size of the bitmap image data.
		imageSize = mTerrainWidth * mTerrainHeight * 3;

		// Allocate memory for the bitmap image data.
		bitmapImage = new unsigned char[imageSize];
		if (!bitmapImage)
		{
			return false;
		}

		// Move to the beginning of the bitmap data.
		fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

		// Read in the bitmap image data.
		count = fread(bitmapImage, 1, imageSize, filePtr);
		if (count != imageSize)
		{
			return false;
		}

		// Close the file.
		error = fclose(filePtr);
		if (error != 0)
		{
			return false;
		}

		// Create the structure to hold the height map data.
		//mScalingMap = new HeightMapData[mTerrainWidth * mTerrainHeight];
		mScalingMap.resize(mTerrainWidth * mTerrainHeight);

		//if (!mScalingMap)
		//{
		//	return false;
		//}

		// Initialize the position in the image data buffer.
		k = 0;

		// Read the image data into the height map.
		for (j = 0; j < mTerrainHeight; j++)
		{
			for (i = 0; i < mTerrainWidth; i++)
			{
				height = bitmapImage[k];

				index = (mTerrainHeight * j) + i;

				mScalingMap[index].x = (float)i;
				mScalingMap[index].y = (float)height;
				mScalingMap[index].z = (float)j;

				k += 3;
			}
		}

		// Release the bitmap image data.
		delete[] bitmapImage;
		bitmapImage = 0;

		return true;
	}

	void Terrain::NormalizeHeightMap()
	{
		for (int j = 0; j < mTerrainHeight; j++)
		{
			for (int i = 0; i < mTerrainWidth; i++)
			{
				mHeightMap[(mTerrainHeight * j) + i].y /= 15.0f;
			}
		}
	}

	void Terrain::NormalizeScalingMap()
	{
		for (int j = 0; j < mTerrainHeight; j++)
		{
			for (int i = 0; i < mTerrainWidth; i++)
			{
				mScalingMap[(mTerrainHeight * j) + i].y /= 15.0f;
				mScalingMap[(mTerrainHeight * j) + i].y /= 15.0f;
			}
		}
	}

	void Terrain::ShutdownHeightMap()
	{
		//if (mHeightMap)
		//{
		//	delete[] mHeightMap;
		//	mHeightMap = 0;
		//}

		//if (mScalingMap)
		//{
		//	delete[] mScalingMap;
		//	mScalingMap = 0;
		//}

		return;
	}

	bool Terrain::InitializeBuffers(ID3D11Device* device)
	{
		float scale;

		mDevice = device;

		VertexType* vertices;
		unsigned long* indices;
		int index, i, j;
		D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
		D3D11_SUBRESOURCE_DATA vertexData, indexData;
		HRESULT result;
		int index1, index2, index3, index4;

		// Calculate the number of vertices in the terrain mesh.
		mVertexCount = (mTerrainWidth - 1) * (mTerrainHeight - 1) * 12;

		// Set the index count to the same as the vertex count.
		mIndexCount = mVertexCount;

		// Create the vertex array.
		vertices = new VertexType[mVertexCount];
		if (!vertices)
		{
			return false;
		}

		// Create the index array.
		indices = new unsigned long[mIndexCount];
		if (!indices)
		{
			return false;
		}

		// Initialize the index to the vertex buffer.
		index = 0;

		// Load the vertex and index array with the terrain data.
		for (j = 0; j < (mTerrainHeight - 1); j++)
		{
			for (i = 0; i < (mTerrainWidth - 1); i++)
			{
				index1 = (mTerrainHeight * j) + i;          // Bottom left.
				index2 = (mTerrainHeight * j) + (i + 1);      // Bottom right.
				index3 = (mTerrainHeight * (j + 1)) + i;      // Upper left.
				index4 = (mTerrainHeight * (j + 1)) + (i + 1);  // Upper right.

				// Upper left.
				//vertices[index].position = D3DXVECTOR3((mHeightMap[index3].x + mXOffset), mHeightMap[index3].y, (mHeightMap[index3].z + mZOffset));
				vertices[index].position = D3DXVECTOR3((mHeightMap[index3].x + mXOffset), mHeightMap[index3].y * mScalingMap[index3].y, (mHeightMap[index3].z + mZOffset));
				vertices[index].color = D3DXVECTOR4(mVertexColorR, mVertexColorG, mVertexColorB, mVertexColorAlpha);
				indices[index] = index;
				index++;

				// Upper right.
				/*vertices[index].position = D3DXVECTOR3((mHeightMap[index4].x + mXOffset), mHeightMap[index4].y, (mHeightMap[index4].z + mZOffset));*/
				vertices[index].position = D3DXVECTOR3((mHeightMap[index4].x + mXOffset), mHeightMap[index4].y * mScalingMap[index4].y, (mHeightMap[index4].z + mZOffset));
				vertices[index].color = D3DXVECTOR4(mVertexColorR, mVertexColorG, mVertexColorB, mVertexColorAlpha);
				indices[index] = index;
				index++;

				// Upper right.
				/*vertices[index].position = D3DXVECTOR3((mHeightMap[index4].x + mXOffset), mHeightMap[index4].y, (mHeightMap[index4].z + mZOffset));*/
				vertices[index].position = D3DXVECTOR3((mHeightMap[index4].x + mXOffset), mHeightMap[index4].y * mScalingMap[index4].y, (mHeightMap[index4].z + mZOffset));
				vertices[index].color = D3DXVECTOR4(mVertexColorR, mVertexColorG, mVertexColorB, mVertexColorAlpha);
				indices[index] = index;
				index++;

				// Bottom left.
				/*vertices[index].position = D3DXVECTOR3((mHeightMap[index1].x + mXOffset), mHeightMap[index1].y, (mHeightMap[index1].z + mZOffset));*/
				vertices[index].position = D3DXVECTOR3((mHeightMap[index1].x + mXOffset), mHeightMap[index1].y * mScalingMap[index1].y, (mHeightMap[index1].z + mZOffset));
				vertices[index].color = D3DXVECTOR4(mVertexColorR, mVertexColorG, mVertexColorB, mVertexColorAlpha);
				indices[index] = index;
				index++;

				// Bottom left.
				/*vertices[index].position = D3DXVECTOR3((mHeightMap[index1].x + mXOffset), mHeightMap[index1].y, (mHeightMap[index1].z + mZOffset));*/
				vertices[index].position = D3DXVECTOR3((mHeightMap[index1].x + mXOffset), mHeightMap[index1].y * mScalingMap[index1].y, (mHeightMap[index1].z + mZOffset));
				vertices[index].color = D3DXVECTOR4(mVertexColorR, mVertexColorG, mVertexColorB, mVertexColorAlpha);
				indices[index] = index;
				index++;

				// Upper left.
				/*vertices[index].position = D3DXVECTOR3((mHeightMap[index3].x + mXOffset), mHeightMap[index3].y, (mHeightMap[index3].z + mZOffset));*/
				vertices[index].position = D3DXVECTOR3((mHeightMap[index3].x + mXOffset), mHeightMap[index3].y * mScalingMap[index3].y, (mHeightMap[index3].z + mZOffset));
				vertices[index].color = D3DXVECTOR4(mVertexColorR, mVertexColorG, mVertexColorB, mVertexColorAlpha);
				indices[index] = index;
				index++;

				// Bottom left.
				/*vertices[index].position = D3DXVECTOR3((mHeightMap[index1].x + mXOffset), mHeightMap[index1].y, (mHeightMap[index1].z + mZOffset));*/
				vertices[index].position = D3DXVECTOR3((mHeightMap[index1].x + mXOffset), mHeightMap[index1].y * mScalingMap[index1].y, (mHeightMap[index1].z + mZOffset));
				vertices[index].color = D3DXVECTOR4(mVertexColorR, mVertexColorG, mVertexColorB, mVertexColorAlpha);
				indices[index] = index;
				index++;

				// Upper right.
				/*vertices[index].position = D3DXVECTOR3((mHeightMap[index4].x + mXOffset), mHeightMap[index4].y, (mHeightMap[index4].z + mZOffset));*/
				vertices[index].position = D3DXVECTOR3((mHeightMap[index4].x + mXOffset), mHeightMap[index4].y * mScalingMap[index4].y, (mHeightMap[index4].z + mZOffset));
				vertices[index].color = D3DXVECTOR4(mVertexColorR, mVertexColorG, mVertexColorB, mVertexColorAlpha);
				indices[index] = index;
				index++;

				// Upper right.
				/*vertices[index].position = D3DXVECTOR3((mHeightMap[index4].x + mXOffset), mHeightMap[index4].y, (mHeightMap[index4].z + mZOffset));*/
				vertices[index].position = D3DXVECTOR3((mHeightMap[index4].x + mXOffset), mHeightMap[index4].y * mScalingMap[index4].y, (mHeightMap[index4].z + mZOffset));
				vertices[index].color = D3DXVECTOR4(mVertexColorR, mVertexColorG, mVertexColorB, mVertexColorAlpha);
				indices[index] = index;
				index++;

				// Bottom right.
				/*vertices[index].position = D3DXVECTOR3((mHeightMap[index2].x + mXOffset), mHeightMap[index2].y, (mHeightMap[index2].z + mZOffset));*/
				vertices[index].position = D3DXVECTOR3((mHeightMap[index2].x + mXOffset), mHeightMap[index2].y * mScalingMap[index2].y, (mHeightMap[index2].z + mZOffset));
				vertices[index].color = D3DXVECTOR4(mVertexColorR, mVertexColorG, mVertexColorB, mVertexColorAlpha);
				indices[index] = index;
				index++;

				// Bottom right.
				/*vertices[index].position = D3DXVECTOR3((mHeightMap[index2].x + mXOffset), mHeightMap[index2].y, (mHeightMap[index2].z + mZOffset));*/
				vertices[index].position = D3DXVECTOR3((mHeightMap[index2].x + mXOffset), mHeightMap[index2].y * mScalingMap[index2].y, (mHeightMap[index2].z + mZOffset));
				vertices[index].color = D3DXVECTOR4(mVertexColorR, mVertexColorG, mVertexColorB, mVertexColorAlpha);
				indices[index] = index;
				index++;

				// Bottom left.
				/*vertices[index].position = D3DXVECTOR3((mHeightMap[index1].x + mXOffset), mHeightMap[index1].y, (mHeightMap[index1].z + mZOffset));*/
				vertices[index].position = D3DXVECTOR3((mHeightMap[index1].x + mXOffset), mHeightMap[index1].y * mScalingMap[index1].y, (mHeightMap[index1].z + mZOffset));
				vertices[index].color = D3DXVECTOR4(mVertexColorR, mVertexColorG, mVertexColorB, mVertexColorAlpha);
				indices[index] = index;
				index++;
			}
		}

		// Set up the description of the static vertex buffer.
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(VertexType) * mVertexCount;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the vertex data.
		vertexData.pSysMem = vertices;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		// Now create the vertex buffer.
		result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &mVertexBuffer);
		if (FAILED(result))
		{
			return false;
		}

		// Set up the description of the static index buffer.
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(unsigned long) * mIndexCount;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the index data.
		indexData.pSysMem = indices;
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		// Create the index buffer.
		result = device->CreateBuffer(&indexBufferDesc, &indexData, &mIndexBuffer);
		if (FAILED(result))
		{
			return false;
		}

		// Release the arrays now that the buffers have been created and loaded.
		delete[] vertices;
		vertices = 0;

		delete[] indices;
		indices = 0;

		return true;
	}


	void Terrain::ShutdownBuffers()
	{
		// Release the index buffer.
		if (mIndexBuffer)
		{
			mIndexBuffer->Release();
			mIndexBuffer = 0;
		}

		// Release the vertex buffer.
		if (mVertexBuffer)
		{
			mVertexBuffer->Release();
			mVertexBuffer = 0;
		}

		return;
	}


	void Terrain::RenderBuffers(ID3D11DeviceContext* context)
	{
		unsigned int stride;
		unsigned int offset;


		// Set vertex buffer stride and offset.
		stride = sizeof(VertexType);
		offset = 0;

		// Set the vertex buffer to active in the input assembler so it can be rendered.
		context->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);

		// Set the index buffer to active in the input assembler so it can be rendered.
		context->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Set the type of primitive that should be rendered from this vertex buffer, in this case a line list.
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		return;
	}
}