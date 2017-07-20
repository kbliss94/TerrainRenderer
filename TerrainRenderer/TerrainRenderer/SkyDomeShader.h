#pragma once

#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>

using namespace std;

namespace TerrainRenderer
{
	class SkyDomeShader
	{
	private:
		struct MatrixBufferType
		{
			D3DXMATRIX world;
			D3DXMATRIX view;
			D3DXMATRIX projection;
		};

		struct GradientBufferType
		{
			D3DXVECTOR4 apexColor;
			D3DXVECTOR4 centerColor;
			D3DXVECTOR4 baseColor;
		};

	public:
		SkyDomeShader();
		SkyDomeShader(const SkyDomeShader& rhs);
		~SkyDomeShader();

		bool Initialize(ID3D11Device*, HWND);
		void Shutdown();
		bool Render(ID3D11DeviceContext*, int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, D3DXVECTOR4, D3DXVECTOR4, D3DXVECTOR4);

	private:
		bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
		void ShutdownShader();
		void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

		bool SetShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, D3DXVECTOR4, D3DXVECTOR4, D3DXVECTOR4);
		void RenderShader(ID3D11DeviceContext*, int);

	private:
		ID3D11VertexShader* mVertexShader;
		ID3D11PixelShader* mPixelShader;
		ID3D11InputLayout* mLayout;
		ID3D11Buffer* mMatrixBuffer;
		ID3D11Buffer* mGradientBuffer;
	};
}