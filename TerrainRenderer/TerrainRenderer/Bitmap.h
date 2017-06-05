#ifndef _BITMAP_
#define _BITMAP_

#pragma once

#include <d3d11.h>
#include <D3DX10math.h>
#include "Texture.h"

namespace TerrainRenderer
{
	class Bitmap
	{
	public:
		//!Constructor
		Bitmap();

		//!Copy constructor
		Bitmap(const Bitmap& rhs);

		//!Assignment operator
		Bitmap& operator=(const Bitmap& rhs);

		//!Destructor
		~Bitmap();

		bool Initialize(ID3D11Device* device, int screenWidth, int screenHeight, WCHAR* textureFilename, int bitmapWidth, int bitmapHeight);
		void Shutdown();
		bool Render(ID3D11DeviceContext* context, int positionX, int positionY);

		int GetIndexCount();
		ID3D11ShaderResourceView* GetTexture();

	private:
		bool InitializeBuffers(ID3D11Device* device);
		void ShutdownBuffers();
		bool UpdateBuffers(ID3D11DeviceContext* context, int positionX, int positionY);
		void RenderBuffers(ID3D11DeviceContext* context);

		bool LoadTexture(ID3D11Device* device, WCHAR* textureFilename);
		void ReleaseTexture();

		struct VertexType
		{
			D3DXVECTOR3 position;
			D3DXVECTOR2 texture;
		};

		ID3D11Buffer* mVertexBuffer, *mIndexBuffer;
		int mVertexCount, mIndexCount;
		Texture* mTexture;
		int mScreenWidth, mScreenHeight;
		int mBitmapWidth, mBitmapHeight;
		int mPreviousPosX, mPreviousPosY;
	};
}

#endif
