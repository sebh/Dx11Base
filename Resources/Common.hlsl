


cbuffer CONSTANT_BUFFER : register(b0)
{
	float4x4 gViewProjMat;

	float4 gColor;

	uint2 gResolution;
	uint2 pad;
};

SamplerState samplerLinearClamp							: register(s0);

Texture2D<float4>  texture2d							: register(t0);

RWTexture2D<float4> rwTexture2d							: register(u0);
//RWBuffer<float4> buffer								: register(u0);
//RasterizerOrderedBuffer<float4> buffer				: register(u0);



////////////////////////////////////////////////////////////////////////////////////////////////////



struct VertexInput
{
	float4 position		: POSITION;
};

struct VertexOutput
{
	float4 position		: SV_POSITION;
};



VertexOutput DefaultVertexShader(VertexInput input)
{
	VertexOutput output = (VertexOutput)0;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, gViewProjMat);

	return output;
}



VertexOutput ScreenTriangleVertexShader(uint vertexId : SV_VertexID)
{
	VertexOutput output = (VertexOutput)0;

	// For a range on screen in [-0.5,0.5]
	float2 uv = -1.0f;
	uv = vertexId == 1 ? float2(-1.0f, 3.0f) : uv;
	uv = vertexId == 2 ? float2( 3.0f,-1.0f) : uv;
	output.position = float4(uv, 0.0f, 1.0f);

	return output;
}


