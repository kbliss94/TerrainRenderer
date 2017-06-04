#ifndef _LIGHTSHADER_
#define _LIGHTSHADER_

#pragma once

#include <d3d11.h>
#include <D3DX10math.h>
#include <D3DX11async.h>
#include <fstream>

using namespace std;

namespace TerrainRenderer
{
	class LightShader
	{
	public:
		//!Constructor
		LightShader();

		//!Copy constructor
		LightShader(const LightShader& rhs);

		//!Assignment operator
		LightShader& operator=(const LightShader& rhs);

		//!Destructor
		~LightShader();

		bool Initialize(ID3D11Device* device, HWND hwnd);
		void Shutdown();
		bool Render(ID3D11DeviceContext* context, int indexCount, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX projection, ID3D11ShaderResourceView* texture, D3DXVECTOR3 lightDirection, D3DXVECTOR4 diffuseColor);

	private:
		bool InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename);
		void ShutdownShader();
		void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename);

		bool SetShaderParameters(ID3D11DeviceContext* context, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX projection, ID3D11ShaderResourceView* texture, D3DXVECTOR3 lightDirection, D3DXVECTOR4 diffuseColor);
		void RenderShader(ID3D11DeviceContext* context, int indexCount);

		struct MatrixBufferType
		{
			D3DXMATRIX world;
			D3DXMATRIX view;
			D3DXMATRIX projection;
		};

		struct LightBufferType
		{
			D3DXVECTOR4 diffuseColor;
			D3DXVECTOR3 lightDirection;
			//extra padding added so struct is a multiple of 16 for CreateBuffer function requirements
			float padding; 
		};

		ID3D11VertexShader* mVertexShader;
		ID3D11PixelShader* mPixelShader;
		ID3D11InputLayout* mLayout;
		ID3D11Buffer* mMatrixBuffer;
		ID3D11SamplerState* mSampleState;
		ID3D11Buffer* mLightBuffer;
	};
}

#endif
