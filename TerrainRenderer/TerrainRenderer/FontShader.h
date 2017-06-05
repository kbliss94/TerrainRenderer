#ifndef _FONTSHADER_
#define _FONTSHADER_

#pragma once

#include <d3d11.h>
#include <D3DX10math.h>
#include <D3DX11async.h>
#include <fstream>

using namespace std;

namespace TerrainRenderer
{
	class FontShader
	{
	public:
		//!Constructor
		FontShader();

		//!Copy constructor
		FontShader(const FontShader& rhs);

		//!Assignment operator
		FontShader& operator=(const FontShader& rhs);

		//!Destructor
		~FontShader();

		bool Initialize(ID3D11Device* device, HWND hwnd);
		void Shutdown();
		bool Render(ID3D11DeviceContext* context, int indexCount, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX projection, ID3D11ShaderResourceView* texture, D3DXVECTOR4 pixelColor);

	private:
		bool InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename);
		void ShutdownShader();
		void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename);

		bool SetShaderParameters(ID3D11DeviceContext* context, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX projection, ID3D11ShaderResourceView* texture, D3DXVECTOR4 pixelColor);
		void RenderShader(ID3D11DeviceContext* context, int indexCount);

		struct ConstantBufferType
		{
			D3DXMATRIX world;
			D3DXMATRIX view;
			D3DXMATRIX projection;
		};

		struct PixelBufferType
		{
			D3DXVECTOR4 pixelColor;
		};

		ID3D11VertexShader* mVertexShader;
		ID3D11PixelShader* mPixelShader;
		ID3D11InputLayout* mLayout;
		ID3D11Buffer* mConstantBuffer;
		ID3D11SamplerState* mSampleState;
		ID3D11Buffer* mPixelBuffer;
	};
}

#endif