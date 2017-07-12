#include "Application.h"
#include "Terrain.h"

namespace TerrainRenderer 
{
	Terrain::Terrain(): 
		mVertexBuffer(nullptr), mIndexBuffer(nullptr), mGrassTexture(nullptr), mSlopeTexture(nullptr), mRockTexture(nullptr), mXOffset(0), mZOffset(0)
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

	bool Terrain::Initialize(ID3D11Device* device, char* heightMapFilename, WCHAR* grassTextureFilename, WCHAR* slopeTextureFilename,
		WCHAR* rockTextureFilename, int xOffset, int zOffset)
	{
		bool result;
		mXOffset = xOffset;
		mZOffset = zOffset;
		mHeightMapFilename = heightMapFilename;

		// Load in the height map for the terrain.
		result = LoadHeightMap(mHeightMapFilename);
		if (!result)
		{
			return false;
		}

		// Normalize the height of the height map.
		NormalizeHeightMap();

		//////////////////////////////////////////////////////////////////for the texturing
		result = CalculateNormals();
		if (!result)
		{
			return false;
		}

		CalculateTextureCoordinates();

		//loading the textures
		result = LoadTextures(device, grassTextureFilename, slopeTextureFilename, rockTextureFilename);
		if (!result)
		{
			return false;
		}
		//////////////////////////////////////////////////////////////////

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
		//releasing the textures
		ReleaseTextures();

		// Release the vertex and index buffer.
		ShutdownBuffers();

		// Release the height map data.
		ShutdownHeightMap();
	}


	void Terrain::Render(ID3D11DeviceContext* context)
	{
		// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
		RenderBuffers(context);
	}

	int Terrain::GetIndexCount()
	{
		return mIndexCount;
	}

	void Terrain::UpdatePosition(int xUpdate, int zUpdate)
	{
		mXOffset = xUpdate;
		mZOffset = zUpdate;

		//recalculating the normals & texture coordinates to update the texture
		CalculateNormals();
		CalculateTextureCoordinates();

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
		mGridPositionX = terrain->mGridPositionX;
		mGridPositionY = terrain->mGridPositionY;
	}

	ID3D11ShaderResourceView* Terrain::GetGrassTexture()
	{
		return mGrassTexture->GetTexture();
	}

	ID3D11ShaderResourceView* Terrain::GetSlopeTexture()
	{
		return mSlopeTexture->GetTexture();
	}

	ID3D11ShaderResourceView* Terrain::GetRockTexture()
	{
		return mRockTexture->GetTexture();
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
	}

	bool Terrain::CalculateNormals()
	{
		int index1, index2, index3, index, count;
		float vertex1[3], vertex2[3], vertex3[3], vector1[3], vector2[3], sum[3], length;
		VectorType* normals;

		// Create a temporary array to hold the un-normalized normal vectors.
		normals = new VectorType[(mTerrainHeight - 1) * (mTerrainWidth - 1)];
		if (!normals)
		{
			return false;
		}

		// Go through all the faces in the mesh and calculate their normals.
		for (int j = 0; j < (mTerrainHeight - 1); j++)
		{
			for (int i = 0; i < (mTerrainWidth - 1); i++)
			{
				index1 = (j * mTerrainHeight) + i;
				index2 = (j * mTerrainHeight) + (i + 1);
				index3 = ((j + 1) * mTerrainHeight) + i;

				// Get three vertices from the face.
				vertex1[0] = mHeightMap[index1].x;
				vertex1[1] = mHeightMap[index1].y;
				vertex1[2] = mHeightMap[index1].z;

				vertex2[0] = mHeightMap[index2].x;
				vertex2[1] = mHeightMap[index2].y;
				vertex2[2] = mHeightMap[index2].z;

				vertex3[0] = mHeightMap[index3].x;
				vertex3[1] = mHeightMap[index3].y;
				vertex3[2] = mHeightMap[index3].z;

				// Calculate the two vectors for this face.
				vector1[0] = vertex1[0] - vertex3[0];
				vector1[1] = vertex1[1] - vertex3[1];
				vector1[2] = vertex1[2] - vertex3[2];
				vector2[0] = vertex3[0] - vertex2[0];
				vector2[1] = vertex3[1] - vertex2[1];
				vector2[2] = vertex3[2] - vertex2[2];

				index = (j * (mTerrainHeight - 1)) + i;

				// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
				normals[index].x = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
				normals[index].y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
				normals[index].z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);
			}
		}

