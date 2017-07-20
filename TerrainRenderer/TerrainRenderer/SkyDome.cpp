#include "SkyDome.h"

namespace TerrainRenderer
{
	SkyDome::SkyDome()
	{
		mModel = 0;
		mVertexBuffer = 0;
		mIndexBuffer = 0;
	}


	SkyDome::SkyDome(const SkyDome& rhs)
	{
	}


	SkyDome::~SkyDome()
	{
	}


	bool SkyDome::Initialize(ID3D11Device* device)
	{
		bool result;


		// Load in the sky dome model.
		result = LoadSkyDomeModel("../TerrainRenderer/data/skydome.txt");
		if (!result)
		{
			return false;
		}

		// Load the sky dome into a vertex and index buffer for rendering.
		result = InitializeBuffers(device);
		if (!result)
		{
			return false;
		}

		// Set the color at the top of the sky dome.
		mApexColor = D3DXVECTOR4(0.10f, 0.30f, 0.90f, 1.0f);

		// Set the color at the center of the sky dome.
		//mCenterColor = D3DXVECTOR4(0.05f, 0.65f, 0.90f, 1.0f);
		mCenterColor = D3DXVECTOR4(0.20f, 0.65f, 0.90f, 1.0f);

		// Set the color at the bottom of the sky dome
		mBaseColor = D3DXVECTOR4(0.15f, 0.35f, 0.01f, 1.0f);

		return true;
	}


	void SkyDome::Shutdown()
	{
		// Release the vertex and index buffer that were used for rendering the sky dome.
		ReleaseBuffers();

		// Release the sky dome model.
		ReleaseSkyDomeModel();
	}


	void SkyDome::Render(ID3D11DeviceContext* deviceContext)
	{
		// Render the sky dome.
		RenderBuffers(deviceContext);

		return;
	}


	int SkyDome::GetIndexCount()
	{
		return mIndexCount;
	}


	D3DXVECTOR4 SkyDome::GetApexColor()
	{
		return mApexColor;
	}


	D3DXVECTOR4 SkyDome::GetCenterColor()
	{
		return mCenterColor;
	}

	D3DXVECTOR4 SkyDome::GetBaseColor()
	{
		return mBaseColor;
	}

	bool SkyDome::LoadSkyDomeModel(char* filename)
	{
		ifstream fin;
		char input;


		// Open the model file.
		fin.open(filename);

		// If it could not open the file then exit.
		if (fin.fail())
		{
			return false;
		}

		// Read up to the value of vertex count.
		fin.get(input);
		while (input != ':')
		{
			fin.get(input);
		}

		// Read in the vertex count.
		fin >> mVertexCount;

		// Set the number of indices to be the same as the vertex count.
		mIndexCount = mVertexCount;

		// Create the model using the vertex count that was read in.
		mModel = new ModelType[mVertexCount];
		if (!mModel)
		{
			return false;
		}

		// Read up to the beginning of the data.
		fin.get(input);
		while (input != ':')
		{
			fin.get(input);
		}
		fin.get(input);
		fin.get(input);

		// Read in the vertex data.
		for (int i = 0; i < mVertexCount; i++)
		{
			fin >> mModel[i].x >> mModel[i].y >> mModel[i].z;
			fin >> mModel[i].tu >> mModel[i].tv;
			fin >> mModel[i].nx >> mModel[i].ny >> mModel[i].nz;
		}

		// Close the model file.
		fin.close();

		return true;
	}


	void SkyDome::ReleaseSkyDomeModel()
	{
		if (mModel)
		{
			delete[] mModel;
			mModel = 0;
		}
	}


	bool SkyDome::InitializeBuffers(ID3D11Device* device)
	{
		VertexType* vertices;
		unsigned long* indices;
		D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
		D3D11_SUBRESOURCE_DATA vertexData, indexData;
		HRESULT result;
		int i;


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

		// Load the vertex array and index array with data.
		for (i = 0; i < mVertexCount; i++)
		{
			vertices[i].position = D3DXVECTOR3(mModel[i].x, mModel[i].y, mModel[i].z);
			indices[i] = i;
		}

		// Set up the description of the vertex buffer.
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

		// Now finally create the vertex buffer.
		result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &mVertexBuffer);
		if (FAILED(result))
		{
			return false;
		}

		// Set up the description of the index buffer.
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

		// Release the arrays now that the vertex and index buffers have been created and loaded.
		delete[] vertices;
		vertices = 0;

		delete[] indices;
		indices = 0;

		return true;
	}


	void SkyDome::ReleaseBuffers()
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


	void SkyDome::RenderBuffers(ID3D11DeviceContext* deviceContext)
	{
		unsigned int stride;
		unsigned int offset;


		// Set vertex buffer stride and offset.
		stride = sizeof(VertexType);
		offset = 0;

		// Set the vertex buffer to active in the input assembler so it can be rendered.
		deviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);

		// Set the index buffer to active in the input assembler so it can be rendered.
		deviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}