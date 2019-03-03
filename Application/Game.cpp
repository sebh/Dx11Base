
#include "Game.h"

#include "windows.h"

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
	success &= reloadShader(&mVertexShader, L"Resources\\Common.hlsl", "DefaultVertexShader", firstTimeLoadShaders, nullptr, false);				// No lazy compilation because it is used to create a layout
	success &= reloadShader(&mScreenVertexShader, L"Resources\\Common.hlsl", "ScreenTriangleVertexShader", firstTimeLoadShaders, nullptr, false);	// No lazy compilation because it is used to create a layout
	success &= reload(&mColoredTrianglesShader, L"Resources\\ColoredTriangles.hlsl", "ColoredTrianglesPixelShader", firstTimeLoadShaders, nullptr, lazyCompilation);
	success &= reload(&mToyShader, L"Resources\\ToyShader.hlsl", "ToyShaderCS", firstTimeLoadShaders, nullptr, lazyCompilation);
	success &= reload(&mPostProcessShader, L"Resources\\PostProcess.hlsl", "PostProcessPS", firstTimeLoadShaders, nullptr, lazyCompilation);

	InputLayoutDesc inputLayout;
	appendSimpleVertexDataToInputLayout(inputLayout, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	resetComPtr(&mLayout);
	mVertexShader->createInputLayout(inputLayout, &mLayout);	// Have a layout object with vertex stride in it
}

void Game::releaseShaders()
{
	resetComPtr(&mLayout);

	resetPtr(&mVertexShader);
	resetPtr(&mScreenVertexShader);

	resetPtr(&mColoredTrianglesShader);
	resetPtr(&mToyShader);
	resetPtr(&mPostProcessShader);
}


void Game::initialise()
{
	////////// Load and compile shaders

	loadShaders(true);

	////////// Create other resources

	D3dDevice* device = g_dx11Device->getDevice();
	const D3dViewport& viewport = g_dx11Device->getBackBufferViewport();
	allocateResolutionIndependentResources();
	allocateResolutionDependentResources(viewport.Width, viewport.Height);
}

void Game::reallocateResolutionDepedent(uint32 newWidth, uint32 newHeight)
{
	releaseResolutionDependentResources();
	allocateResolutionDependentResources(newWidth, newHeight);
}


void Game::allocateResolutionIndependentResources()
{
	D3dDevice* device = g_dx11Device->getDevice();

	// Simple triangle geometry
	VertexType vertices[3];
	vertices[0] = { { 0.0f, 0.0f, 0.0f } };
	vertices[1] = { { 0.0f, 0.5f, 0.0f } };
	vertices[2] = { { 0.5f, 0.0f, 0.0f } };
	uint32 indices[3];
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	vertexBuffer = new RenderBuffer(RenderBuffer::initVertexBufferDesc_default(sizeof(vertices)), vertices);
	indexBuffer = new RenderBuffer(RenderBuffer::initIndexBufferDesc_default(sizeof(indices)), indices);

	mConstantBuffer = new CommonConstantBuffer();

	uint32 bufferElementSize = (sizeof(float) * 4);
	uint32 bufferElementCount = 1024;
	D3dBufferDesc someBufferDesc = { bufferElementCount * bufferElementSize , D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 0, 0, 0 };
	mSomeBuffer = new RenderBuffer(someBufferDesc);

	D3dUnorderedAccessViewDesc someBufferUavViewDesc;
	someBufferUavViewDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	someBufferUavViewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	someBufferUavViewDesc.Buffer.FirstElement = 0;
	someBufferUavViewDesc.Buffer.NumElements = bufferElementCount;
	someBufferUavViewDesc.Buffer.Flags = 0; // D3D11_BUFFER_UAV_FLAG_RAW
	HRESULT hr = device->CreateUnorderedAccessView(mSomeBuffer->mBuffer, &someBufferUavViewDesc, &mSomeBufferUavView);
	ATLASSERT(hr == S_OK);

	mDefaultDepthStencilState = new DepthStencilState(DepthStencilState::initDefaultDepthOnStencilOff());
	mDefaultRasterizerState = new RasterizerState(RasterizerState::initDefaultState());
	mDefaultBlendState = new BlendState(BlendState::initDisabledState());
}

void Game::releaseResolutionIndependentResources()
{
	resetPtr(&mConstantBuffer);
	resetPtr(&indexBuffer);
	resetPtr(&vertexBuffer);

	resetComPtr(&mSomeBufferUavView);
	resetPtr(&mSomeBuffer);

	resetPtr(&mDefaultDepthStencilState);
	resetPtr(&mDefaultBlendState);
	resetPtr(&mDefaultRasterizerState);
}