		// Now go through all the vertices and take an average of each face normal 	
		// that the vertex touches to get the averaged normal for that vertex.
		for (int j = 0; j < mTerrainHeight; j++)
		{
			for (int i = 0; i < mTerrainWidth; i++)
			{
				// Initialize the sum.
				sum[0] = 0.0f;
				sum[1] = 0.0f;
				sum[2] = 0.0f;

				// Initialize the count.
				count = 0;

				// Bottom left face.
				if (((i - 1) >= 0) && ((j - 1) >= 0))
				{
					index = ((j - 1) * (mTerrainHeight - 1)) + (i - 1);

					sum[0] += normals[index].x;
					sum[1] += normals[index].y;
					sum[2] += normals[index].z;
					count++;
				}

				// Bottom right face.
				if ((i < (mTerrainWidth - 1)) && ((j - 1) >= 0))
				{
					index = ((j - 1) * (mTerrainHeight - 1)) + i;

					sum[0] += normals[index].x;
					sum[1] += normals[index].y;
					sum[2] += normals[index].z;
					count++;
				}

				// Upper left face.
				if (((i - 1) >= 0) && (j < (mTerrainHeight - 1)))
				{
					index = (j * (mTerrainHeight - 1)) + (i - 1);

					sum[0] += normals[index].x;
					sum[1] += normals[index].y;
					sum[2] += normals[index].z;
					count++;
				}

				// Upper right face.
				if ((i < (mTerrainWidth - 1)) && (j < (mTerrainHeight - 1)))
				{
					index = (j * (mTerrainHeight - 1)) + i;

					sum[0] += normals[index].x;
					sum[1] += normals[index].y;
					sum[2] += normals[index].z;
					count++;
				}

				// Take the average of the faces touching this vertex.
				sum[0] = (sum[0] / (float)count);
				sum[1] = (sum[1] / (float)count);
				sum[2] = (sum[2] / (float)count);

				// Calculate the length of this normal.
				length = sqrt((sum[0] * sum[0]) + (sum[1] * sum[1]) + (sum[2] * sum[2]));

				// Get an index to the vertex location in the height map array.
				index = (j * mTerrainHeight) + i;

				// Normalize the final shared normal for this vertex and store it in the height map array.
				mHeightMap[index].nx = (sum[0] / length);
				mHeightMap[index].ny = (sum[1] / length);
				mHeightMap[index].nz = (sum[2] / length);
			}
		}

		// Release the temporary normals.
		delete[] normals;
		normals = 0;

