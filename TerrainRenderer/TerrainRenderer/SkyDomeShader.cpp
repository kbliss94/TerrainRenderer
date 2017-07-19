#include "SkyDomeShader.h"

namespace TerrainRenderer
{
	SkyDomeShader::SkyDomeShader()
	{
		mVertexShader = 0;
		mPixelShader = 0;
		mLayout = 0;
		mMatrixBuffer = 0;
		mGradientBuffer = 0;
	}


	SkyDomeShader::SkyDomeShader(const SkyDomeShader& rhs)
	{
	}


	SkyDomeShader::~SkyDomeShader()
	{
	}


	bool SkyDomeShader::Initialize(ID3D11Device* device, HWND hwnd)
	{
		bool result;


		// Initialize the vertex and pixel shaders.
		result = InitializeShader(device, hwnd, L"../TerrainRenderer/SkyDomeVS.hlsl", L"../TerrainRenderer/SkyDomePS.hlsl");
		if (!result)
		{
			return false;
		}

		return true;
	}


	void SkyDomeShader::Shutdown()
	{
		// Shutdown the vertex and pixel shaders as well as the related objects.
		ShutdownShader();

		return;
	}


	bool SkyDomeShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
		D3DXMATRIX projectionMatrix, D3DXVECTOR4 apexColor, D3DXVECTOR4 centerColor)
	{
		bool result;


		// Set the shader parameters that it will use for rendering.
		result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, apexColor, centerColor);
		if (!result)
		{
			return false;
		}

		// Now render the prepared buffers with the shader.
		RenderShader(deviceContext, indexCount);

		return true;
	}


	bool SkyDomeShader::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
	{
		HRESULT result;
		ID3D10Blob* errorMessage;
		ID3D10Blob* vertexShaderBuffer;
		ID3D10Blob* pixelShaderBuffer;
		D3D11_INPUT_ELEMENT_DESC polygonLayout[1];
		unsigned int numElements;
		D3D11_BUFFER_DESC matrixBufferDesc;
		D3D11_BUFFER_DESC gradientBufferDesc;


		// Initialize the pointers this function will use to null.
		errorMessage = 0;
		vertexShaderBuffer = 0;
		pixelShaderBuffer = 0;

		// Compile the vertex shader code.
		result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "SkyDomeVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
			&vertexShaderBuffer, &errorMessage, NULL);
		if (FAILED(result))
		{
			// If the shader failed to compile it should have written something to the error message.
			if (errorMessage)
			{
				OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
			}
			// If there was  nothing in the error message then it simply could not find the shader file itself.
			else
			{
				MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
			}

			return false;
		}

		// Compile the pixel shader code.
		result = D3DX11CompileFromFile(psFilename, NULL, NULL, "SkyDomePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
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

		// Setup the description of the gradient constant buffer that is in the pixel shader.
		gradientBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		gradientBufferDesc.ByteWidth = sizeof(GradientBufferType);
		gradientBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		gradientBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		gradientBufferDesc.MiscFlags = 0;
		gradientBufferDesc.StructureByteStride = 0;

		// Create the constant buffer pointer so we can access the pixel shader constant buffer from within this class.
		result = device->CreateBuffer(&gradientBufferDesc, NULL, &mGradientBuffer);
		if (FAILED(result))
		{
			return false;
		}

		return true;
	}


	void SkyDomeShader::ShutdownShader()
	{
		// Release the gradient constant buffer.
		if (mGradientBuffer)
		{
			mGradientBuffer->Release();
			mGradientBuffer = 0;
		}

		// Release the matrix constant buffer.
		if (mMatrixBuffer)
		{
			mMatrixBuffer->Release();
			mMatrixBuffer = 0;
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

		return;
	}


	void SkyDomeShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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

		return;
	}


	bool SkyDomeShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
		D3DXMATRIX projectionMatrix, D3DXVECTOR4 apexColor, D3DXVECTOR4 centerColor)
	{
		HRESULT result;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		MatrixBufferType* dataPtr;
		GradientBufferType* dataPtr2;
		unsigned int bufferNumber;


		// Transpose the matrices to prepare them for the shader.
		D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
		D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
		D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

		// Lock the constant buffer so it can be written to.
		result = deviceContext->Map(mMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
		{
			return false;
		}

		// Get a pointer to the data in the constant buffer.
		dataPtr = (MatrixBufferType*)mappedResource.pData;

		// Copy the matrices into the constant buffer.
		dataPtr->world = worldMatrix;
		dataPtr->view = viewMatrix;
		dataPtr->projection = projectionMatrix;

		// Unlock the constant buffer.
		deviceContext->Unmap(mMatrixBuffer, 0);

		// Set the position of the constant buffer in the vertex shader.
		bufferNumber = 0;

		// Finally set the constant buffer in the vertex shader with the updated values.
		deviceContext->VSSetConstantBuffers(bufferNumber, 1, &mMatrixBuffer);

		// Lock the gradient constant buffer so it can be written to.
		result = deviceContext->Map(mGradientBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
		{
			return false;
		}

		// Get a pointer to the data in the constant buffer.
		dataPtr2 = (GradientBufferType*)mappedResource.pData;

		// Copy the gradient color variables into the constant buffer.
		dataPtr2->apexColor = apexColor;
		dataPtr2->centerColor = centerColor;

		// Unlock the constant buffer.
		deviceContext->Unmap(mGradientBuffer, 0);

		// Set the position of the gradient constant buffer in the pixel shader.
		bufferNumber = 0;

		// Finally set the gradient constant buffer in the pixel shader with the updated values.
		deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mGradientBuffer);

		return true;
	}


	void SkyDomeShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
	{
		// Set the vertex input layout.
		deviceContext->IASetInputLayout(mLayout);

		// Set the vertex and pixel shaders that will be used to render the triangles.
		deviceContext->VSSetShader(mVertexShader, NULL, 0);
		deviceContext->PSSetShader(mPixelShader, NULL, 0);

		// Render the triangle.
		deviceContext->DrawIndexed(indexCount, 0, 0);
	}
}