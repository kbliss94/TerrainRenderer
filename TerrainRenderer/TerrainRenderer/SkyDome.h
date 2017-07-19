#pragma once

#include <d3d11.h>
#include <d3dx10math.h>
#include <fstream>

using namespace std;

namespace TerrainRenderer
{
	class SkyDome
	{
	private:
		struct ModelType
		{
			float x, y, z;
			float tu, tv;
			float nx, ny, nz;
		};

		struct VertexType
		{
			D3DXVECTOR3 position;
		};

	public:
		SkyDome();
		SkyDome(const SkyDome& rhs);
		~SkyDome();

		bool Initialize(ID3D11Device* device);
		void Shutdown();
		void Render(ID3D11DeviceContext* context);

		int GetIndexCount();
		D3DXVECTOR4 GetApexColor();
		D3DXVECTOR4 GetCenterColor();

	private:
		bool LoadSkyDomeModel(char*);
		void ReleaseSkyDomeModel();

		bool InitializeBuffers(ID3D11Device*);
		void ReleaseBuffers();
		void RenderBuffers(ID3D11DeviceContext*);

	private:
		ModelType* mModel;
		int mVertexCount, mIndexCount;
		ID3D11Buffer *mVertexBuffer, *mIndexBuffer;
		D3DXVECTOR4 mApexColor, mCenterColor;
	};
}


