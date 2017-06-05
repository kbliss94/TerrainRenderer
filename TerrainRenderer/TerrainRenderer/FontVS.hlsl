float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}

//globals
cbuffer PerFrameBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

//typedefs
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

//vertex shader
PixelInputType FontVertexShader(VertexInputType input)
{
    PixelInputType output;

    //changing the position vector to be 4 units for proper matrix calculation
    input.position.w = 1.0f;

    //calculating the position of the vertex against the world/view/projection matrices
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    //storing the texture coordinates for the pixel shader
    output.tex = input.tex;

    return output;
}