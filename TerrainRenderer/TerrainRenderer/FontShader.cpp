#include "FontShader.h"

namespace TerrainRenderer
{
	FontShader::FontShader() :
		mVertexShader(nullptr), mPixelShader(nullptr), mLayout(nullptr), mConstantBuffer(nullptr), mSampleState(nullptr)
	{

	}

	FontShader::FontShader(const FontShader& rhs)
	{

	}

	FontShader& FontShader::operator=(const FontShader& rhs)
	{
		return *this;
	}

	FontShader::~FontShader()
	{

	}

	bool FontShader::Initialize(ID3D11Device* device, HWND hwnd)
	{
		bool result;

		//setting up the vertex & pixel shaders
		result = InitializeShader(device, hwnd, L"../TerrainRenderer/FontVS.hlsl", L"../TerrainRenderer/FontPS.hlsl");
		if (!result)
		{
			return false;
		}

		return true;
	}

	void FontShader::Shutdown()
	{
		ShutdownShader();
	}

	bool FontShader::Render(ID3D11DeviceContext* context, int indexCount, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX projection, ID3D11ShaderResourceView* texture, D3DXVECTOR4 pixelColor)
	{
		bool result;

		//setting up the shader parameters that will be used for rendering
		result = SetShaderParameters(context, world, view, projection, texture, pixelColor);
		if (!result)
		{
			return false;
		}

		//rendering the prepared buffers with the shader
		RenderShader(context, indexCount);

		return true;
	}

