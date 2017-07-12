float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

//globals
Texture2D grassTexture : register(t0);
Texture2D slopeTexture : register(t1);
Texture2D rockTexture : register(t2);

SamplerState SampleType;

cbuffer LightBuffer
{
    float4 ambientColor;
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
float4 TerrainPixelShader(PixelInputType input) : SV_TARGET
{
    float4 grassColor;
    float4 slopeColor;
    float4 rockColor;
    float blendAmount;
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color;

    // Sample the grass color from the texture using the sampler at this texture coordinate location.
    grassColor = grassTexture.Sample(SampleType, input.tex);

    // Sample the slope color from the texture using the sampler at this texture coordinate location.
    slopeColor = slopeTexture.Sample(SampleType, input.tex);

    // Sample the rock color from the texture using the sampler at this texture coordinate location.
    rockColor = rockTexture.Sample(SampleType, input.tex);

    //calculating the slope for this pixel/point
    float slope = 1.0f - input.normal.y;

    //using the slope to determine which texture to use
        //doing a linear interpolation b/w textures so the transition b/w each one isn't a sharp line in the terrain
    if (slope < 0.2)
    {
        blendAmount = slope / 0.2f;
        textureColor = lerp(grassColor, slopeColor, blendAmount);
    }
	
    if ((slope < 0.7) && (slope >= 0.2f))
    {
        blendAmount = (slope - 0.2f) * (1.0f / (0.7f - 0.2f));
        textureColor = lerp(slopeColor, rockColor, blendAmount);
    }

    if (slope >= 0.7)
    {
        textureColor = rockColor;
    }

    // Invert the light direction for calculations.
    lightDir = -lightDirection;

    // Calculate the amount of light on this pixel.
    lightIntensity = saturate(dot(input.normal, lightDir));
	
    // Determine the final diffuse color based on the diffuse color and the amount of light intensity.
    color = diffuseColor * lightIntensity;

    // Saturate the final light color.
    color = saturate(color);

    // Multiply the texture color and the final light color to get the result.
    color = color * textureColor;

    return color;
}