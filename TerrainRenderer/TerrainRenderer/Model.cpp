#include "Model.h"

namespace TerrainRenderer
{
	Model::Model() :
		mVertexBuffer(nullptr), mIndexBuffer(nullptr), mTexture(nullptr)
	{
		
	}

	Model::Model(const Model& rhs)
	{

	}

	Model& Model::operator=(const Model& rhs)
	{
		return *this;
	}

	Model::~Model()
	{

	}

	bool Model::Initialize(ID3D11Device* device, WCHAR* filename)
	{
		bool result;

		//initialing the vertex & index buffer that holds the geometry for the triangle
		result = InitializeBuffers(device);
		if (!result)
		{
			return false;
		}

		//loading the texture for this model
		result = LoadTexture(device, filename);
		if (!result)
		{
			return false;
		}

		return true;
	}

	void Model::Shutdown()
	{
		ReleaseTexture();
		ShutdownBuffers();
	}

	void Model::Render(ID3D11DeviceContext* context)
	{
		RenderBuffers(context);
	}

	int Model::GetIndexCount()
	{
		return mIndexCount;
	}

	ID3D11ShaderResourceView* Model::GetTexture()
	{
		return mTexture->GetTexture();
	}

	bool Model::InitializeBuffers(ID3D11Device* device)
	{
		VertexType* vertices;
		unsigned long* indices;
		D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
		D3D11_SUBRESOURCE_DATA vertexData, indexData;
		HRESULT result;

		mVertexCount = 3;
		mIndexCount = 3;
		
		vertices = new VertexType[mVertexCount];
		if (!vertices)
		{
			return false;
		}

		indices = new unsigned long[mIndexCount];
		if (!indices)
		{
			return false;
		}

		//bottom left
		vertices[0].position = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
		vertices[0].texture = D3DXVECTOR2(0.0f, 1.0f);
		vertices[0].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

		//top middle
		vertices[1].position = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		vertices[1].texture = D3DXVECTOR2(0.5f, 0.0f);
		vertices[1].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

		//bottom right
		vertices[2].position = D3DXVECTOR3(1.0f, -1.0f, 0.0f);
		vertices[2].texture = D3DXVECTOR2(1.0f, 1.0f);
		vertices[2].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(VertexType) * mVertexCount;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		vertexData.pSysMem = vertices;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		//creating the vertex buffer
		result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &mVertexBuffer);
		if (FAILED(result))
		{
			return false;
		}

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(unsigned long) * mIndexCount;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		indexData.pSysMem = indices;
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		//creating the index buffer
		result = device->CreateBuffer(&indexBufferDesc, &indexData, &mIndexBuffer);
		if (FAILED(result))
		{
			return false;
		}

		delete[] vertices;
		vertices = nullptr;

		delete[] indices;
		indices = nullptr;

		return true;
	}

	void Model::ShutdownBuffers()
	{
		if (mIndexBuffer)
		{
			mIndexBuffer->Release();
			mIndexBuffer = nullptr;
		}

		if (mVertexBuffer)
		{
			mVertexBuffer->Release();
			mVertexBuffer = nullptr;
		}
	}

	void Model::RenderBuffers(ID3D11DeviceContext* context)
	{
		unsigned int stride;
		unsigned int offset;

		stride = sizeof(VertexType);
		offset = 0;

		context->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	bool Model::LoadTexture(ID3D11Device* device, WCHAR* filename)
	{
		bool result;

		//setting up the texture object
		mTexture = new Texture;
		if (!mTexture)
		{
			return false;
		}

		result = mTexture->Initialize(device, filename);
		if (!result)
		{
			return false;
		}

		return true;
	}

	void Model::ReleaseTexture()
	{
		if (mTexture)
		{
			mTexture->Shutdown();
			delete mTexture;
			mTexture = nullptr;
		}
	}
}