		return true;
	}

	void Terrain::CalculateTextureCoordinates()
	{
		int incrementCount, tuCount, tvCount;
		float incrementValue, tuCoordinate, tvCoordinate;


		// Calculate how much to increment the texture coordinates by.
		incrementValue = (float)TEXTURE_REPEAT / (float)mTerrainWidth;

		// Calculate how many times to repeat the texture.
		incrementCount = mTerrainWidth / TEXTURE_REPEAT;

		// Initialize the tu and tv coordinate values.
		tuCoordinate = 0.0f;
		tvCoordinate = 1.0f;

		// Initialize the tu and tv coordinate indexes.
		tuCount = 0;
		tvCount = 0;

		// Loop through the entire height map and calculate the tu and tv texture coordinates for each vertex.
		for (int j = 0; j < mTerrainHeight; j++)
		{
			for (int i = 0; i < mTerrainWidth; i++)
			{
				// Store the texture coordinate in the height map.
				mHeightMap[(mTerrainHeight * j) + i].tu = tuCoordinate;
				mHeightMap[(mTerrainHeight * j) + i].tv = tvCoordinate;

				// Increment the tu texture coordinate by the increment value and increment the index by one.
				tuCoordinate += incrementValue;
				tuCount++;

				// Check if at the far right end of the texture and if so then start at the beginning again.
				if (tuCount == incrementCount)
				{
					tuCoordinate = 0.0f;
					tuCount = 0;
				}
			}

			// Increment the tv texture coordinate by the increment value and increment the index by one.
			tvCoordinate -= incrementValue;
			tvCount++;

			// Check if at the top of the texture and if so then start at the bottom again.
			if (tvCount == incrementCount)
			{
				tvCoordinate = 1.0f;
				tvCount = 0;
			}
		}
	}

	bool Terrain::LoadTextures(ID3D11Device* device, WCHAR* grassFilename, WCHAR* slopeFilename, WCHAR* rockFilename)
	{
		bool result;


		// Create the grass texture object.
		mGrassTexture = new Texture;
		if (!mGrassTexture)
		{
			return false;
		}

		// Initialize the grass texture object.
		result = mGrassTexture->Initialize(device, grassFilename);
		if (!result)
		{
			return false;
		}

		// Create the slope texture object.
		mSlopeTexture = new Texture;
		if (!mSlopeTexture)
		{
			return false;
		}

		// Initialize the slope texture object.
		result = mSlopeTexture->Initialize(device, slopeFilename);
		if (!result)
		{
			return false;
		}

		// Create the rock texture object.
		mRockTexture = new Texture;
		if (!mRockTexture)
		{
			return false;
		}

		// Initialize the rock texture object.
		result = mRockTexture->Initialize(device, rockFilename);
		if (!result)
		{
			return false;
		}

		return true;
	}

	void Terrain::ReleaseTextures()
	{
		// Release the texture objects.
		if (mGrassTexture)
		{
			mGrassTexture->Shutdown();
			delete mGrassTexture;
			mGrassTexture = nullptr;
		}

		if (mSlopeTexture)
		{
			mSlopeTexture->Shutdown();
			delete mSlopeTexture;
			mSlopeTexture = nullptr;
		}

		if (mRockTexture)
		{
			mRockTexture->Shutdown();
			delete mRockTexture;
			mRockTexture = nullptr;
		}
	}

	bool Terrain::InitializeBuffers(ID3D11Device* device)
	{
		mDevice = device;

		VertexType* vertices;
		unsigned long* indices;
		int index, i, j;
		D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
		D3D11_SUBRESOURCE_DATA vertexData, indexData;
		HRESULT result;
		int index1, index2, index3, index4;
		float tu, tv;

		// Calculate the number of vertices in the terrain mesh.
		//the mesh is made up of two triangles per quad, so only 6 vertices for each quad is needed
		mVertexCount = (mTerrainWidth - 1) * (mTerrainHeight - 1) * 6;

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
				tv = mHeightMap[index3].tv;

				//modify the texture coordinates to cover the top edge
				if (tv == 1.0f)
				{
					tv = 0.0f;
				}

				vertices[index].position = D3DXVECTOR3((mHeightMap[index3].x + mXOffset), mHeightMap[index3].y, (mHeightMap[index3].z + mZOffset));
				vertices[index].texture = D3DXVECTOR2(mHeightMap[index3].tu, tv);
				vertices[index].normal = D3DXVECTOR3(mHeightMap[index3].nx, mHeightMap[index3].ny, mHeightMap[index3].nz);
				indices[index] = index;
				index++;

				// Upper right.
				tu = mHeightMap[index4].tu;
				tv = mHeightMap[index4].tv;

				// Modify the texture coordinates to cover the top and right edge.
				if (tu == 0.0f)
				{
					tu = 1.0f;
				}

				if (tv == 1.0f)
				{
					tv = 0.0f;
				}

				vertices[index].position = D3DXVECTOR3((mHeightMap[index4].x + mXOffset), mHeightMap[index4].y, (mHeightMap[index4].z + mZOffset));
				vertices[index].texture = D3DXVECTOR2(tu, tv);
				vertices[index].normal = D3DXVECTOR3(mHeightMap[index4].nx, mHeightMap[index4].ny, mHeightMap[index4].nz);
				indices[index] = index;
				index++;

				// Bottom left.
				vertices[index].position = D3DXVECTOR3((mHeightMap[index1].x + mXOffset), mHeightMap[index1].y, (mHeightMap[index1].z + mZOffset));
				vertices[index].texture = D3DXVECTOR2(mHeightMap[index1].tu, mHeightMap[index1].tv);
				vertices[index].normal = D3DXVECTOR3(mHeightMap[index1].nx, mHeightMap[index1].ny, mHeightMap[index1].nz);
				indices[index] = index;
				index++;

				// Bottom left.
				vertices[index].position = D3DXVECTOR3((mHeightMap[index1].x + mXOffset), mHeightMap[index1].y, (mHeightMap[index1].z + mZOffset));
				vertices[index].texture = D3DXVECTOR2(mHeightMap[index1].tu, mHeightMap[index1].tv);
				vertices[index].normal = D3DXVECTOR3(mHeightMap[index1].nx, mHeightMap[index1].ny, mHeightMap[index1].nz);
				indices[index] = index;
				index++;

				// Upper right.
				tu = mHeightMap[index4].tu;
				tv = mHeightMap[index4].tv;

				// Modify the texture coordinates to cover the top and right edge.
				if (tu == 0.0f)
				{
					tu = 1.0f;
				}

				if (tv == 1.0f)
				{
					tv = 0.0f;
				}

				vertices[index].position = D3DXVECTOR3((mHeightMap[index4].x + mXOffset), mHeightMap[index4].y, (mHeightMap[index4].z + mZOffset));
				vertices[index].texture = D3DXVECTOR2(tu, tv);
				vertices[index].normal = D3DXVECTOR3(mHeightMap[index4].nx, mHeightMap[index4].ny, mHeightMap[index4].nz);
				indices[index] = index;
				index++;

				// Bottom right.
				tu = mHeightMap[index2].tu;

				// Modify the texture coordinates to cover the right edge.
				if (tu == 0.0f)
				{
					tu = 1.0f;
				}

				vertices[index].position = D3DXVECTOR3((mHeightMap[index2].x + mXOffset), mHeightMap[index2].y, (mHeightMap[index2].z + mZOffset));
				vertices[index].texture = D3DXVECTOR2(tu, mHeightMap[index2].tv);
				vertices[index].normal = D3DXVECTOR3(mHeightMap[index2].nx, mHeightMap[index2].ny, mHeightMap[index2].nz);
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
		//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}