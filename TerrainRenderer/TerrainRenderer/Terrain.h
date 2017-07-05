#ifndef _TERRAIN_
#define _TERRAIN_

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
			D3DXVECTOR4 color;
		};

		struct HeightMapData
		{
			float x, y, z;

			HeightMapData() {};

			HeightMapData(float ix, float iy, float iz):
				x(ix), y(iy), z(iz)
			{
				
			};

			template<class Archive>
			void serialize(Archive& archive)
			{
				archive(x, y, z);
			};
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

		bool Initialize(ID3D11Device* device, char* heightMapFilename, char* scalingFilename, int xOffset = 0, int zOffset = 0);
		void Shutdown();
		void Render(ID3D11DeviceContext* context);

		int GetIndexCount();
		void UpdatePosition(int xUpdate, int zUpdate);

		//will need to add a scaling filename as a param
		void UpdateHeightMap(char* heightMapFilename);

		int GetGridPositionX();
		int GetGridPositionY();
		void SetGridPosition(int x, int y);

		//!Replaces current data with deserialized data without destroying other data
		/*
		Replaces: mHeightMap, mScalingMap, mGridPositionX, mGridPositionY
		*/
		void SetHeightMapInfo(std::shared_ptr<Terrain> terrain);

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(mHeightMap, mScalingMap, mGridPositionX, mGridPositionY);
		};

	private:
		bool LoadHeightMap(char* filename);
		bool LoadScalingMap(char* filename);
		void NormalizeHeightMap();
		void NormalizeScalingMap();
		void ShutdownHeightMap();

		bool InitializeBuffers(ID3D11Device* device);
		void ShutdownBuffers();
		void RenderBuffers(ID3D11DeviceContext* context);

	private:
		int mTerrainWidth, mTerrainHeight;
		int mVertexCount, mIndexCount;
		ID3D11Buffer *mVertexBuffer, *mIndexBuffer;
		vector<HeightMapData> mHeightMap;

		const float mVertexColorR = 1.0f;
		const float mVertexColorG = 1.0f;
		const float mVertexColorB = 1.0f;
		const float mVertexColorAlpha = 1.0f;

		int mXOffset;
		int mZOffset;
		int mGridPositionX;
		int mGridPositionY;

		ID3D11Device* mDevice;

		const float mHeightScaling = 0.5f;
		char* mHeightScalingMap;
		vector<HeightMapData> mScalingMap;

		friend class cereal::access;
	};
}

#endif