void Game::allocateResolutionDependentResources(uint32 newWidth, uint32 newHeight)
{
	mBackBufferDepth = new Texture2D(Texture2D::initDepthStencilBuffer(newWidth, newHeight, false));
	mBackBufferHdr = new Texture2D(Texture2D::initDefault(DXGI_FORMAT_R32G32B32A32_FLOAT, newWidth, newHeight, true, true));
}

void Game::releaseResolutionDependentResources()
{
	resetPtr(&mBackBufferHdr);
	resetPtr(&mBackBufferDepth);
}

void Game::shutdown()
{
	////////// Release resources

	releaseResolutionIndependentResources();
	releaseResolutionDependentResources();

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

	const D3dViewport& backBufferViewport = g_dx11Device->getBackBufferViewport();
	D3dRenderContext* context = g_dx11Device->getDeviceContext();
	D3dRenderTargetView* backBuffer = g_dx11Device->getBackBufferRT();

	// Constant buffer update
	{
		XMMATRIX viewMatrix = XMMatrixIdentity();
		XMMATRIX projMatrix = XMMatrixOrthographicLH(1.0, 1.0, -1.0, 1.0);
		XMMATRIX mViewProjMat = XMMatrixMultiply(viewMatrix, projMatrix);

		CommonConstantBufferStructure cb;
		cb.gViewProjMat = mViewProjMat;
		cb.gColor = { 0.0, 1.0, 1.0, 1.0 };
		cb.gResolution[0] = backBufferViewport.Width;
		cb.gResolution[1] = backBufferViewport.Height;
		mConstantBuffer->update(cb);
	}

	// Set default state
	{
		context->OMSetDepthStencilState(mDefaultDepthStencilState->mState, 0);
		context->OMSetBlendState(mDefaultBlendState->mState, nullptr, 0xffffffff);
		context->RSSetState(mDefaultRasterizerState->mState);
		context->RSSetViewports(1, &backBufferViewport);
	}

	// Clear the HDR back buffer
	{
		GPU_SCOPED_TIMER(Clear, 34, 177, 76);

		D3DCOLORVALUE clearColor = { 0.1f, 0.2f, 0.4f, 1.0f };
		context->ClearRenderTargetView(mBackBufferHdr->mRenderTargetView, &clearColor.r);
		context->ClearDepthStencilView(mBackBufferDepth->mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);;
	}

	// Render a shader toy using compute
	{
		GPU_SCOPED_TIMEREVENT(ShaderToy, 34, 177, 76);

		mToyShader->setShader(*context);
		context->CSSetConstantBuffers(0, 1, &mConstantBuffer->mBuffer);
		context->CSSetUnorderedAccessViews(0, 1, &mBackBufferHdr->mUnorderedAccessView, nullptr);

		int32 sX = divRoundUp(backBufferViewport.Width, 8);
		int32 sY = divRoundUp(backBufferViewport.Height, 8);

		context->Dispatch(sX, sY, 1);
		g_dx11Device->setNullCsResources(context);
		g_dx11Device->setNullCsUnorderedAccessViews(context);
	}

	context->OMSetRenderTargetsAndUnorderedAccessViews(1, &mBackBufferHdr->mRenderTargetView, mBackBufferDepth->mDepthStencilView, 0, 0, nullptr, nullptr);

	// Render some triangles using the rasterizer
	{
		GPU_SCOPED_TIMEREVENT(Triangles, 34, 177, 76);

		// Set vertex buffer stride and offset.
		uint32 stride = sizeof(VertexType);
		uint32 offset = 0;
		context->IASetVertexBuffers(0, 1, &vertexBuffer->mBuffer, &stride, &offset);
		context->IASetIndexBuffer(indexBuffer->mBuffer, DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// Set the vertex input layout.
		context->IASetInputLayout(mLayout);

		// Final view
		mVertexShader->setShader(*context);
		mColoredTrianglesShader->setShader(*context);
		context->VSSetConstantBuffers(0, 1, &mConstantBuffer->mBuffer);
		context->PSSetConstantBuffers(0, 1, &mConstantBuffer->mBuffer);

		context->DrawIndexed(3, 0, 0);
	}

	// Post process into the back buffer using a pixel shader
	{
		GPU_SCOPED_TIMEREVENT(Post, 34, 177, 76);

		const uint32* initialCount = 0;
		context->OMSetRenderTargetsAndUnorderedAccessViews(1, &backBuffer, nullptr, 1, 1, &mSomeBufferUavView, initialCount);

		// Set null input assembly and layout
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(nullptr);

		// Final view
		mScreenVertexShader->setShader(*context);
		mPostProcessShader->setShader(*context);
		context->VSSetConstantBuffers(0, 1, &mConstantBuffer->mBuffer);
		context->PSSetConstantBuffers(0, 1, &mConstantBuffer->mBuffer);

		context->PSSetShaderResources(0, 1, &mBackBufferHdr->mShaderResourceView);

		context->Draw(3, 0);
		g_dx11Device->setNullPsResources(context);
	}
}



