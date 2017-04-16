

struct VertexInput
{
	float4 position		: POSITION;
	float4 color		: COLOR;
};

struct VertexOutput
{
	float4 position		: SV_POSITION;
	float4 color		: COLOR;
};

VertexOutput ColorVertexShader(VertexInput input)
{
	VertexOutput output;	// TODO init to 0


	// Change the position vector to be 4 units for proper matrix calculations.
	//input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	/*output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);*/


	output.position = input.position;

	output.color = input.color;

	return output;
}

//RWBuffer<float4> buffer : register(u1);
//RasterizerOrderedBuffer<float4> buffer : register(u1);

float4 ColorPixelShader(VertexOutput input) : SV_TARGET
{
	//int index = input.position.x + input.position.y * 1280;

	//float4 data = buffer[index];
	//buffer[index] = data + float4(0.1, 0.5, 1.0, 2.0);
	//buffer[index] = data + float4(0.1, 0.5, 1.0, asin(asin(asin(asin(asin(asin(asin(asin(asin(asin(asin(asin(asin(asin(asin(asin(asin(0.1 + 0.001*data.x))))))))))))))))));

	return input.color;
}


float4 ClearPixelShader(VertexOutput input) : SV_TARGET
{
	//int index = input.position.x + input.position.y * 1280;
	//buffer[index] = float4(0.0, 0.0, 0.0, 0.0);

	return float4(0.0, 0.0, 0.0, 0.0);
}


float4 FinalPixelShader(VertexOutput input) : SV_TARGET
{
	//int index = input.position.x + input.position.y * 1280;
	//float4 data = buffer[index];

	//return float4(index / (1280.0*720.0), data.x, 0, 0);
	//return data*0.02;

	return float4(0.1, 0.2, 0.4, 1.0);
}