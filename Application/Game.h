#pragma once

#include "Dx11Base/WindowInput.h"
#include "Dx11Base/Dx11Device.h"



class Game
{
public:
	Game();
	~Game();

	void initialise();
	void reallocateResolutionDepedent(uint32 newWidth, uint32 newHeight);
	void shutdown();

	void update(const WindowInputData& inputData);
	void render();

private:

	/// Load/reload all shaders if compilation is succesful.
	/// @firstTimeLoadShaders: calls exit(0) if any of the reload/compilation failed.
	void loadShaders(bool firstTimeLoadShaders);
	/// release all shaders
	void releaseShaders();

	void allocateResolutionIndependentResources();
	void releaseResolutionIndependentResources();
	void allocateResolutionDependentResources(uint32 newWidth, uint32 newHeight);
	void releaseResolutionDependentResources();

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
	D3dUnorderedAccessView* mSomeBufferUavView;

	DepthStencilState* mDefaultDepthStencilState;
	BlendState* mDefaultBlendState;
	RasterizerState* mDefaultRasterizerState;

	VertexShader* mVertexShader;
	VertexShader* mScreenVertexShader;
	PixelShader*  mColoredTrianglesShader;
	ComputeShader*mToyShader;
	PixelShader*  mPostProcessShader;

	D3dInputLayout* mLayout;

	Texture2D* mBackBufferHdr;
	Texture2D* mBackBufferDepth;

};


