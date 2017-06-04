#include "ColorShader.h"

namespace TerrainRenderer
{
	ColorShader::ColorShader():
		mVertexShader(nullptr), mPixelShader(nullptr), mLayout(nullptr), mMatrixBuffer(nullptr)
	{

	}

	ColorShader::ColorShader(const ColorShader& rhs)
	{

	}

	ColorShader& ColorShader::operator=(const ColorShader& rhs)
	{
		return *this;
	}

	ColorShader::~ColorShader()
	{

	}

	bool ColorShader::Initialize(ID3D11Device* device, HWND hwnd)
	{
		return InitializeShader(device, hwnd, L"../TerrainRenderer/ColorVS.hlsl", L"../TerrainRenderer/ColorPS.hlsl");
	}

	void ColorShader::Shutdown()
	{
		ShutdownShader();
	}

	bool ColorShader::Render(ID3D11DeviceContext* context, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix)
	{
		bool result;

		result = SetShaderParameters(context, worldMatrix, viewMatrix, projectionMatrix);
		if (!result)
		{
			return false;
		}

		RenderShader(context, indexCount);
		return true;
	}

	bool ColorShader::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
	{
		HRESULT result;
		ID3D10Blob* errorMessage = nullptr;
		ID3D10Blob* vertexShaderBuffer = nullptr;
		ID3D10Blob* pixelShaderBuffer = nullptr;
		D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
		unsigned int numElements;
		D3D11_BUFFER_DESC matrixBufferDesc;

		//compiling vertex shader program into buffer
		result = D3DX11CompileFromFile(vsFilename, nullptr, nullptr, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &vertexShaderBuffer, 
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
		result = D3DX11CompileFromFile(psFilename, nullptr, nullptr, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pixelShaderBuffer, 
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

		polygonLayout[1].SemanticName = "COLOR";
		polygonLayout[1].SemanticIndex = 0;
		polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
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

		return true;
	}

	void ColorShader::ShutdownShader()
	{
		if (mMatrixBuffer)
		{
			mMatrixBuffer->Release();
			mMatrixBuffer = nullptr;
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

	void ColorShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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

	bool ColorShader::SetShaderParameters(ID3D11DeviceContext* context, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix)
	{
		HRESULT result;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		MatrixBufferType* dataPtr;
		unsigned int bufferNumber;

		//transposing matrices before sending them into the shader (requirement for DirectX 11)
		D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
		D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
		D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

		//locking the matrix buffer, setting the new matrices inside of it, & unlocking it
		result = context->Map(mMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
		{
			return false;
		}

		dataPtr = static_cast<MatrixBufferType*>(mappedResource.pData);

		dataPtr->world = worldMatrix;
		dataPtr->view = viewMatrix;
		dataPtr->projection = projectionMatrix;

		context->Unmap(mMatrixBuffer, 0);
		bufferNumber = 0;

		context->VSSetConstantBuffers(bufferNumber, 1, &mMatrixBuffer);
		return true;
	}

	void ColorShader::RenderShader(ID3D11DeviceContext* context, int indexCount)
	{
		context->IASetInputLayout(mLayout);
		context->VSSetShader(mVertexShader, nullptr, 0);
		context->PSSetShader(mPixelShader, nullptr, 0);

		context->DrawIndexed(indexCount, 0, 0);
	}
}