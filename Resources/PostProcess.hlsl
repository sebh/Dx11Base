


#include "./Resources/Common.hlsl"



float sRGB(float x)
{
	if (x <= 0.00031308)
		return 12.92 * x;
	else
		return 1.055*pow(x, (1.0 / 2.4)) - 0.055;
}

float4 sRGB(float4 vec)
{
	return float4(sRGB(vec.x), sRGB(vec.y), sRGB(vec.z), vec.w);
}



float4 PostProcessPS(VertexOutput input) : SV_TARGET
{
	uint2 texCoord = input.position.xy;
	float4 rgbA = texture2d.Load(uint3(texCoord,0));
	return sRGB(rgbA);
}



