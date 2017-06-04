float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

//!Pixel shader
float4 ColorPixelShader(PixelInputType input) : SV_TARGET
{
	return input.color;
}