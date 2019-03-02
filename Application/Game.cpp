
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
	success &= reloadShader(&mVertexShader, L"Resources\\Common.hlsl", "DefaultVertexShader", firstTimeLoadShaders, nullptr, false);				// No lazy compilation because it is used to create a layout
	success &= reloadShader(&mScreenVertexShader, L"Resources\\Common.hlsl", "ScreenTriangleVertexShader", firstTimeLoadShaders, nullptr, false);	// No lazy compilation because it is used to create a layout
	success &= reload(&mColoredTrianglesShader, L"Resources\\ColoredTriangles.hlsl", "ColoredTrianglesPixelShader", firstTimeLoadShaders, nullptr, lazyCompilation);
	success &= reload(&mToyShader, L"Resources\\ToyShader.hlsl", "ToyShaderCS", firstTimeLoadShaders, nullptr, lazyCompilation);
	success &= reload(&mPostProcessShader, L"Resources\\PostProcess.hlsl", "PostProcessPS", firstTimeLoadShaders, nullptr, lazyCompilation);

	InputLayoutDesc inputLayout;
	appendSimpleVertexDataToInputLayout(inputLayout, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	resetComPtr(&mLayout);
	mVertexShader->createInputLayout(inputLayout, &mLayout);	// Have a layout object with vertex stride in it

	D3D11_TEXTURE2D_DESC backBufferDepthDesc;
	Texture2D::initDepthStencilBuffer(backBufferDepthDesc, 1280, 720, false);
	mBackBufferDepth = new Texture2D(backBufferDepthDesc);

	D3D11_TEXTURE2D_DESC backBufferHdrDesc;
	Texture2D::initDefault(backBufferHdrDesc, DXGI_FORMAT_R32G32B32A32_FLOAT, 1280, 720, true, true); // using high precision for Monte Carlo integration
	mBackBufferHdr = new Texture2D(backBufferHdrDesc);
}

void Game::releaseShaders()
{
	resetComPtr(&mLayout);

	resetPtr(&mVertexShader);
	resetPtr(&mScreenVertexShader);

	resetPtr(&mColoredTrianglesShader);
	resetPtr(&mToyShader);
	resetPtr(&mPostProcessShader);

	resetPtr(&mBackBufferHdr);
	resetPtr(&mBackBufferDepth);
}


void Game::initialise()
{
	////////// Load and compile shaders

	loadShaders(true);

	////////// Create other resources

	ID3D11Device* device = g_dx11Device->getDevice();
	//ID3D11DeviceContext* context = g_dx11Device->getDeviceContext();

	// Simple triangle geometry
	VertexType vertices[3];
	vertices[0]  = { { 0.0f, 0.0f, 0.0f } };
	vertices[1]  = { { 0.0f, 0.5f, 0.0f } };
	vertices[2]  = { { 0.5f, 0.0f, 0.0f } };
	UINT indices[3];
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;


	// Create 
	D3D11_BUFFER_DESC vertexBufferDesc;
	RenderBuffer::initVertexBufferDesc_default(vertexBufferDesc, sizeof(vertices));
	vertexBuffer = new RenderBuffer(vertexBufferDesc, vertices);

	D3D11_BUFFER_DESC indexBufferDesc;
	RenderBuffer::initIndexBufferDesc_default(indexBufferDesc, sizeof(indices));
	indexBuffer = new RenderBuffer(indexBufferDesc, indices);

	mConstantBuffer = new CommonConstantBuffer();

	UINT bufferElementSize = (sizeof(float) * 4);
	UINT bufferElementCount = 1280 * 720;
	D3D11_BUFFER_DESC someBufferDesc = { bufferElementCount * bufferElementSize , D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 0, 0, 0 };
	mSomeBuffer = new RenderBuffer(someBufferDesc);

	D3D11_UNORDERED_ACCESS_VIEW_DESC someBufferUavViewDesc;
	someBufferUavViewDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	someBufferUavViewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	someBufferUavViewDesc.Buffer.FirstElement = 0;
	someBufferUavViewDesc.Buffer.NumElements = bufferElementCount;
	someBufferUavViewDesc.Buffer.Flags = 0; // D3D11_BUFFER_UAV_FLAG_RAW
	HRESULT hr = device->CreateUnorderedAccessView(mSomeBuffer->mBuffer, &someBufferUavViewDesc, &mSomeBufferUavView);
	ATLASSERT(hr == S_OK);

	D3D11_DEPTH_STENCIL_DESC depthStencilState;
	DepthStencilState::initDefaultDepthOnStencilOff(depthStencilState);
	mDefaultDepthStencilState = new DepthStencilState(depthStencilState);

	D3D11_RASTERIZER_DESC rasterDesc;
	RasterizerState::initDefaultState(rasterDesc);
	mDefaultRasterizerState = new RasterizerState(rasterDesc);

	D3D11_BLEND_DESC blendDesc;
	BlendState::initDisabledState(blendDesc);
	mDefaultBlendState = new BlendState(blendDesc);
}

void Game::shutdown()
{
	////////// Release resources

	resetPtr(&mConstantBuffer);
	resetPtr(&indexBuffer);
	resetPtr(&vertexBuffer);

	resetComPtr(&mSomeBufferUavView);
	resetPtr(&mSomeBuffer);

	resetPtr(&mDefaultDepthStencilState);
	resetPtr(&mDefaultBlendState);
	resetPtr(&mDefaultRasterizerState);

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

	// Constant buffer update
	{
		XMMATRIX viewMatrix = XMMatrixIdentity();
		XMMATRIX projMatrix = XMMatrixOrthographicLH(1.0, 1.0, -1.0, 1.0);
		XMMATRIX mViewProjMat = XMMatrixMultiply(viewMatrix, projMatrix);

		CommonConstantBufferStructure cb;
		cb.gViewProjMat = mViewProjMat;
		cb.gColor = { 0.0, 1.0, 1.0, 1.0 };
		cb.gResolution[0] = 1280;
		cb.gResolution[1] = 720;
		mConstantBuffer->update(cb);
	}

	// Set default state
	{
		context->OMSetDepthStencilState(mDefaultDepthStencilState->mState, 0);
		context->OMSetBlendState(mDefaultBlendState->mState, nullptr, 0xffffffff);
		context->RSSetState(mDefaultRasterizerState->mState);

		Viewport viewport;
		ZeroMemory(&viewport, sizeof(Viewport));
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = 1280;					// TODO manage that as it is not in sync with  D:\Projects\DX11Intro\dx11Intro\WindowHelper.cpp
		viewport.Height = 720;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		context->RSSetViewports(1, &viewport);
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

		int sX = divRoundUp(mBackBufferHdr->mDesc.Width, 8);
		int sY = divRoundUp(mBackBufferHdr->mDesc.Height, 8);

		context->Dispatch(sX, sY, 1);
		g_dx11Device->setNullCsResources(context);
		g_dx11Device->setNullCsUnorderedAccessViews(context);
	}

	UINT const uavInitCounts[2] = { -1, -1 };
	context->OMSetRenderTargetsAndUnorderedAccessViews(1, &mBackBufferHdr->mRenderTargetView, mBackBufferDepth->mDepthStencilView, 0, 0, nullptr, nullptr);

	// Render some triangles using the rasterizer
	{
		GPU_SCOPED_TIMEREVENT(Triangles, 34, 177, 76);

		// Set vertex buffer stride and offset.
		unsigned int stride = sizeof(VertexType);
		unsigned int offset = 0;
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

		const UINT* initialCount = 0;
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



