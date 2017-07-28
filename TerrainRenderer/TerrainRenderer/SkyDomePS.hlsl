float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

//globals
cbuffer GradientBuffer
{
    float4 apexColor;
    float4 centerColor;
    float4 baseColor;
};


//typedefs
struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 domePosition : TEXCOORD0;
};

float4 SkyDomePixelShader(PixelInputType input) : SV_TARGET
{
    float height;
    float4 outputColor;


    // Determine the position on the sky dome where this pixel is located.
    height = input.domePosition.y;

    if (height < -0.4)
    {
        outputColor = baseColor;
    }

    if (-0.4 <= height && height < 0.0)
    {
        outputColor = centerColor;
    }

    if (0.0 <= height)
    {
        outputColor = lerp(centerColor, apexColor, height);
    }

    return outputColor;
}
