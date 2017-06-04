float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

//globals
//texture resource
Texture2D shaderTexture;
//allows modification of how the pixels are written to the polygon face when shaded
SamplerState SampleType;

//typedefs
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

//pixel shader
float4 TexturePixelShader(PixelInputType input) : SV_TARGET
{
    float4 textureColor;

	//sample the pixel color from the texture using the sampler at this texture coordinate location
    textureColor = shaderTexture.Sample(SampleType, input.tex);

    return textureColor;
}
