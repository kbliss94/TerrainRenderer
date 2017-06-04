#include "Texture.h"

namespace TerrainRenderer
{
	Texture::Texture():
		mTexture(nullptr)
	{

	}

	Texture::Texture(const Texture& rhs)
	{

	}

	Texture& Texture::operator=(const Texture& rhs)
	{
		return *this;
	}

	Texture::~Texture()
	{

	}

	bool Texture::Initialize(ID3D11Device* device, WCHAR* filename)
	{
		HRESULT result;

		result = D3DX11CreateShaderResourceViewFromFile(device, filename, nullptr, nullptr, &mTexture, nullptr);
		if (FAILED(result))
		{
			return false;
		}

		return true;
	}

	void Texture::Shutdown()
	{
		if (mTexture)
		{
			mTexture->Release();
			mTexture = nullptr;
		}
	}

	ID3D11ShaderResourceView* Texture::GetTexture()
	{
		return mTexture;
	}
}