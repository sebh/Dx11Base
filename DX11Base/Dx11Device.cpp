
#include "Dx11Device.h"
#include "D3Dcompiler.h"
#include "Strsafe.h"
#include <iostream>

// Good dx tutorial: http://www.directxtutorial.com/Lesson.aspx?lessonid=11-4-2
// Dx debug API http://seanmiddleditch.com/direct3d-11-debug-api-tricks/, also https://msdn.microsoft.com/en-us/library/windows/desktop/ff476881(v=vs.85).aspx#Debug


Dx11Device* g_dx11Device = nullptr;

Dx11Device::Dx11Device()
{
}

Dx11Device::~Dx11Device()
{
	internalShutdown();
}

void Dx11Device::initialise(const HWND& hWnd)
{
	Dx11Device::shutdown();

	g_dx11Device = new Dx11Device();
	g_dx11Device->internalInitialise(hWnd);
}

void Dx11Device::shutdown()
{
	delete g_dx11Device;
	g_dx11Device = nullptr;
}

void Dx11Device::internalInitialise(const HWND& hWnd)
{
	HRESULT hr;

	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferDesc.RefreshRate.Numerator = 1;				// 60fps target
	scd.BufferDesc.RefreshRate.Denominator = 60;			// 60fps target
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = hWnd;                                // the window to be used
	scd.SampleDesc.Count = 1;                               // multisample
	scd.Windowed = TRUE;                                    // windowed/full-screen mode

	const UINT requestedFeatureLevelsCount = 3;
	const D3D_FEATURE_LEVEL requestedFeatureLevels[requestedFeatureLevelsCount] =
	{
		D3D_FEATURE_LEVEL_12_1,         // Always ask for 12.1 feature level if available
										// other fallbacks
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,       // Always ask for 11.1 (or higher) if available
	};

	// create a device, device context and swap chain using the information in the scd struct
	hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		DX_DEBUG_EVENT|DX_DEBUG_RESOURCE_NAME ? D3D11_CREATE_DEVICE_DEBUG : NULL,
		requestedFeatureLevels,
		requestedFeatureLevelsCount,
		D3D11_SDK_VERSION,
		&scd,
		&mSwapchain,
		&mDev,
		NULL,
		&mDevcon);
	ATLASSERT(hr == S_OK);

#if DX_DEBUG_EVENT
	hr = mDevcon->QueryInterface(__uuidof(mUserDefinedAnnotation), reinterpret_cast<void**>(&mUserDefinedAnnotation));
	ATLASSERT( hr == S_OK );
#endif // DX_DEBUG_EVENT

	// Create the back buffer RT
	ID3D11Texture2D* mBackBufferTexture;// back buffer texture
	mSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&mBackBufferTexture);
	mDev->CreateRenderTargetView(mBackBufferTexture, NULL, &mBackBufferRT);
	mBackBufferTexture->Release();
	//DX_SET_DEBUG_NAME(mBackBufferTexture, "BackBuffer");	// Let dx name this

	// By default, set the back buffer as current render target and viewport (no sate tracking for now...)
	mDevcon->OMSetRenderTargets(1, &mBackBufferRT, NULL); 
	
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = 1280;					// TODO manage that as it is not in sync with  D:\Projects\DX11Intro\dx11Intro\WindowHelper.cpp
	viewport.Height = 720;
	mDevcon->RSSetViewports(1, &viewport);


	D3D_FEATURE_LEVEL deviceFeatureLevel = mDev->GetFeatureLevel();
	OutputDebugStringA("\n\nSelected D3D feature level: ");
	switch (deviceFeatureLevel)
	{
	case D3D_FEATURE_LEVEL_12_1:
		OutputDebugStringA("D3D_FEATURE_LEVEL_12_1\n\n");
		break;
	case D3D_FEATURE_LEVEL_12_0:
		OutputDebugStringA("D3D_FEATURE_LEVEL_12_0\n\n");
		break;
	case D3D_FEATURE_LEVEL_11_1:
		OutputDebugStringA("D3D_FEATURE_LEVEL_11_1\n\n");
		break;
	default:
		ATLASSERT(false); // unhandled level string
	}

	/*D3D11_FEATURE_DATA_D3D11_OPTIONS2 featuretest;
	hr = mDev->CheckFeatureSupport(
		D3D11_FEATURE_D3D11_OPTIONS2,
		&featuretest,
		sizeof(D3D11_FEATURE_DATA_D3D11_OPTIONS2));
	ATLASSERT(hr == S_OK);*/
}

