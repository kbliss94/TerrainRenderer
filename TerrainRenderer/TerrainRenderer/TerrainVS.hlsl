float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}

//globals
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    //for fogging
    float3 cameraPosition;
    float fogStart;
    float3 padding;
    float fogRange;
};

//typedefs
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    //for fogging
    float3 worldPosition : WORLDPOS;
    float fogAmount : FOG;
};

//vertex shader
PixelInputType TerrainVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Store the texture coordinates for the pixel shader.
    output.tex = input.tex;

    // Calculate the normal vector against the world matrix only.
    output.normal = mul(input.normal, (float3x3) worldMatrix);
	
    // Normalize the normal vector.
    output.normal = normalize(output.normal);

    //setting the world position
    output.worldPosition = mul(input.position, worldMatrix).xyz;

    //calculating the amount of fog
    output.fogAmount = saturate((distance(cameraPosition, output.worldPosition) - fogStart) / (fogRange));

    return output;
}
