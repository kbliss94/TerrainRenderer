#include "TerrainShader.h"

namespace TerrainRenderer
{
	TerrainShader::TerrainShader():
		mVertexShader(nullptr), mPixelShader(nullptr), mLayout(nullptr), mSampleState(nullptr), mMatrixBuffer(nullptr), mLightBuffer(nullptr)
	{

	}

	TerrainShader::TerrainShader(const TerrainShader& rhs)
	{

	}

	TerrainShader::~TerrainShader()
	{

	}

	bool TerrainShader::Initialize(ID3D11Device* device, HWND hwnd)
	{
		bool result;

		// Initialize the vertex and pixel shaders.
		result = InitializeShader(device, hwnd, L"../TerrainRenderer/TerrainVS.hlsl", L"../TerrainRenderer/TerrainPS.hlsl");
		if (!result)
		{
			return false;
		}

		return true;
	}

	void TerrainShader::Shutdown()
	{
		// Shutdown the vertex and pixel shaders as well as the related objects.
		ShutdownShader();

		return;
	}
	
	bool TerrainShader::Render(ID3D11DeviceContext* context, int indexCount, D3DXMATRIX world, D3DXMATRIX view,
			D3DXMATRIX projection, D3DXVECTOR4 ambientColor, D3DXVECTOR4 diffuseColor, D3DXVECTOR3 lightDirection, D3DXVECTOR3 cameraPosition, D3DXVECTOR3 fogColor,
			ID3D11ShaderResourceView* grassTexture, ID3D11ShaderResourceView* slopeTexture, ID3D11ShaderResourceView* rockTexture)
	{
		bool result;

		// Set the shader parameters that it will use for rendering.
		result = SetShaderParameters(context, world, view, projection, ambientColor, diffuseColor, lightDirection, cameraPosition, fogColor, 
			grassTexture, slopeTexture, rockTexture);
		if (!result)
		{
			return false;
		}

		// Now render the prepared buffers with the shader.
		RenderShader(context, indexCount);

		return true;
	}

	bool TerrainShader::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
	{
		HRESULT result;
		ID3D10Blob* errorMessage;
		ID3D10Blob* vertexShaderBuffer;
		ID3D10Blob* pixelShaderBuffer;

		D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
		unsigned int numElements;
		D3D11_SAMPLER_DESC samplerDesc;
		D3D11_BUFFER_DESC matrixBufferDesc;
		D3D11_BUFFER_DESC lightBufferDesc;


		// Initialize the pointers this function will use to null.
		errorMessage = 0;
		vertexShaderBuffer = 0;
		pixelShaderBuffer = 0;

		// Compile the vertex shader code.
		result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "TerrainVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
			&vertexShaderBuffer, &errorMessage, NULL);
		if (FAILED(result))
		{
			// If the shader failed to compile it should have written something to the error message.
			if (errorMessage)
			{
				OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
			}
			// If there was nothing in the error message then it simply could not find the shader file itself.
			else
			{
				MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
			}

			return false;
		}

		// Compile the pixel shader code.
		result = D3DX11CompileFromFile(psFilename, NULL, NULL, "TerrainPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
			&pixelShaderBuffer, &errorMessage, NULL);
		if (FAILED(result))
		{
			// If the shader failed to compile it should have written something to the error message.
			if (errorMessage)
			{
				OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
			}
			// If there was nothing in the error message then it simply could not find the file itself.
			else
			{
				MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
			}

			return false;
		}

