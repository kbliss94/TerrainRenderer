#include "LightShader.h"

namespace TerrainRenderer
{
	LightShader::LightShader() :
		mVertexShader(nullptr), mPixelShader(nullptr), mLayout(nullptr), mMatrixBuffer(nullptr), mSampleState(nullptr), mLightBuffer(nullptr)
	{

	}

	LightShader::LightShader(const LightShader& rhs)
	{

	}

	LightShader& LightShader::operator=(const LightShader& rhs)
	{
		return *this;
	}

	LightShader::~LightShader()
	{

	}

	bool LightShader::Initialize(ID3D11Device* device, HWND hwnd)
	{
		bool result;

		//setting up the vertex & pixel shaders
		result = InitializeShader(device, hwnd, L"../TerrainRenderer/LightVS.hlsl", L"../TerrainRenderer/LightPS.hlsl");
		if (!result)
		{
			return false;
		}

		return true;
	}

	void LightShader::Shutdown()
	{
		ShutdownShader();
	}

	bool LightShader::Render(ID3D11DeviceContext* context, int indexCount, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX projection, ID3D11ShaderResourceView* texture, D3DXVECTOR3 lightDirection, D3DXVECTOR4 diffuseColor)
	{
		bool result;

		//setting up the shader parameters that will be used for rendering
		result = SetShaderParameters(context, world, view, projection, texture, lightDirection, diffuseColor);
		if (!result)
		{
			return false;
		}

		//rendering the prepared buffers with the shader
		RenderShader(context, indexCount);

		return true;
	}

	bool LightShader::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
	{
		HRESULT result;
		ID3D10Blob* errorMessage = nullptr;
		ID3D10Blob* vertexShaderBuffer = nullptr;
		ID3D10Blob* pixelShaderBuffer = nullptr;
		D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
		unsigned int numElements;
		D3D11_BUFFER_DESC matrixBufferDesc;
		D3D11_SAMPLER_DESC samplerDesc;
		D3D11_BUFFER_DESC lightBufferDesc;

		//compiling vertex shader program into buffer
		result = D3DX11CompileFromFile(vsFilename, nullptr, nullptr, "LightVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &vertexShaderBuffer,
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
		result = D3DX11CompileFromFile(psFilename, nullptr, nullptr, "LightPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pixelShaderBuffer,
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

		polygonLayout[2].SemanticName = "NORMAL";
		polygonLayout[2].SemanticIndex = 0;
		polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		polygonLayout[2].InputSlot = 0;
		polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[2].InstanceDataStepRate = 0;

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

		//setting up the constant buffer (buffer usage is dynamic bc we will be updating it each frame)
		matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
		matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		matrixBufferDesc.MiscFlags = 0;
		matrixBufferDesc.StructureByteStride = 0;

		result = device->CreateBuffer(&matrixBufferDesc, nullptr, &mMatrixBuffer);
		if (FAILED(result))
		{
			return false;
		}

		//setting up the light dynamic constant buffer that's in the pixel shader
		//ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail
		lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		lightBufferDesc.ByteWidth = sizeof(LightBufferType);
		lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		lightBufferDesc.MiscFlags = 0;
		lightBufferDesc.StructureByteStride = 0;

		result = device->CreateBuffer(&lightBufferDesc, nullptr, &mLightBuffer);
		if (FAILED(result))
		{
			return false;
		}

		return true;
	}

	void LightShader::ShutdownShader()
	{
		if (mLightBuffer)
		{
			mLightBuffer->Release();
			mLightBuffer = nullptr;
		}

		if (mMatrixBuffer)
		{
			mMatrixBuffer->Release();
			mMatrixBuffer = nullptr;
		}

		if (mSampleState)
		{
			mSampleState->Release();
			mSampleState = nullptr;
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

	void LightShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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

	bool LightShader::SetShaderParameters(ID3D11DeviceContext* context, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX projection, ID3D11ShaderResourceView* texture, D3DXVECTOR3 lightDirection, D3DXVECTOR4 diffuseColor)
	{
		HRESULT result;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		MatrixBufferType* dataPtr;
		LightBufferType* dataPtr2;
		unsigned int bufferNumber;

		D3DXMatrixTranspose(&world, &world);
		D3DXMatrixTranspose(&view, &view);
		D3DXMatrixTranspose(&projection, &projection);

		//locking the constant buffer so that it can be written to
		result = context->Map(mMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
		{
			return false;
		}

		//getting a pointer to the data in the constant buffer
		dataPtr = static_cast<MatrixBufferType*>(mappedResource.pData);

		//copying the matrices into the constant buffer
		dataPtr->world = world;
		dataPtr->view = view;
		dataPtr->projection = projection;

		//unlocking the constant buffer
		context->Unmap(mMatrixBuffer, 0);

		bufferNumber = 0;

		//setting the constant buffer in the vertex shader with the updated values
		context->VSSetConstantBuffers(bufferNumber, 1, &mMatrixBuffer);

		//setting shader texture resource in the pixel shader
		context->PSSetShaderResources(0, 1, &texture);

		//locking the light constant buffer so it can be written to
		result = context->Map(mLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
		{
			return false;
		}

		//getting a pointer to the data in the constant buffer
		dataPtr2 = static_cast<LightBufferType*>(mappedResource.pData);

		dataPtr2->diffuseColor = diffuseColor;
		dataPtr2->lightDirection = lightDirection;
		dataPtr2->padding = 0.0f;

		//unlocking the constant buffer
		context->Unmap(mLightBuffer, 0);

		bufferNumber = 0;

		//setting the light constant buffer in the pixel shader with the updated values
		context->PSSetConstantBuffers(bufferNumber, 1, &mLightBuffer);

		return true;
	}

	void LightShader::RenderShader(ID3D11DeviceContext* context, int indexCount)
	{
		context->IASetInputLayout(mLayout);
		context->VSSetShader(mVertexShader, nullptr, 0);
		context->PSSetShader(mPixelShader, nullptr, 0);
		context->PSSetSamplers(0, 1, &mSampleState);

		//drawing the triangle
		context->DrawIndexed(indexCount, 0, 0);
	}
}