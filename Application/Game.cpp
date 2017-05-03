
#include "Game.h"
#include "Dx11Base/Dx11Device.h"

#include "windows.h"
#include "DirectXMath.h"

#include <imgui.h>


// hack for testing
RenderBuffer* vertexBuffer;
RenderBuffer* indexBuffer;
RenderBuffer* constantBuffer;

RenderBuffer* someBuffer;
ID3D11UnorderedAccessView* someBufferUavView;

VertexShader* vertexShader;
PixelShader*  pixelShader;
PixelShader*  pixelShaderClear;
PixelShader*  pixelShaderFinal;

ID3D11InputLayout* layout;

struct VertexType
{
	float position[3];
	float color[4];
};


Game::Game()
{
	ID3D11Device* device = g_dx11Device->getDevice();
	//ID3D11DeviceContext* context = g_dx11Device->getDeviceContext();

	// Simple triangle geometry
	VertexType vertices[12];
	vertices[0] = { { -10.0f, -1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } };
	vertices[1] = { { 10.0f,  10.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } };
	vertices[2] = { { 10.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } };
	vertices[3] = { { -10.0f, -1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } };
	vertices[4] = { { 10.0f,  10.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } };
	vertices[5] = { { 10.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } };
	vertices[6] = { { -10.0f, -1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } };
	vertices[7] = { { 10.0f,  10.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } };
	vertices[8] = { { 10.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } };
	vertices[9] = { { -10.0f, -1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } };
	vertices[10] = { { 10.0f,  10.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } };
	vertices[11] = { { 10.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } };
	UINT indices[12];
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 3;
	indices[4] = 4;
	indices[5] = 5;
	indices[6] = 6;
	indices[7] = 7;
	indices[8] = 8;
	indices[9] = 9;
	indices[10] = 10;
	indices[11] = 11;


	// Create 
	D3D11_BUFFER_DESC vertexBufferDesc;
	RenderBuffer::initVertexBufferDesc_default(vertexBufferDesc, sizeof(vertices));
	vertexBuffer = new RenderBuffer(vertexBufferDesc, vertices);

	D3D11_BUFFER_DESC indexBufferDesc;
	RenderBuffer::initIndexBufferDesc_default(indexBufferDesc, sizeof(indices));
	indexBuffer = new RenderBuffer(indexBufferDesc, indices);

	D3D11_BUFFER_DESC constantBufferDesc;
	RenderBuffer::initConstantBufferDesc_dynamic(constantBufferDesc, 128);
	constantBuffer = new RenderBuffer(constantBufferDesc);

	vertexShader = new VertexShader(L"Resources\\TestShader.hlsl", "ColorVertexShader");
	pixelShader  = new PixelShader (L"Resources\\TestShader.hlsl", "ColorPixelShader");
	pixelShaderClear  = new PixelShader (L"Resources\\TestShader.hlsl", "ClearPixelShader");
	pixelShaderFinal  = new PixelShader (L"Resources\\TestShader.hlsl", "FinalPixelShader");

	InputLayoutDescriptors inputLayout;
	appendSimpleVertexDataToInputLayout(inputLayout, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	appendSimpleVertexDataToInputLayout(inputLayout, "COLOR"   , DXGI_FORMAT_R32G32B32A32_FLOAT);
	vertexShader->createInputLayout(inputLayout, &layout);	// Have a layout object with vertex stride in it

	UINT bufferElementSize = (sizeof(float) * 4);
	UINT bufferElementCount = 1280 * 720;
	D3D11_BUFFER_DESC someBufferDesc = { bufferElementCount * bufferElementSize , D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE| D3D11_BIND_UNORDERED_ACCESS, 0, 0, 0 };
	someBuffer = new RenderBuffer(someBufferDesc);

	D3D11_UNORDERED_ACCESS_VIEW_DESC someBufferUavViewDesc;
	someBufferUavViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	someBufferUavViewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	someBufferUavViewDesc.Buffer.FirstElement = 0;
	someBufferUavViewDesc.Buffer.NumElements = bufferElementCount;
	someBufferUavViewDesc.Buffer.Flags = 0; // D3D11_BUFFER_UAV_FLAG_RAW
	HRESULT hr = device->CreateUnorderedAccessView(someBuffer->mBuffer, &someBufferUavViewDesc, &someBufferUavView);
	ATLASSERT(hr == S_OK);
}


Game::~Game()
{
	delete constantBuffer;
	delete indexBuffer;
	delete vertexBuffer;

	delete pixelShader;
	delete pixelShaderClear;
	delete pixelShaderFinal;
	delete vertexShader;

	layout->Release();

	someBufferUavView->Release();

	delete someBuffer;
}



void Game::initialise()
{
	// TODO
}

void Game::shutdown()
{
	// TODO
}

void Game::update(const WindowInputData& inputData)
{
	for (auto& event : inputData.mInputEvents)
	{
		// Process events
	}
}


void costTest(int loop)
{
	ID3D11DeviceContext* context = g_dx11Device->getDeviceContext();
	context->PSSetShader(pixelShaderFinal->mPixelShader, NULL, 0);
	for(int i=0; i<loop; ++i)
		context->DrawIndexed(3, 0, 0);
}

void Game::render()
{
	GPU_SCOPED_TIMEREVENT(GameRender, 75, 75, 75);

	ID3D11DeviceContext* context = g_dx11Device->getDeviceContext();
	ID3D11RenderTargetView* backBuffer = g_dx11Device->getBackBufferRT();

	// Constant buffer test
	{	
		RenderBuffer::ScopedMappedRenderbuffer bufferMap;
		constantBuffer->map(D3D11_MAP_WRITE_DISCARD, bufferMap);
		((char*)bufferMap.getDataPtr())[0] = 0;
		((char*)bufferMap.getDataPtr())[1] = 1;
		((char*)bufferMap.getDataPtr())[2] = 2;
		((char*)bufferMap.getDataPtr())[3] = 3;
		((char*)bufferMap.getDataPtr())[4] = 4;
		((char*)bufferMap.getDataPtr())[5] = 5;
		((char*)bufferMap.getDataPtr())[6] = 6;
	}

	{
		GPU_SCOPED_TIMER(Clear, 34, 177, 76);

		// Clear the back buffer
		D3DCOLORVALUE clearColor = { 0.1f, 0.2f, 0.4f, 1.0f };
		context->ClearRenderTargetView(backBuffer, &clearColor.r);

		const UINT* initialCount = 0;
		context->OMSetRenderTargetsAndUnorderedAccessViews(1, &backBuffer, nullptr, 1, 1, &someBufferUavView, initialCount);
	}

	{
		GPU_SCOPED_TIMER(Iterate, 34, 177, 76);

		// Set vertex buffer stride and offset.
		unsigned int stride = sizeof(VertexType);
		unsigned int offset = 0;

		context->IASetVertexBuffers(0, 1, &vertexBuffer->mBuffer, &stride, &offset);
		context->IASetIndexBuffer(indexBuffer->mBuffer, DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// Set the vertex input layout.
		context->IASetInputLayout(layout);

		// Set the vertex and pixel shaders that will be used to render this triangle.
		context->VSSetShader(vertexShader->mVertexShader, NULL, 0);

		// Clear
		context->PSSetShader(pixelShaderClear->mPixelShader, NULL, 0);
		context->DrawIndexed(3, 0, 0);

		//ScopedGpuTimer

		// Accum
		context->PSSetShader(pixelShader->mPixelShader, NULL, 0);
		context->DrawIndexed(12, 0, 0);
	}

	{
		GPU_SCOPED_TIMER(FinalPass, 34, 177, 76);

		// Final view
		context->PSSetShader(pixelShaderFinal->mPixelShader, NULL, 0);
		context->DrawIndexed(3, 0, 0);
	}

	{
		GPU_SCOPED_TIMER(Shadow, 128, 128, 255);
		{
			GPU_SCOPED_TIMER(ShadowCasc0, 64, 64, 255);
			{
				GPU_SCOPED_TIMER(Shadow0, 64, 0, 255);
				costTest(4);
			}
			{
				GPU_SCOPED_TIMER(PartShad0, 0, 64, 255);
				costTest(2);
			}
		}
		{
			GPU_SCOPED_TIMER(ShadowCasc1, 64, 64, 255);
			{
				GPU_SCOPED_TIMER(Shadow1, 64, 0, 255);
				costTest(5);
			}
			{
				GPU_SCOPED_TIMER(PartShad1, 0, 64, 255);
				costTest(2);
			}
		}
		{
			GPU_SCOPED_TIMER(ShadowCasc2, 64, 64, 255);
			{
				GPU_SCOPED_TIMER(Shadow2, 64, 0, 255);
				costTest(6);
			}
			{
				GPU_SCOPED_TIMER(PartShad2, 0, 64, 255);
				costTest(1);
			}
		}
		{
			GPU_SCOPED_TIMER(ShadowCasc3, 64, 64, 255);
			{
				GPU_SCOPED_TIMER(Shadow3, 64, 0, 255);
				costTest(7);
			}
			{
				GPU_SCOPED_TIMER(PartShad3, 0, 64, 255);
				costTest(1);
			}
		}
	}

	{
		GPU_SCOPED_TIMER(GBuffer, 255, 128, 128);
		{
			{
				GPU_SCOPED_TIMER(GbClear, 255, 64, 0);
				costTest(1);
			}
			{
				GPU_SCOPED_TIMER(GbRender, 255, 0, 64);
				costTest(15);
			}
		}
	}
	{
		GPU_SCOPED_TIMER(Volumetric, 100, 200, 100);
		{
			{
				GPU_SCOPED_TIMER(MaterialVoxelisation, 50, 200, 50);
				costTest(10);
			}
			{
				GPU_SCOPED_TIMER(LuminanceVoxelisation, 50, 200, 0);
				costTest(15);
			}
			{
				GPU_SCOPED_TIMER(FroxelIntegration, 0, 200, 50);
				costTest(7);
			}
		}
	}
	{
		GPU_SCOPED_TIMER(Lighting, 240, 201, 14);
		{
			{
				GPU_SCOPED_TIMER(TiledLighting, 128, 100, 7);
				costTest(12);
			}
			{
				GPU_SCOPED_TIMER(SkyAndMedia, 64, 50, 4);
				costTest(6);
			}
		}
	}
	{
		GPU_SCOPED_TIMER(Post, 200, 200, 0);
		{
			{
				GPU_SCOPED_TIMER(TAA, 128, 128, 0);
				costTest(2);
			}
			{
				GPU_SCOPED_TIMER(Tone, 64, 64, 0);
				costTest(2);
			}
		}
	}
}



