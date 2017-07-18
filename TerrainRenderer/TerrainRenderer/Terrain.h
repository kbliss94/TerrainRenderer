#ifndef _TERRAIN_
#define _TERRAIN_

//globals
const int TEXTURE_REPEAT = 32;

#include "Texture.h"

#include <d3d11.h>
#include <d3dx10math.h>
#include <stdio.h>
#include <memory>
#include <vector>
#include <cereal/access.hpp>
#include <cereal/archives/xml.hpp>

using namespace std;

namespace TerrainRenderer
{
	class Terrain
	{
	public:
	//private:
		struct VertexType
		{
			D3DXVECTOR3 position;
			D3DXVECTOR2 texture;
			D3DXVECTOR3 normal;
		};

		struct HeightMapData
		{
			float x, y, z;
			float tu, tv;
			float nx, ny, nz;

			HeightMapData() {};

			HeightMapData(float ix, float iy, float iz, float itu, float itv, float inx, float iny, float inz):
				x(ix), y(iy), z(iz), tu(itu), tv(itv), nx(inx), ny(iny), nz(inz)
			{
				
			};

			template<class Archive>
			void serialize(Archive& archive)
			{
				archive(x, y, z, tu, tv, nx, ny, nz);
			};
		};

		struct VectorType
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

		bool Initialize(ID3D11Device* device, char* heightMapFilename, WCHAR* grassTextureFilename, WCHAR* slopeTextureFilename,
			WCHAR* rockTextureFilename, int xOffset = 0, int zOffset = 0);
		void Shutdown();
		void Render(ID3D11DeviceContext* context);

		int GetIndexCount();
		void UpdatePosition(int xUpdate, int zUpdate);

		void UpdateHeightMap();

		int GetGridPositionX();
		int GetGridPositionY();
		void SetGridPosition(int x, int y);

		char* GetHeightMapFilename();

		vector<HeightMapData>& GetHeightMap();

		//!Replaces current data with deserialized data without destroying other data
		/*
		Replaces: mHeightMap, mGridPositionX, mGridPositionY
		*/
		void SetHeightMapInfo(std::shared_ptr<Terrain> terrain);

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(mHeightMap, mGridPositionX, mGridPositionY);
		};

		ID3D11ShaderResourceView* GetGrassTexture();
		ID3D11ShaderResourceView* GetSlopeTexture();
		ID3D11ShaderResourceView* GetRockTexture();

	private:
		bool LoadHeightMap(char* filename);
		void NormalizeHeightMap();
		void ShutdownHeightMap();

		bool CalculateNormals();
		void CalculateTextureCoordinates();
		bool LoadTextures(ID3D11Device* device, WCHAR* grassFilename, WCHAR* slopeFilename, WCHAR* rockFilename);
		void ReleaseTextures();

		bool InitializeBuffers(ID3D11Device* device);
		void ShutdownBuffers();
		void RenderBuffers(ID3D11DeviceContext* context);

	private:
		int mTerrainWidth, mTerrainHeight;
		int mVertexCount, mIndexCount;
		int mXOffset;
		int mZOffset;
		int mGridPositionX;
		int mGridPositionY;

		ID3D11Device* mDevice;
		ID3D11Buffer *mVertexBuffer, *mIndexBuffer;
		vector<HeightMapData> mHeightMap;
		char* mHeightMapFilename;

		const float mVertexColorR = 1.0f;
		const float mVertexColorG = 1.0f;
		const float mVertexColorB = 1.0f;
		const float mVertexColorAlpha = 1.0f;

		Texture* mGrassTexture, *mSlopeTexture, *mRockTexture;

		friend class cereal::access;
	};
}

#endif
