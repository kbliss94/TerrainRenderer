#ifndef _TERRAINSHADER_
#define _TERRAINSHADER_

#pragma once

#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>

using namespace std;

namespace TerrainRenderer
{
	class TerrainShader
	{
	private:
		struct MatrixBufferType
		{
			D3DXMATRIX world;
			D3DXMATRIX view;
			D3DXMATRIX projection;
		};

		struct LightBufferType
		{
			D3DXVECTOR4 ambientColor;
			D3DXVECTOR4 diffuseColor;
			D3DXVECTOR3 lightDirection;
			float padding;
		};

	public:
		TerrainShader();
		TerrainShader(const TerrainShader& rhs);
		~TerrainShader();

		bool Initialize(ID3D11Device* device, HWND hwnd);
		void Shutdown();
		bool Render(ID3D11DeviceContext* context, int indexCount, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX projection, D3DXVECTOR4 ambientColor, 
			D3DXVECTOR4 diffuseColor, D3DXVECTOR3 lightDirection, ID3D11ShaderResourceView* grassTexture, ID3D11ShaderResourceView* slopeTexture,
			ID3D11ShaderResourceView* rockTexture);

	private:
		bool InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename);
		void ShutdownShader();
		void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename);

		bool SetShaderParameters(ID3D11DeviceContext* context, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX projection, D3DXVECTOR4 ambientColor,
			D3DXVECTOR4 diffuseColor, D3DXVECTOR3 lightDirection, ID3D11ShaderResourceView* grassTexture, ID3D11ShaderResourceView* slopeTexture, 
			ID3D11ShaderResourceView* rockTexture);
		void RenderShader(ID3D11DeviceContext* context, int indexCount);

	private:
		ID3D11VertexShader* mVertexShader;
		ID3D11PixelShader* mPixelShader;
		ID3D11InputLayout* mLayout;
		ID3D11SamplerState* mSampleState;
		ID3D11Buffer* mMatrixBuffer;
		ID3D11Buffer* mLightBuffer;
	};
}

#endif