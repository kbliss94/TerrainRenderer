#ifndef _MODEL_
#define _MODEL_

#pragma once

#include <d3d11.h>
#include <D3DX10math.h>
#include "Texture.h"

using namespace std;

namespace TerrainRenderer
{
	class Model
	{
	public:
		//!Constructor
		Model();

		//!Copy constructor
		Model(const Model& rhs);

		//!Assignment operator
		Model& operator=(const Model& rhs);

		//!Destructor
		~Model();

		bool Initialize(ID3D11Device* device, WCHAR* filename);
		void Shutdown();
		void Render(ID3D11DeviceContext* context);

		int GetIndexCount();
		ID3D11ShaderResourceView* GetTexture();


	private:
		//!Creates the vertex & index buffers
		bool InitializeBuffers(ID3D11Device* device);
		void ShutdownBuffers();
		void RenderBuffers(ID3D11DeviceContext* context);

		bool LoadTexture(ID3D11Device* device, WCHAR* filename);
		void ReleaseTexture();

		struct VertexType
		{
			D3DXVECTOR3 position;
			D3DXVECTOR2 texture;
			D3DXVECTOR3 normal;
		};

		ID3D11Buffer* mVertexBuffer, *mIndexBuffer;
		int mVertexCount, mIndexCount;
		Texture* mTexture;
	};
}

#endif