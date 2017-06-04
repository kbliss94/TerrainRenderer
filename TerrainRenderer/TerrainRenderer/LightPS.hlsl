float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

//globals
Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer
{
    float4 diffuseColor;
    float3 lightDirection;
    float padding;
};

//typedefs
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

//pixel shader
float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color;

    //sampling the pixel color from the texture using the sampler at this texture coordinate location
    textureColor = shaderTexture.Sample(SampleType, input.tex);

    //light intensity value is calculated as the dot product between the normal vector 
    //of the triangle & the light direction vector

    //inverting the light direction for calculations
    lightDir = -lightDirection;

    //calculating the amount of light on this pixel
    lightIntensity = saturate(dot(input.normal, lightDir));

    //determine the final amount of diffuse color based on the diffuse color combined w/the light intensity
    color = saturate(diffuseColor * lightIntensity);

    //multiplying the texture pixel & the final diffuse color to get the final pixel color result
    color = color * textureColor;

    return color;
}