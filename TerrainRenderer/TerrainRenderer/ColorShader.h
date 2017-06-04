#ifndef _COLORSHADER_
#define _COLORSHADER_

#pragma once

#include <d3d11.h>
#include <D3DX10math.h>
#include <D3DX11async.h>
#include <fstream>

using namespace std;

namespace TerrainRenderer
{
	class ColorShader
	{
	public:
		//!Constructor
		ColorShader();

		//!Copy constructor
		ColorShader(const ColorShader& rhs);

		//!Assignment operator
		ColorShader& operator=(const ColorShader& rhs);

		//!Destructor
		~ColorShader();

		bool Initialize(ID3D11Device* device, HWND hwnd);
		void Shutdown();
		bool Render(ID3D11DeviceContext* context, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix);

	private:
		//!Loads the shader files & makes them usable to DirectX & the GPU
		bool InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename);

		//!Releases the four interfaces that were set up in InitializeShader
		void ShutdownShader();
		void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename);
		bool SetShaderParameters(ID3D11DeviceContext* context, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix);

		//!Draws a green triangle using ColorPS & ColorVS
		void RenderShader(ID3D11DeviceContext* context, int indexCount);

		struct MatrixBufferType
		{
			D3DXMATRIX world;
			D3DXMATRIX view;
			D3DXMATRIX projection;
		};

		ID3D11VertexShader* mVertexShader;
		ID3D11PixelShader* mPixelShader;
		ID3D11InputLayout* mLayout;
		ID3D11Buffer* mMatrixBuffer;
	};
}

#endif 
