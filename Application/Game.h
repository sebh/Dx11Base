#pragma once

#include "Dx11Base/WindowInput.h"
#include "Dx11Base/Dx11Device.h"

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
	/// @exitIfFail: calls exit(0) if any of the reload/compilation failed.
	void loadShaders(bool exitIfFail);
	/// release all shaders
	void releaseShaders();

	// hack for testing
	RenderBuffer* vertexBuffer;
	RenderBuffer* indexBuffer;

	//
	// Testing some GPU buffers and shaders 
	//

	struct ConstantBufferStructureExemple
	{
		float f;
		int i;
		uint u;
		float f2;
	};
	typedef ConstantBuffer<ConstantBufferStructureExemple> MyConstantBuffer;
	MyConstantBuffer* mConstantBuffer;

	RenderBuffer* mSomeBuffer;
	ID3D11UnorderedAccessView* mSomeBufferUavView;

	VertexShader* mVertexShader;
	PixelShader*  mPixelShader;
	PixelShader*  mPixelShaderClear;
	PixelShader*  mPixelShaderFinal;

	ID3D11InputLayout* mLayout;

	struct VertexType
	{
		float position[3];
		float color[4];
	};
};


