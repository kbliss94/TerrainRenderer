#ifndef _TERRAIN_
#define _TERRAIN_

#include <d3d11.h>
#include <d3dx10math.h>
#include <stdio.h>

namespace TerrainRenderer
{
	class Terrain
	{
	private:
		struct VertexType
		{
			D3DXVECTOR3 position;
			D3DXVECTOR4 color;
		};

		struct HeightMapType
		{
			float x, y, z;
		};

	public:
		//!Constructor
		Terrain();

		//!Copy constructor
		Terrain(const Terrain& rhs);

		//!Assignment operator
		Terrain& operator=(const Terrain& rhs);

		//!Destructor
		~Terrain();

		bool Initialize(ID3D11Device* device, char* heightMapFilename);
		void Shutdown();
		void Render(ID3D11DeviceContext* context);

		int GetIndexCount();

	private:
		bool LoadHeightMap(char* filename);
		void NormalizeHeightMap();
		void ShutdownHeightMap();

		bool InitializeBuffers(ID3D11Device* device);
		void ShutdownBuffers();
		void RenderBuffers(ID3D11DeviceContext* context);

	private:
		int mTerrainWidth, mTerrainHeight;
		int mVertexCount, mIndexCount;
		ID3D11Buffer *mVertexBuffer, *mIndexBuffer;
		HeightMapType* mHeightMap;

		const float mVertexColorR = 1.0f;
		const float mVertexColorG = 1.0f;
		const float mVertexColorB = 1.0f;
		const float mVertexColorAlpha = 1.0f;
	};
}

#endif
