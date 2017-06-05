#include "Bitmap.h"

namespace TerrainRenderer
{
	Bitmap::Bitmap():
		mVertexBuffer(nullptr), mIndexBuffer(nullptr), mTexture(nullptr)
	{

	}

	Bitmap::Bitmap(const Bitmap& rhs)
	{

	}

	Bitmap& Bitmap::operator=(const Bitmap& rhs)
	{
		return *this;
	}

	Bitmap::~Bitmap()
	{

	}

	bool Bitmap::Initialize(ID3D11Device* device, int screenWidth, int screenHeight, WCHAR* textureFilename, int bitmapWidth, int bitmapHeight)
	{
		bool result;

		mScreenWidth = screenWidth;
		mScreenHeight = screenHeight;
		mBitmapWidth = bitmapWidth;
		mBitmapHeight = bitmapHeight;
		mPreviousPosX = -1;
		mPreviousPosY = -1;

		//setting up the vertex & index buffers
		result = InitializeBuffers(device);
		if (!result)
		{
			return false;
		}

		result = LoadTexture(device, textureFilename);
		if (!result)
		{
			return false;
		}

		return true;
	}

	void Bitmap::Shutdown()
	{
		ReleaseTexture();
		ShutdownBuffers();
	}

	bool Bitmap::Render(ID3D11DeviceContext* context, int positionX, int positionY)
	{
		bool result;

		result = UpdateBuffers(context, positionX, positionY);
		if (!result)
		{
			return false;
		}

		RenderBuffers(context);

		return true;
	}

	int Bitmap::GetIndexCount()
	{
		return mIndexCount;
	}

	ID3D11ShaderResourceView* Bitmap::GetTexture()
	{
		return mTexture->GetTexture();
	}

	bool Bitmap::InitializeBuffers(ID3D11Device* device)
	{
		VertexType* vertices;
		unsigned long* indices;
		D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
		D3D11_SUBRESOURCE_DATA vertexData, indexData;
		HRESULT result;
		
		//6 vertices/indices because we're making a square out of two triangles
		mVertexCount = 6;
		mIndexCount = mVertexCount;

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

		//initializing vertex array to zeroes at first
		memset(vertices, 0, (sizeof(VertexType)*mVertexCount));

		//loading the index array with data
		for (int i = 0; i < mIndexCount; ++i)
		{
			indices[i] = i;
		}

		//creating a dynamic vertex buffer so we can modify the data inside the vertex buffer
		//each frame if needed
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.ByteWidth = sizeof(VertexType) *mVertexCount;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		vertexData.pSysMem = vertices;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &mVertexBuffer);
		if (FAILED(result))
		{
			return false;
		}

		//setting up the static index buffer
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(unsigned long) * mIndexCount;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		indexData.pSysMem = indices;
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

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

	void Bitmap::ShutdownBuffers()
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

	bool Bitmap::UpdateBuffers(ID3D11DeviceContext* context, int positionX, int positionY)
	{
		float left, right, top, bottom;
		VertexType* vertices;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		VertexType* verticesPtr;
		HRESULT result;

		//checking to see if the rendering position has changed
		if ((positionX == mPreviousPosX) && (positionY == mPreviousPosY))
		{
			return true;
		}

		mPreviousPosX = positionX;
		mPreviousPosY = positionY;

		//calculating the screen coordinates of the left side of the bitmap
		left = static_cast<float>((mScreenWidth / 2)* -1) + static_cast<float>(positionX);

		//calculating the screen coordinates of the right side of the bitmap
		right = left + static_cast<float>(mBitmapWidth);

		//calculating the screen coordinates of the top of the bitmap
		top = static_cast<float>(mScreenHeight / 2) - static_cast<float>(positionY);

		//calculating the screen coordinates of the bottom of the bitmap
		bottom = top - static_cast<float>(mBitmapHeight);

		//creating a temporary vertex array & filling it with the new six vertex points
		vertices = new VertexType[mVertexCount];
		if (!vertices)
		{
			return false;
		}

		//loading vertex array with first triangle data
		vertices[0].position = D3DXVECTOR3(left, top, 0.0f); //top left
		vertices[0].texture = D3DXVECTOR2(0.0f, 0.0f);

		vertices[1].position = D3DXVECTOR3(right, bottom, 0.0f); //bottom right
		vertices[1].texture = D3DXVECTOR2(1.0f, 1.0f);

		vertices[2].position = D3DXVECTOR3(left, bottom, 0.0f); //bottom left
		vertices[2].texture = D3DXVECTOR2(0.0f, 1.0f);

		//loading vertex array with second triangle data
		vertices[3].position = D3DXVECTOR3(left, top, 0.0f); //top left
		vertices[3].texture = D3DXVECTOR2(0.0f, 0.0f);

		vertices[4].position = D3DXVECTOR3(right, top, 0.0f); //top right
		vertices[4].texture = D3DXVECTOR2(1.0f, 0.0f);

		vertices[5].position = D3DXVECTOR3(right, bottom, 0.0f); //bottom right
		vertices[5].texture = D3DXVECTOR2(1.0f, 1.0f);

		//locking the vertex buffer so it can be written to
		result = context->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
		{
			return false;
		}

		verticesPtr = static_cast<VertexType*>(mappedResource.pData);
		memcpy(verticesPtr, static_cast<void*>(vertices), (sizeof(VertexType)*mVertexCount));

		//unlocking the vertex buffer
		context->Unmap(mVertexBuffer, 0);

		delete[] vertices;
		vertices = nullptr;

		return true;
	}

	void Bitmap::RenderBuffers(ID3D11DeviceContext* context)
	{
		unsigned int stride, offset;

		stride = sizeof(VertexType);
		offset = 0;

		//setting the vertex buffer to active in the input assembler so it can be rendered
		context->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);

		//setting the index buffer to active in the input assembler so it can be rendered
		context->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		//setting the type of primitive that should be rendered from this vertex buffer (triangles)
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	bool Bitmap::LoadTexture(ID3D11Device* device, WCHAR* textureFilename)
	{
		bool result;

		//setting up the texture object
		mTexture = new Texture;
		if (!mTexture)
		{
			return false;
		}

		result = mTexture->Initialize(device, textureFilename);
		if (!result)
		{
			return false;
		}

		return true;
	}

	void Bitmap::ReleaseTexture()
	{
		if (mTexture)
		{
			mTexture->Shutdown();
			delete mTexture;
			mTexture = nullptr;
		}
	}
}