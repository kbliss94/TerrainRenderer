#ifndef _TEXTURE_
#define _TEXTURE_

#pragma once

#include <d3d11.h>
#include <D3DX11tex.h>

namespace TerrainRenderer
{
	class Texture
	{
	public:
		//!Constructor
		Texture();

		//!Copy constructor 
		Texture(const Texture& rhs);

		//!Assignment operator
		Texture& operator=(const Texture& rhs);

		//!Destructor
		~Texture();

		bool Initialize(ID3D11Device* device, WCHAR* filename);
		void Shutdown();

		ID3D11ShaderResourceView* GetTexture();

	private:
		ID3D11ShaderResourceView* mTexture;
	};
}

#endif
