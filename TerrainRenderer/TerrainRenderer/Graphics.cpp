#include "Graphics.h"

namespace TerrainRenderer
{
	Graphics::Graphics()
	{

	}

	Graphics::Graphics(const Graphics& rhs)
	{

	}

	Graphics& Graphics::operator=(const Graphics& rhs)
	{
		return *this;
	}

	Graphics::~Graphics()
	{

	}

	bool Graphics::Initialize(int width, int height, HWND hwnd)
	{
		return true;
	}

	void Graphics::Shutdown()
	{

	}

	bool Graphics::Frame()
	{
		return true;
	}

	bool Graphics::Render()
	{
		return true;
	}
}