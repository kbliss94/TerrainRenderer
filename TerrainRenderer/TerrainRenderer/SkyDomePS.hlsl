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

    if (height < -0.5)
    {
        outputColor = baseColor;
    }

    if (-0.5 <= height)
    {
        outputColor = lerp(centerColor, apexColor, height);
    }

    //// The value ranges from -1.0f to +1.0f so change it to only positive values.
    //if (height < 0.0)
    //{
    //    height = 0.0f;
    //}

    //// Determine the gradient color by interpolating between the apex and center based on the height of the pixel in the sky dome.
    //outputColor = lerp(centerColor, apexColor, height);

    return outputColor;
}
