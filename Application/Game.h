#pragma once

#include "Dx11Base/WindowInput.h"
#include "Dx11Base/Dx11Device.h"


// TODO put in its own file CustomDirectXMath file
#include "DirectXMath.h"
using namespace DirectX;
typedef XMMATRIX float4x4;
typedef XMVECTOR Vector4;
typedef XMFLOAT4 float4;
typedef XMFLOAT3 float3;
#define CLAMP(x, x0, x1) (x < x0 ? x0 : (x > x1 ? x1 : x))



class Game
{
public:
	Game();
	~Game();

	void initialise();
	void shutdown();

	void update(const WindowInputData& inputData);
	void render();

private:

	/// Load/reload all shaders if compilation is succesful.
	/// @firstTimeLoadShaders: calls exit(0) if any of the reload/compilation failed.
	void loadShaders(bool firstTimeLoadShaders);
	/// release all shaders
	void releaseShaders();

	// Test vertex buffer
	struct VertexType
	{
		float position[3];
	};
	RenderBuffer* vertexBuffer;
	RenderBuffer* indexBuffer;

	//
	// Testing some GPU buffers and shaders 
	//

	struct CommonConstantBufferStructure
	{
		float4x4 gViewProjMat;

		float4 gColor;

		unsigned int gResolution[2];
		unsigned int pad;
	};
	typedef ConstantBuffer<CommonConstantBufferStructure> CommonConstantBuffer;
	CommonConstantBuffer* mConstantBuffer;

	RenderBuffer* mSomeBuffer;
	UnorderedAccessView* mSomeBufferUavView;

	DepthStencilState* mDefaultDepthStencilState;
	BlendState* mDefaultBlendState;
	RasterizerState* mDefaultRasterizerState;

	VertexShader* mVertexShader;
	VertexShader* mScreenVertexShader;
	PixelShader*  mColoredTrianglesShader;
	ComputeShader*mToyShader;
	PixelShader*  mPostProcessShader;

	InputLayout* mLayout;

	Texture2D* mBackBufferHdr;
	Texture2D* mBackBufferDepth;

};