	bool FontShader::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
	{
		HRESULT result;
		ID3D10Blob* errorMessage = nullptr;
		ID3D10Blob* vertexShaderBuffer = nullptr;
		ID3D10Blob* pixelShaderBuffer = nullptr;
		D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
		unsigned int numElements;
		D3D11_BUFFER_DESC constantBufferDesc;
		D3D11_SAMPLER_DESC samplerDesc;
		D3D11_BUFFER_DESC pixelBufferDesc;

		//compiling vertex shader program into buffer
		result = D3DX11CompileFromFile(vsFilename, nullptr, nullptr, "FontVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &vertexShaderBuffer,
			&errorMessage, nullptr);
		if (FAILED(result))
		{
			if (errorMessage)
			{
				OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
			}
			else
			{
				MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
			}

			return false;
		}

		//compiling pixel shader program into buffer
		result = D3DX11CompileFromFile(psFilename, nullptr, nullptr, "FontPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pixelShaderBuffer,
			&errorMessage, nullptr);
		if (FAILED(result))
		{
			if (errorMessage)
			{
				OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
			}
			else
			{
				MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
			}

			return false;
		}

		//creating vertex shader object from the buffer
		result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), nullptr, &mVertexShader);
		if (FAILED(result))
		{
			return false;
		}

		//creating pixel shader object from the buffer
		result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &mPixelShader);
		if (FAILED(result))
		{
			return false;
		}

		//creating the layout of the vertex data (this setup needs to match the VertexType struct in Model & the shader)
		//AlignedByteOffset: indicates how the data is spaced in the buffer; shows where each element starts
		polygonLayout[0].SemanticName = "POSITION";
		polygonLayout[0].SemanticIndex = 0;
		polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		polygonLayout[0].InputSlot = 0;
		polygonLayout[0].AlignedByteOffset = 0;
		polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[0].InstanceDataStepRate = 0;

		polygonLayout[1].SemanticName = "TEXCOORD";
		polygonLayout[1].SemanticIndex = 0;
		polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		polygonLayout[1].InputSlot = 0;
		polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[1].InstanceDataStepRate = 0;

		//creating the input layout using the D3D device
		numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

		result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &mLayout);
		if (FAILED(result))
		{
			return false;
		}

		vertexShaderBuffer->Release();
		vertexShaderBuffer = nullptr;

		pixelShaderBuffer->Release();
		pixelShaderBuffer = nullptr;

		//setting up the constant buffer (buffer usage is dynamic bc we will be updating it each frame)
		constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		constantBufferDesc.ByteWidth = sizeof(ConstantBufferType);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		constantBufferDesc.MiscFlags = 0;
		constantBufferDesc.StructureByteStride = 0;

		result = device->CreateBuffer(&constantBufferDesc, nullptr, &mConstantBuffer);
		if (FAILED(result))
		{
			return false;
		}

		//setting up texture sampler state description
		//AddressU/V/W set to Wrap, ensuring the coordinates stay between 0.0f & 1.0f
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		result = device->CreateSamplerState(&samplerDesc, &mSampleState);
		if (FAILED(result))
		{
			return false;
		}
		
		//setting up the pixel color constant buffer
		pixelBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		pixelBufferDesc.ByteWidth = sizeof(PixelBufferType);
		pixelBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		pixelBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pixelBufferDesc.MiscFlags = 0;
		pixelBufferDesc.StructureByteStride = 0;

		result = device->CreateBuffer(&pixelBufferDesc, nullptr, &mPixelBuffer);
		if (FAILED(result))
		{
			return false;
		}

		return true;
	}

	void FontShader::ShutdownShader()
	{
		if (mPixelBuffer)
		{
			mPixelBuffer->Release();
			mPixelBuffer = nullptr;
		}

		if (mSampleState)
		{
			mSampleState->Release();
			mSampleState = nullptr;
		}

		if (mConstantBuffer)
		{
			mConstantBuffer->Release();
			mConstantBuffer = nullptr;
		}

		if (mLayout)
		{
			mLayout->Release();
			mLayout = nullptr;
		}

		if (mPixelShader)
		{
			mPixelShader->Release();
			mPixelShader = nullptr;
		}

		if (mVertexShader)
		{
			mVertexShader->Release();
			mVertexShader = nullptr;
		}
	}

	void FontShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
	{
		char* compileErrors;
		unsigned long bufferSize;
		ofstream fout;

		compileErrors = static_cast<char*>(errorMessage->GetBufferPointer());
		bufferSize = errorMessage->GetBufferSize();
		fout.open("shader-error.txt");

		for (unsigned long i = 0; i < bufferSize; ++i)
		{
			fout << compileErrors[i];
		}

		fout.close();

		errorMessage->Release();
		errorMessage = nullptr;

		MessageBox(hwnd, L"Error compiling shader. Check shader-error.txt for message.", shaderFilename, MB_OK);
	}

	bool FontShader::SetShaderParameters(ID3D11DeviceContext* context, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX projection, ID3D11ShaderResourceView* texture, D3DXVECTOR4 pixelColor)
	{
		HRESULT result;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ConstantBufferType* dataPtr;
		unsigned int bufferNumber;
		PixelBufferType* dataPtr2;

		//locking the constant buffer so that it can be written to
		result = context->Map(mConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
		{
			return false;
		}

		//getting a pointer to the data in the constant buffer
		dataPtr = static_cast<ConstantBufferType*>(mappedResource.pData);

		D3DXMatrixTranspose(&world, &world);
		D3DXMatrixTranspose(&view, &view);
		D3DXMatrixTranspose(&projection, &projection);

		//copying the matrices into the constant buffer
		dataPtr->world = world;
		dataPtr->view = view;
		dataPtr->projection = projection;

		//unlocking the constant buffer
		context->Unmap(mConstantBuffer, 0);

		bufferNumber = 0;

		//setting the constant buffer in the vertex shader with the updated values
		context->VSSetConstantBuffers(bufferNumber, 1, &mConstantBuffer);

		//setting shader texture resource in the pixel shader
		context->PSSetShaderResources(0, 1, &texture);

		//setting the pixel color before rendering
		result = context->Map(mPixelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
		{
			return false;
		}

		dataPtr2 = static_cast<PixelBufferType*>(mappedResource.pData);
		dataPtr2->pixelColor = pixelColor;

		context->Unmap(mPixelBuffer, 0);

		bufferNumber = 0;
		
		context->PSSetConstantBuffers(bufferNumber, 1, &mPixelBuffer);

		return true;
	}

	void FontShader::RenderShader(ID3D11DeviceContext* context, int indexCount)
	{
		context->IASetInputLayout(mLayout);
		context->VSSetShader(mVertexShader, nullptr, 0);
		context->PSSetShader(mPixelShader, nullptr, 0);
		context->PSSetSamplers(0, 1, &mSampleState);

		//drawing the triangle
		context->DrawIndexed(indexCount, 0, 0);
	}
}