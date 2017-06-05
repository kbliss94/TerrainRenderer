float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

//globals
Texture2D shaderTexture;
SamplerState SampleType;

cbuffer PixelBuffer
{
    float4 pixelColor;
};

//typedefs
struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 tex : TEXCOORD0;
};

//pixel shader
float4 FontPixelShader(PixelInputType input) : SV_TARGET
{
    float4 color;

    //sampling the texture pixel at this location
    color = shaderTexture.Sample(SampleType, input.tex);

    //if the color is black on the texture then treat this pixel as transparent
    if (color.r == 0.0f)
    {
        color.a = 0.0f;
    }
    else
    {
        color.a = 1.0f;
        color = color * pixelColor;
    }

    return color;
}