void Dx11Device::internalShutdown()
{
	// close and release all existing COM objects
	mBackBufferRT->Release();
	mSwapchain->Release();
	mDev->Release();
	mDevcon->Release();
}

void Dx11Device::swap(bool vsyncEnabled)
{
	mSwapchain->Present(vsyncEnabled ? 1 : 0, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


RenderBuffer::RenderBuffer(D3D11_BUFFER_DESC& bufferDesc, void* initialData) :
	mBufferDesc(bufferDesc)
{
	ID3D11Device* device = g_dx11Device->getDevice();

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = initialData;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer( &mBufferDesc, initialData ? &data : nullptr, &mBuffer);
	ATLASSERT(hr == S_OK);
}

RenderBuffer::~RenderBuffer()
{
	mBuffer->Release();
	mBuffer = 0;
}

void RenderBuffer::map(D3D11_MAP map, ScopedMappedRenderbuffer& mappedBuffer)
{
	// Check some state
	ATLASSERT( mBufferDesc.Usage == D3D11_USAGE_DYNAMIC && (mBufferDesc.CPUAccessFlags&D3D11_CPU_ACCESS_WRITE)!=0 );

	// Reset to 0
	ZeroMemory(&mappedBuffer.mMappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	ID3D11DeviceContext* context = g_dx11Device->getDeviceContext();
	context->Map(mBuffer, 0, map, 0, &mappedBuffer.mMappedResource);
	mappedBuffer.mMappedBuffer = mBuffer;
}

void RenderBuffer::unmap(ScopedMappedRenderbuffer& mappedBuffer)
{
	if (mappedBuffer.mMappedBuffer)
	{
		ID3D11DeviceContext* context = g_dx11Device->getDeviceContext();
		context->Unmap(mappedBuffer.mMappedBuffer, 0);
		mappedBuffer.mMappedBuffer = nullptr;
	}
}

void RenderBuffer::initConstantBufferDesc_dynamic(D3D11_BUFFER_DESC& desc, UINT byteSize)
{
	desc = { byteSize , D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0 };
}
void RenderBuffer::initVertexBufferDesc_default(D3D11_BUFFER_DESC& desc, UINT byteSize)
{
	desc = { byteSize , D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
}
void RenderBuffer::initIndexBufferDesc_default(D3D11_BUFFER_DESC& desc, UINT byteSize)
{
	desc = { byteSize , D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, 0, 0, 0 };
}
void RenderBuffer::initBufferDesc_default(D3D11_BUFFER_DESC& desc, UINT byteSize)
{
	desc = { byteSize , D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, 0 };
}
void RenderBuffer::initBufferDesc_uav(D3D11_BUFFER_DESC& desc, UINT byteSize)
{
	desc = { byteSize , D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 0, 0, 0 };
}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


void appendSimpleVertexDataToInputLayout(InputLayoutDescriptors& inputLayout, const char* semanticName, DXGI_FORMAT format)
{
	D3D11_INPUT_ELEMENT_DESC desc;

	desc.SemanticName = semanticName;
	desc.SemanticIndex = 0;
	desc.Format = format;
	desc.InputSlot = 0;
	desc.AlignedByteOffset = inputLayout.empty() ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
	desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc.InstanceDataStepRate = 0;

	inputLayout.push_back(desc);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderBase::ShaderBase(const TCHAR* filename, const char* entryFunction, const char* profile)
{
	ID3DBlob * errorbuffer = NULL;

	const UINT defaultFlags = 0;

	HRESULT hr = D3DCompileFromFile(
		filename,		// filename
		NULL,			// defines
		NULL,			// include
		entryFunction,	// function name
		profile,		// target profile
		defaultFlags,	// flag1
		defaultFlags,	// flag2
		&mShaderBuffer, // ouput
		&errorbuffer);	// errors

	if (FAILED(hr))
	{
		OutputDebugStringA("\n===> Failed to compile shader: function=");
		OutputDebugStringA(entryFunction);
		OutputDebugStringA(", profile=");
		OutputDebugStringA(profile);
		OutputDebugStringA(", file=");
		OutputDebugStringW(filename);
		OutputDebugStringA(" :\n");

		if (errorbuffer)
		{
			OutputDebugStringA((char*)errorbuffer->GetBufferPointer());
			errorbuffer->Release();
		}

		if (mShaderBuffer)
			mShaderBuffer->Release();
		OutputDebugStringA("\n\n");

		// Not great but good enough for now...
		exit(0);
	}
}

ShaderBase::~ShaderBase()
{
	mShaderBuffer->Release();
}

VertexShader::VertexShader(const TCHAR* filename, const char* entryFunction)
	: ShaderBase(filename, entryFunction, "vs_5_0")
{
	ID3D11Device* device = g_dx11Device->getDevice();
	HRESULT hr = device->CreateVertexShader(mShaderBuffer->GetBufferPointer(), mShaderBuffer->GetBufferSize(), NULL, &mVertexShader);
	ATLASSERT(hr == S_OK);
}
VertexShader::~VertexShader()
{
	mVertexShader->Release();
}

void VertexShader::createInputLayout(InputLayoutDescriptors inputLayout, ID3D11InputLayout** layout)
{
	ID3D11Device* device = g_dx11Device->getDevice();
	HRESULT hr = device->CreateInputLayout(inputLayout.data(), UINT(inputLayout.size()), mShaderBuffer->GetBufferPointer(), mShaderBuffer->GetBufferSize(), layout);
	ATLASSERT(hr == S_OK);
}

PixelShader::PixelShader(const TCHAR* filename, const char* entryFunction)
	: ShaderBase(filename, entryFunction, "ps_5_0")
{
	ID3D11Device* device = g_dx11Device->getDevice();
	HRESULT hr = device->CreatePixelShader(mShaderBuffer->GetBufferPointer(), mShaderBuffer->GetBufferSize(), NULL, &mPixelShader);
	ATLASSERT(hr == S_OK);
}
PixelShader::~PixelShader()
{
	mPixelShader->Release();
}

HullShader::HullShader(const TCHAR* filename, const char* entryFunction)
	: ShaderBase(filename, entryFunction, "hs_5_0")
{
	ID3D11Device* device = g_dx11Device->getDevice();
	HRESULT hr = device->CreateHullShader(mShaderBuffer->GetBufferPointer(), mShaderBuffer->GetBufferSize(), NULL, &mHullShader);
	ATLASSERT(hr == S_OK);
}
HullShader::~HullShader()
{
	mHullShader->Release();
}

DomainShader::DomainShader(const TCHAR* filename, const char* entryFunction)
	: ShaderBase(filename, entryFunction, "ds_5_0")
{
	ID3D11Device* device = g_dx11Device->getDevice();
	HRESULT hr = device->CreateDomainShader(mShaderBuffer->GetBufferPointer(), mShaderBuffer->GetBufferSize(), NULL, &mDomainShader);
	ATLASSERT(hr == S_OK);
}
DomainShader::~DomainShader()
{
	mDomainShader->Release();
}

GeometryShader::GeometryShader(const TCHAR* filename, const char* entryFunction)
	: ShaderBase(filename, entryFunction, "gs_5_0")
{
	ID3D11Device* device = g_dx11Device->getDevice();
	HRESULT hr = device->CreateGeometryShader(mShaderBuffer->GetBufferPointer(), mShaderBuffer->GetBufferSize(), NULL, &mGeometryShader);
	ATLASSERT(hr == S_OK);
}
GeometryShader::~GeometryShader()
{
	mGeometryShader->Release();
}

ComputeShader::ComputeShader(const TCHAR* filename, const char* entryFunction)
	: ShaderBase(filename, entryFunction, "cs_5_0")
{
	ID3D11Device* device = g_dx11Device->getDevice();
	HRESULT hr = device->CreateComputeShader(mShaderBuffer->GetBufferPointer(), mShaderBuffer->GetBufferSize(), NULL, &mComputeShader);
	ATLASSERT(hr == S_OK);
}
ComputeShader::~ComputeShader()
{
	mComputeShader->Release();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// static members
DxGpuPerformance::GpuTimerMap DxGpuPerformance::mTimers;
int DxGpuPerformance::mMeasureTimerFrameId;
int DxGpuPerformance::mReadTimerFrameId;
int DxGpuPerformance::mLastReadTimerFrameId;

DxGpuPerformance::DxGpuTimer DxGpuPerformance::mTimerArray[V_TIMER_MAX_COUNT];
int DxGpuPerformance::mAllocatedTimers;

DxGpuPerformance::TimerGraphNode DxGpuPerformance::mTimerGraphNodeArray[V_GPU_TIMER_FRAMECOUNT][V_TIMER_MAX_COUNT];
int DxGpuPerformance::mAllocatedTimerGraphNodes[V_GPU_TIMER_FRAMECOUNT];

DxGpuPerformance::GpuTimerGraph DxGpuPerformance::mTimerGraphs[V_GPU_TIMER_FRAMECOUNT];
DxGpuPerformance::TimerGraphNode* DxGpuPerformance::mCurrentTimeGraph = nullptr;

void DxGpuPerformance::initialise()
{
	mTimers.clear();
	mAllocatedTimers = 0;

	mMeasureTimerFrameId = 0;							// first frame
	mReadTimerFrameId = -V_GPU_TIMER_FRAMECOUNT+1;		// invalid
	mLastReadTimerFrameId = -V_GPU_TIMER_FRAMECOUNT-1;	// invalid
}
void DxGpuPerformance::shutdown()
{
	for (int i = 0; i < mAllocatedTimers; ++i)
		mTimerArray[i].release();
	mTimers.clear();
}

void DxGpuPerformance::startFrame()
{
	// Clear the frame we are going to measure and append root timer node
	for (int i = 0, cnt = mAllocatedTimerGraphNodes[mMeasureTimerFrameId]; i < cnt; ++i)
		mTimerGraphNodeArray[mMeasureTimerFrameId][i].subGraph.clear();
	mAllocatedTimerGraphNodes[mMeasureTimerFrameId] = 0; // reset the counter
	TimerGraphNode* root = &mTimerGraphNodeArray [mMeasureTimerFrameId] [mAllocatedTimerGraphNodes[mMeasureTimerFrameId]++];
	root->name = "root";
	root->r = root->g = root->b = 127;
	mTimerGraphs[mMeasureTimerFrameId].clear();

	mTimerGraphs[mMeasureTimerFrameId].push_back(root);
	mCurrentTimeGraph = (*mTimerGraphs[mMeasureTimerFrameId].begin());
}

void DxGpuPerformance::startGpuTimer(const char* name, unsigned char r, unsigned char g, unsigned char  b)
{
	GpuTimerMap::iterator it = mTimers.find(name);
	DxGpuTimer* timer = nullptr;
	if (it == mTimers.end())
	{
		// Allocate a timer and insert into the map
		ATLASSERT(mAllocatedTimers<(V_TIMER_MAX_COUNT-1));
		timer = &mTimerArray[mAllocatedTimers++];
		mTimers[name] = timer;
		timer->initialize();
	}
	else
	{
		ATLASSERT(!(*it).second->mUsedThisFrame);	// a timer can only be used once a frame
		timer = (*it).second;
	}

	ID3D11DeviceContext* context = g_dx11Device->getDeviceContext();
	context->Begin(timer->mDisjointQueries[mMeasureTimerFrameId]);
	context->End(timer->mBeginQueries[mMeasureTimerFrameId]);

	// Make sure we do not add the timer twice per frame
	ATLASSERT(!timer->mUsedThisFrame);
	timer->mUsedThisFrame = true;

	// Push the timer node we just started
	ATLASSERT(mAllocatedTimerGraphNodes[mMeasureTimerFrameId]<(V_TIMER_MAX_COUNT - 1));
	TimerGraphNode* node = &mTimerGraphNodeArray[mMeasureTimerFrameId][mAllocatedTimerGraphNodes[mMeasureTimerFrameId]++];
	node->name = name;
	node->r = float(r) / 255.0f;
	node->g = float(g) / 255.0f;
	node->b = float(b) / 255.0f;
	node->timer = timer;
	node->parent = mCurrentTimeGraph;
	mCurrentTimeGraph->subGraph.push_back(node);
	mCurrentTimeGraph = (*mCurrentTimeGraph->subGraph.rbegin());
	timer->mNode[mMeasureTimerFrameId] = mCurrentTimeGraph;
								// TO SOLVE: have static array of node 
}

void DxGpuPerformance::endGpuTimer(const char* name)
{
	DxGpuTimer* timer = mTimers[name];

	ID3D11DeviceContext* context = g_dx11Device->getDeviceContext();
	context->End(timer->mDisjointQueries[mMeasureTimerFrameId]);
	context->End(timer->mEndQueries[mMeasureTimerFrameId]);
	timer->mEnded = true;

	// Pop to the parent of the timer node that has just ended
	mCurrentTimeGraph = mCurrentTimeGraph->parent;
	ATLASSERT(mCurrentTimeGraph!=nullptr);
}

void DxGpuPerformance::endFrame()
{
	// Fetch data from ready timer
	if (mReadTimerFrameId >= 0)
	{
		int localReadTimerFrameId = mReadTimerFrameId%V_GPU_TIMER_FRAMECOUNT;

		ID3D11DeviceContext* context = g_dx11Device->getDeviceContext();
		DxGpuPerformance::GpuTimerMap::iterator it;

		// Get all the data first
		for (it = mTimers.begin(); it != mTimers.end(); it++)
		{
			DxGpuPerformance::DxGpuTimer* timer = (*it).second;
			if (!timer->mUsedThisFrame)	// we should test usePreviousFrame but that will be enough for now
				continue;
			ATLASSERT(timer->mEnded);	// the timer must have been ended this frame

			TimerGraphNode* node = timer->mNode[localReadTimerFrameId];
			ATLASSERT(node);

			while (context->GetData(timer->mBeginQueries[localReadTimerFrameId], &node->mBeginTick, sizeof(UINT64), 0) != S_OK);
			while (context->GetData(timer->mEndQueries[localReadTimerFrameId], &node->mEndTick, sizeof(UINT64), 0) != S_OK);
			while (context->GetData(timer->mDisjointQueries[localReadTimerFrameId], &node->disjointData, sizeof(D3D11_QUERY_DATA_TIMESTAMP_DISJOINT), 0) != S_OK);
		}

		// Get the begining of the frame measurement
		UINT64 minBeginTime = 0xFFFFFFFFFFFFFFFF;
		for (it = mTimers.begin(); it != mTimers.end(); it++)
		{
			DxGpuPerformance::DxGpuTimer* timer = (*it).second;
			if (!timer->mUsedThisFrame)	// we should test usePreviousFrame but that will be enough for now
				continue;
			minBeginTime = min(minBeginTime, timer->mNode[localReadTimerFrameId]->mBeginTick);
		}


		for (it = mTimers.begin(); it != mTimers.end(); it++)
		{
			DxGpuPerformance::DxGpuTimer* timer = (*it).second;

			if (!timer->mUsedThisFrame)	// we should test usePreviousFrame but that will be enough for now
				continue;

			// Reset the safety checks
			timer->mUsedThisFrame = false;
			timer->mEnded = false;

			float beginMs = 0.0f;
			float endMs = 0.0f;
			float lastDurationMs = 0.0f;
			TimerGraphNode* node = timer->mNode[localReadTimerFrameId];
			if (node->disjointData.Disjoint == FALSE)
			{
				float factor = 1000.0f / float(node->disjointData.Frequency);
				beginMs = (node->mBeginTick - minBeginTime) * factor;
				endMs   = (node->mEndTick - minBeginTime) * factor;
				lastDurationMs = (node->mEndTick - node->mBeginTick) * factor;
			}
			node->mBeginMs = beginMs;
			node->mEndMs = endMs;
			node->mLastDurationMs = lastDurationMs;
		}
	}

	// Move onto next frame
	mReadTimerFrameId++;
	mMeasureTimerFrameId = (mMeasureTimerFrameId + 1) % V_GPU_TIMER_FRAMECOUNT;
	mLastReadTimerFrameId++;
}

const DxGpuPerformance::TimerGraphNode* DxGpuPerformance::getLastUpdatedTimerGraphRootNode()
{
	if (mLastReadTimerFrameId >= 0)
	{
		return *mTimerGraphs[mLastReadTimerFrameId%V_GPU_TIMER_FRAMECOUNT].begin();
	}
	return nullptr;
}

DxGpuPerformance::DxGpuTimer::DxGpuTimer()
{
}

DxGpuPerformance::DxGpuTimer::~DxGpuTimer()
{
}

void DxGpuPerformance::DxGpuTimer::initialize()
{
	ID3D11Device* device = g_dx11Device->getDevice();
	D3D11_QUERY_DESC queryDesc;
	queryDesc.Query = D3D11_QUERY_TIMESTAMP;
	queryDesc.MiscFlags = 0;
	D3D11_QUERY_DESC disjointQueryDesc;
	disjointQueryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
	disjointQueryDesc.MiscFlags = 0;
	for (int i = 0; i < V_GPU_TIMER_FRAMECOUNT; ++i)
	{
		device->CreateQuery(&disjointQueryDesc, &mDisjointQueries[i]);
		device->CreateQuery(&queryDesc, &mBeginQueries[i]);
		device->CreateQuery(&queryDesc, &mEndQueries[i]);
	}
}
void DxGpuPerformance::DxGpuTimer::release()
{
	ID3D11Device* device = g_dx11Device->getDevice();
	for (int i = 0; i < V_GPU_TIMER_FRAMECOUNT; ++i)
	{
		mDisjointQueries[i]->Release();
		mBeginQueries[i]->Release();
		mEndQueries[i]->Release();
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


//buffer, constant, view, render target and shader creation
// http://www.rastertek.com/dx11tut04.html
// https://msdn.microsoft.com/en-us/library/windows/desktop/dn508285(v=vs.85).aspx









