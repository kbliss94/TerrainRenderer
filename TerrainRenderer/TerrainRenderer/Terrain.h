#ifndef _TERRAIN_
#define _TERRAIN_

#include <d3d11.h>
#include <d3dx10math.h>

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

	public:
		Terrain();
		Terrain(const Terrain&);
		~Terrain();

		bool Initialize(ID3D11Device*);
		void Shutdown();
		void Render(ID3D11DeviceContext*);

		int GetIndexCount();

	private:
		bool InitializeBuffers(ID3D11Device*);
		void ShutdownBuffers();
		void RenderBuffers(ID3D11DeviceContext*);

	private:
		int m_terrainWidth, m_terrainHeight;
		int m_vertexCount, m_indexCount;
		ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	};
}

#endif
