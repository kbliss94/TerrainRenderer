#ifndef _MODEL_
#define _MODEL_

#pragma once

#include <d3d11.h>
#include <D3DX10math.h>

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

		bool Initialize(ID3D11Device* device);
		void Shutdown();
		void Render(ID3D11DeviceContext* context);
		int GetIndexCount();

	private:
		//!Creates the vertex & index buffers
		bool InitializeBuffers(ID3D11Device* device);
		void ShutdownBuffers();
		void RenderBuffers(ID3D11DeviceContext* context);

		struct VertexType
		{
			D3DXVECTOR3 position;
			D3DXVECTOR4 color;
		};

		ID3D11Buffer* mVertexBuffer, *mIndexBuffer;
		int mVertexCount, mIndexCount;
	};
}

#endif