		// Create the vertex shader from the buffer.
		result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &mVertexShader);
		if (FAILED(result))
		{
			return false;
		}

		// Create the pixel shader from the buffer.
		result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &mPixelShader);
		if (FAILED(result))
		{
			return false;
		}

		// Create the vertex input layout description.
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

		// Get a count of the elements in the layout.
		numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

		// Create the vertex input layout.
		result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),
			&mLayout);
		if (FAILED(result))
		{
			return false;
		}

		// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
		vertexShaderBuffer->Release();
		vertexShaderBuffer = 0;

		pixelShaderBuffer->Release();
		pixelShaderBuffer = 0;

		// Create a texture sampler state description.
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

		// Create the texture sampler state.
		result = device->CreateSamplerState(&samplerDesc, &mSampleState);
		if (FAILED(result))
		{
			return false;
		}

		// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
		matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
		matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		matrixBufferDesc.MiscFlags = 0;
		matrixBufferDesc.StructureByteStride = 0;

		// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
		result = device->CreateBuffer(&matrixBufferDesc, NULL, &mMatrixBuffer);
		if (FAILED(result))
		{
			return false;
		}

		// Setup the description of the light dynamic constant buffer that is in the pixel shader.
		// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
		lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		lightBufferDesc.ByteWidth = sizeof(LightBufferType);
		lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		lightBufferDesc.MiscFlags = 0;
		lightBufferDesc.StructureByteStride = 0;

		// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
		result = device->CreateBuffer(&lightBufferDesc, NULL, &mLightBuffer);
		if (FAILED(result))
		{
			return false;
		}

		return true;
	}


	void TerrainShader::ShutdownShader()
	{
		// Release the light constant buffer.
		if (mLightBuffer)
		{
			mLightBuffer->Release();
			mLightBuffer = 0;
		}

		// Release the matrix constant buffer.
		if (mMatrixBuffer)
		{
			mMatrixBuffer->Release();
			mMatrixBuffer = 0;
		}

		// Release the sampler state.
		if (mSampleState)
		{
			mSampleState->Release();
			mSampleState = 0;
		}

		// Release the layout.
		if (mLayout)
		{
			mLayout->Release();
			mLayout = 0;
		}

		// Release the pixel shader.
		if (mPixelShader)
		{
			mPixelShader->Release();
			mPixelShader = 0;
		}

		// Release the vertex shader.
		if (mVertexShader)
		{
			mVertexShader->Release();
			mVertexShader = 0;
		}
	}


	void TerrainShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
	{
		char* compileErrors;
		unsigned long bufferSize, i;
		ofstream fout;

		// Get a pointer to the error message text buffer.
		compileErrors = (char*)(errorMessage->GetBufferPointer());

		// Get the length of the message.
		bufferSize = errorMessage->GetBufferSize();

		// Open a file to write the error message to.
		fout.open("shader-error.txt");

		// Write out the error message.
		for (i = 0; i < bufferSize; i++)
		{
			fout << compileErrors[i];
		}

		// Close the file.
		fout.close();

		// Release the error message.
		errorMessage->Release();
		errorMessage = 0;

		// Pop a message up on the screen to notify the user to check the text file for compile errors.
		MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);
	}


	bool TerrainShader::SetShaderParameters(ID3D11DeviceContext* context, D3DXMATRIX world, D3DXMATRIX view,
			D3DXMATRIX projection, D3DXVECTOR4 ambientColor, D3DXVECTOR4 diffuseColor, D3DXVECTOR3 lightDirection, D3DXVECTOR3 cameraPosition, D3DXVECTOR3 fogColor,
			ID3D11ShaderResourceView* grassTexture, ID3D11ShaderResourceView* slopeTexture, ID3D11ShaderResourceView* rockTexture)
	{
		HRESULT result;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		unsigned int bufferNumber;
		MatrixBufferType* dataPtr;
		LightBufferType* dataPtr2;

		if (!FOG_ENABLED)
		{
			fogColor = D3DXVECTOR3(0.0, 0.0, 0.0);
		}

		// Transpose the matrices to prepare them for the shader.
		D3DXMatrixTranspose(&world, &world);
		D3DXMatrixTranspose(&view, &view);
		D3DXMatrixTranspose(&projection, &projection);

		// Lock the constant buffer so it can be written to.
		result = context->Map(mMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
		{
			return false;
		}

		// Get a pointer to the data in the constant buffer.
		dataPtr = (MatrixBufferType*)mappedResource.pData;

		// Copy the matrices into the constant buffer.
		dataPtr->world = world;
		dataPtr->view = view;
		dataPtr->projection = projection;
		dataPtr->cameraPosition = cameraPosition;
		dataPtr->fogStart = mFogStart;
		dataPtr->fogRange = mFogRange;

		// Unlock the constant buffer.
		context->Unmap(mMatrixBuffer, 0);

		// Set the position of the constant buffer in the vertex shader.
		bufferNumber = 0;

		// Now set the constant buffer in the vertex shader with the updated values.
		context->VSSetConstantBuffers(bufferNumber, 1, &mMatrixBuffer);

		// Lock the light constant buffer so it can be written to.
		result = context->Map(mLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
		{
			return false;
		}

		// Get a pointer to the data in the constant buffer.
		dataPtr2 = (LightBufferType*)mappedResource.pData;

		// Copy the lighting variables into the constant buffer.
		dataPtr2->ambientColor = ambientColor;
		dataPtr2->diffuseColor = diffuseColor;
		dataPtr2->lightDirection = lightDirection;
		dataPtr2->padding = 0.0f;
		dataPtr2->cameraPosition = cameraPosition;
		dataPtr2->padding2 = 0.0f;
		dataPtr2->fogColor = fogColor;

		// Unlock the constant buffer.
		context->Unmap(mLightBuffer, 0);

		// Set the position of the light constant buffer in the pixel shader.
		bufferNumber = 0;

		// Finally set the light constant buffer in the pixel shader with the updated values.
		context->PSSetConstantBuffers(bufferNumber, 1, &mLightBuffer);

		// Set shader texture resource in the pixel shader.
			//setting terrain texture in the pixel shader here
		context->PSSetShaderResources(0, 1, &grassTexture);
		context->PSSetShaderResources(1, 1, &slopeTexture);
		context->PSSetShaderResources(2, 1, &rockTexture);

		return true;
	}


	void TerrainShader::RenderShader(ID3D11DeviceContext* context, int indexCount)
	{
		// Set the vertex input layout.
		context->IASetInputLayout(mLayout);

		// Set the vertex and pixel shaders that will be used to render this triangle.
		context->VSSetShader(mVertexShader, NULL, 0);
		context->PSSetShader(mPixelShader, NULL, 0);

		// Set the sampler state in the pixel shader.
		context->PSSetSamplers(0, 1, &mSampleState);

		// Render the triangle.
		context->DrawIndexed(indexCount, 0, 0);
	}
}