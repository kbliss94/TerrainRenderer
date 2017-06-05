#ifndef _FONT_
#define _FONT_

#pragma once

#include <d3d11.h>
#include <D3DX10math.h>
#include <fstream>
#include "Texture.h"

using namespace std;

namespace TerrainRenderer
{
	class Font
	{
	public:
		//!Constructor
		Font();

		//!Copy constructor
		Font(const Font& rhs);

		//!Assignment operator
		Font& operator=(const Font& rhs);

		//!Destructor
		~Font();

		bool Initialize(ID3D11Device* device, char* fontFilename, WCHAR* textureFilename);
		void Shutdown();

		ID3D11ShaderResourceView* GetTexture();

		//!Will build & return a vertex array of triangles; will be called by Text
		void BuildVertexArray(void* vertices, char* sentence, float drawX, float drawY);

	private:
		bool LoadFontData(char* filename);
		void ReleaseFontData();
		bool LoadTexture(ID3D11Device* device, WCHAR* filename);
		void ReleaseTexture();

		struct FontType
		{
			float left, right;
			int size;
		};

		struct VertexType
		{
			D3DXVECTOR3 position;
			D3DXVECTOR2 texture;
		};

		FontType* mFont;
		Texture* mTexture;
		const int mNumCharsInTexture = 95;
	};
}

#endif