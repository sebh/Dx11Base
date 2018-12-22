
#include "Game.h"

#include "windows.h"
#include "DirectXMath.h"

#include <imgui.h>


Game::Game()
{
}


Game::~Game()
{
}


void Game::loadShaders(bool firstTimeLoadShaders)
{
	bool success = true;

	auto reloadShader = [&](auto** previousShader, const TCHAR* filename, const char* entryFunction, bool firstTimeLoadShaders, const Macros* macros = NULL, bool lazyCompilation = false)
	{
		if (firstTimeLoadShaders)
		{
			// The first time we want to compile the shader and make sure we fail if not succesful
			return reload(previousShader, filename, entryFunction, true, macros, lazyCompilation);
		}
		else
		{
			// other time we only want to make the shader dirty to schedule compilation when used
			(*previousShader)->markDirty();
			return true;
		}
	};

	const bool lazyCompilation = true;
	success &= reloadShader(&mVertexShader, L"Resources\\TestShader.hlsl", "ColorVertexShader", firstTimeLoadShaders, nullptr, false); // No lazy compilation because it is used to create a layout
	success &= reload(&mPixelShader, L"Resources\\TestShader.hlsl", "ColorPixelShader", firstTimeLoadShaders, nullptr, lazyCompilation);
	success &= reload(&mPixelShaderClear, L"Resources\\TestShader.hlsl", "ClearPixelShader", firstTimeLoadShaders, nullptr, lazyCompilation);
	success &= reload(&mPixelShaderFinal, L"Resources\\TestShader.hlsl", "FinalPixelShader", firstTimeLoadShaders, nullptr, lazyCompilation);

	InputLayoutDesc inputLayout;
	appendSimpleVertexDataToInputLayout(inputLayout, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	appendSimpleVertexDataToInputLayout(inputLayout, "COLOR", DXGI_FORMAT_R32G32B32A32_FLOAT);
	resetComPtr(&mLayout);
	mVertexShader->createInputLayout(inputLayout, &mLayout);	// Have a layout object with vertex stride in it
}

void Game::releaseShaders()
{
	resetPtr(&mPixelShader);
	resetPtr(&mPixelShaderClear);
	resetPtr(&mPixelShaderFinal);
	resetPtr(&mVertexShader);
	resetComPtr(&mLayout);
}


void Game::initialise()
{
	////////// Load and compile shaders

	loadShaders(true);

	////////// Create other resources

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

	mConstantBuffer = new MyConstantBuffer();

	UINT bufferElementSize = (sizeof(float) * 4);
	UINT bufferElementCount = 1280 * 720;
	D3D11_BUFFER_DESC someBufferDesc = { bufferElementCount * bufferElementSize , D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 0, 0, 0 };
	mSomeBuffer = new RenderBuffer(someBufferDesc);

	D3D11_UNORDERED_ACCESS_VIEW_DESC someBufferUavViewDesc;
	someBufferUavViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	someBufferUavViewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	someBufferUavViewDesc.Buffer.FirstElement = 0;
	someBufferUavViewDesc.Buffer.NumElements = bufferElementCount;
	someBufferUavViewDesc.Buffer.Flags = 0; // D3D11_BUFFER_UAV_FLAG_RAW
	HRESULT hr = device->CreateUnorderedAccessView(mSomeBuffer->mBuffer, &someBufferUavViewDesc, &mSomeBufferUavView);
	ATLASSERT(hr == S_OK);
}

void Game::shutdown()
{
	////////// Release resources

	delete mConstantBuffer;
	delete indexBuffer;
	delete vertexBuffer;

	mSomeBufferUavView->Release();
	delete mSomeBuffer;

	////////// Release shaders

	releaseShaders();
}

void Game::update(const WindowInputData& inputData)
{
	for (auto& event : inputData.mInputEvents)
	{
		// Process events
	}

	// Listen to CTRL+S for shader live update in a very simple fashion (from http://www.lofibucket.com/articles/64k_intro.html)
	static ULONGLONG lastLoadTime = GetTickCount64();
	if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState('S'))
	{
		const ULONGLONG tickCount = GetTickCount64();
		if (tickCount - lastLoadTime > 200)
		{
			Sleep(100);					// Wait for a while to let the file system finish the file write.
			loadShaders(false);			// Reload (all) the shaders
		}
		lastLoadTime = tickCount;
	}
}


void Game::render()
{
	GPU_SCOPED_TIMEREVENT(GameRender, 75, 75, 75);

	ID3D11DeviceContext* context = g_dx11Device->getDeviceContext();
	ID3D11RenderTargetView* backBuffer = g_dx11Device->getBackBufferRT();

	// Constant buffer test
	{	
		ConstantBufferStructureExemple cb;
		cb.f = 1.0f;
		cb.f2= 2.0f;
		cb.i = -1;
		cb.u = 2;
		mConstantBuffer->update(cb);
	}

	{
		GPU_SCOPED_TIMER(Clear, 34, 177, 76);

		// Clear the back buffer
		D3DCOLORVALUE clearColor = { 0.1f, 0.2f, 0.4f, 1.0f };
		context->ClearRenderTargetView(backBuffer, &clearColor.r);

		const UINT* initialCount = 0;
		context->OMSetRenderTargetsAndUnorderedAccessViews(1, &backBuffer, nullptr, 1, 1, &mSomeBufferUavView, initialCount);
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
		context->IASetInputLayout(mLayout);

		// Set the vertex and pixel shaders that will be used to render this triangle.
		mVertexShader->setShader(*context);

		// Clear
		mPixelShaderClear->setShader(*context);
		context->DrawIndexed(3, 0, 0);

		// Accum
		mPixelShader->setShader(*context);
		context->DrawIndexed(12, 0, 0);
	}

	{
		GPU_SCOPED_TIMER(FinalPass, 34, 177, 76);

		// Final view
		mPixelShaderFinal->setShader(*context);
		context->DrawIndexed(3, 0, 0);
	}

	auto costTest = [&](int loop)
	{
		ID3D11DeviceContext* context = g_dx11Device->getDeviceContext();
		mPixelShaderFinal->setShader(*context);
		for (int i = 0; i<loop; ++i)
			context->DrawIndexed(3, 0, 0);
	};

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



