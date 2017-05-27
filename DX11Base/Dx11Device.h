#pragma once

#define DX_DEBUG_EVENT 1
#define DX_DEBUG_RESOURCE_NAME 1

// Windows and Dx11 includes
#include <map>
#include <vector>
#include <windows.h>
#include <windowsx.h>
#include <atlbase.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>

// include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")
#if DX_DEBUG_EVENT || DX_DEBUG_RESOURCE_NAME
	#pragma comment( lib, "dxguid.lib")	// For debug name guid
#endif

class Dx11Device
{
public:

	static void initialise(const HWND& hWnd);
	static void shutdown();

	ID3D11Device*							getDevice()			{ return mDev; }
	ID3D11DeviceContext*					getDeviceContext()	{ return mDevcon; }
	IDXGISwapChain*							getSwapChain()		{ return mSwapchain; }
	ID3D11RenderTargetView*					getBackBufferRT()	{ return mBackBufferRT; }

#if DX_DEBUG_EVENT
	CComPtr<ID3DUserDefinedAnnotation>		mUserDefinedAnnotation;
#endif

	void swap(bool vsyncEnabled);

	static void setNullRenderTarget(ID3D11DeviceContext* devcon)
	{
		ID3D11RenderTargetView*    nullRTV = nullptr;
		ID3D11UnorderedAccessView* nullUAV = nullptr;
		//devcon->OMSetRenderTargets(1, &nullRTV, nullptr);
		devcon->OMSetRenderTargetsAndUnorderedAccessViews(1, &nullRTV, nullptr, 1, 0, &nullUAV, nullptr);
	}
	static void setNullPsResources(ID3D11DeviceContext* devcon)
	{
		static ID3D11ShaderResourceView* null[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };	// not good, only 8, would need something smarter maybe...
		devcon->PSSetShaderResources(0, 8, null);
	}
	static void setNullVsResources(ID3D11DeviceContext* devcon)
	{
		static ID3D11ShaderResourceView* null[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		devcon->VSSetShaderResources(0, 8, null);
	}
	static void setNullCsResources(ID3D11DeviceContext* devcon)
	{
		static ID3D11ShaderResourceView* null[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		devcon->CSSetShaderResources(0, 8, null);
	}
	static void setNullCsUnorderedAccessViews(ID3D11DeviceContext* devcon)
	{
		static ID3D11UnorderedAccessView* null[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		devcon->CSSetUnorderedAccessViews(0, 8, null, nullptr);
	}

private:
	Dx11Device();
	Dx11Device(Dx11Device&);
	//Dx11Device(const Dx11Device&);
	~Dx11Device();

	void internalInitialise(const HWND& hWnd);
	void internalShutdown();

	IDXGISwapChain*							mSwapchain;				// the pointer to the swap chain interface
	ID3D11Device*							mDev;					// the pointer to our Direct3D device interface
	ID3D11DeviceContext*					mDevcon;				// the pointer to our Direct3D device context

	ID3D11RenderTargetView*					mBackBufferRT;			// back buffer render target
};

extern Dx11Device* g_dx11Device;


#if DX_DEBUG_RESOURCE_NAME
#define DX_SET_DEBUG_NAME(obj, debugName) obj->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(debugName), debugName)
#else
#define DX_SET_DEBUG_NAME(obj, debugName) 
#endif

#if DX_DEBUG_EVENT
#define GPU_BEGIN_EVENT(eventName) g_dx11Device->mUserDefinedAnnotation->BeginEvent(L""#eventName)
#define GPU_END_EVENT() g_dx11Device->mUserDefinedAnnotation->EndEvent()
#else
#define GPU_BEGIN_EVENT(eventName) 
#define GPU_END_EVENT() 
#endif

struct ScopedGpuEvent
{
	ScopedGpuEvent(LPCWSTR name)
		: mName(name)
	{
		g_dx11Device->mUserDefinedAnnotation->BeginEvent(mName);
	}
	~ScopedGpuEvent()
	{
		release();
	}
	void release()
	{
		if (!released)
		{
			released = true;
			g_dx11Device->mUserDefinedAnnotation->EndEvent();
		}
	}
private:
	ScopedGpuEvent() = delete;
	ScopedGpuEvent(ScopedGpuEvent&) = delete;
	LPCWSTR mName;
	bool released = false;
};
#define GPU_SCOPED_EVENT(timerName) ScopedGpuEvent gpuEvent##timerName##(L""#timerName)


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


class RenderBuffer
{
public:

	RenderBuffer(D3D11_BUFFER_DESC& mBufferDesc, void* initialData=nullptr);
	virtual ~RenderBuffer();


	// Usage of dynamic resources and mapping

	struct ScopedMappedRenderbuffer
	{
		ScopedMappedRenderbuffer()
			: mMappedBuffer(nullptr)
		{}
		~ScopedMappedRenderbuffer()
		{ RenderBuffer::unmap(*this); }

		void* getDataPtr() { return mMappedResource.pData; }
	private:
		friend class RenderBuffer;
		D3D11_MAPPED_SUBRESOURCE mMappedResource;
		ID3D11Buffer* mMappedBuffer;
	};
	void map(D3D11_MAP map, ScopedMappedRenderbuffer& mappedBuffer);
	static void unmap(ScopedMappedRenderbuffer& mappedBuffer);


	// Some basic descriptor initialisation methods

	static void initConstantBufferDesc_dynamic(D3D11_BUFFER_DESC& desc, UINT byteSize);
	static void initVertexBufferDesc_default(D3D11_BUFFER_DESC& desc, UINT byteSize);
	static void initIndexBufferDesc_default(D3D11_BUFFER_DESC& desc, UINT byteSize);
	static void initBufferDesc_default(D3D11_BUFFER_DESC& desc, UINT byteSize);
	static void initBufferDesc_uav(D3D11_BUFFER_DESC& desc, UINT byteSize);

public:///////////////////////////////////protected:
	D3D11_BUFFER_DESC mDesc;
	ID3D11Buffer* mBuffer;

private:
	RenderBuffer();
	RenderBuffer(RenderBuffer&);
};


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


class Texture2D
{
public:
	Texture2D(D3D11_TEXTURE2D_DESC& desc);
	virtual ~Texture2D();
	static void initDepthStencilBuffer(D3D11_TEXTURE2D_DESC& desc, UINT width, UINT height, bool uav);
	static void initDefault(D3D11_TEXTURE2D_DESC& desc, DXGI_FORMAT format, UINT width, UINT height, bool renderTarget, bool uav);
	D3D11_TEXTURE2D_DESC mDesc;
	ID3D11Texture2D* mTexture = nullptr;
	ID3D11DepthStencilView* mDepthStencilView = nullptr;
	ID3D11RenderTargetView* mRenderTargetView = nullptr;
	ID3D11ShaderResourceView* mShaderResourceView = nullptr;
	ID3D11UnorderedAccessView* mUnorderedAccessView = nullptr;
private:
	Texture2D();
	Texture2D(Texture2D&);
};

class Texture3D
{
public:
	Texture3D(D3D11_TEXTURE3D_DESC& desc);
	virtual ~Texture3D();
	static void initDefault(D3D11_TEXTURE3D_DESC& desc, DXGI_FORMAT format, UINT width, UINT height, UINT depth, bool uav);
	D3D11_TEXTURE3D_DESC mDesc;
	ID3D11Texture3D* mTexture = nullptr;
	ID3D11ShaderResourceView* mShaderResourceView = nullptr;			// level 0
	ID3D11UnorderedAccessView* mUnorderedAccessView = nullptr;			// level 0
	std::vector<ID3D11ShaderResourceView*> mShaderResourceViewMips;		// all levels
	std::vector<ID3D11UnorderedAccessView*> mUnorderedAccessViewMips;	// all levels
private:
	Texture3D();
	Texture3D(Texture2D&);
};

class SamplerState
{
public:
	SamplerState(D3D11_SAMPLER_DESC& desc);
	virtual ~SamplerState();
	static void initLinearClamp(D3D11_SAMPLER_DESC& desc);
	static void initShadowCmpClamp(D3D11_SAMPLER_DESC& desc);
	ID3D11SamplerState* mSampler = nullptr;
private:
	SamplerState();
	SamplerState(SamplerState&);
};


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


class DepthStencilState
{
public:
	DepthStencilState(D3D11_DEPTH_STENCIL_DESC& desc);
	virtual ~DepthStencilState();
	static void initDefaultDepthOnStencilOff(D3D11_DEPTH_STENCIL_DESC& desc);
	ID3D11DepthStencilState* mState;
private:
	DepthStencilState();
	DepthStencilState(DepthStencilState&);
};

class RasterizerState
{
public:
	RasterizerState(D3D11_RASTERIZER_DESC& desc);
	virtual ~RasterizerState();
	static void initDefaultState(D3D11_RASTERIZER_DESC& desc);
	ID3D11RasterizerState* mState;
private:
	RasterizerState();
	RasterizerState(RasterizerState&);
};

class BlendState
{
public:
	BlendState(D3D11_BLEND_DESC & desc);
	virtual ~BlendState();
	static void initDisabledState(D3D11_BLEND_DESC & desc);
	static void initAdditiveState(D3D11_BLEND_DESC & desc);
	ID3D11BlendState* mState;
private:
	BlendState();
	BlendState(BlendState&);
};


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


typedef std::vector<D3D11_INPUT_ELEMENT_DESC> InputLayoutDescriptors;

// Append a simple per vertex data layout input 
void appendSimpleVertexDataToInputLayout(InputLayoutDescriptors& inputLayout, const char* semanticName, DXGI_FORMAT format);

// Semantic names: https://msdn.microsoft.com/en-us/library/windows/desktop/bb509647(v=vs.85).aspx


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


class ShaderBase
{
public:
	ShaderBase(const TCHAR* filename, const char* entryFunction, const char* profile);
	virtual ~ShaderBase();

protected:
	ID3D10Blob* mShaderBuffer;

private:
	ShaderBase();
	ShaderBase(ShaderBase&);
};
// TODO use ComPtr

class VertexShader : public ShaderBase
{
public:
	VertexShader(const TCHAR* filename, const char* entryFunction);
	virtual ~VertexShader();
	void createInputLayout(InputLayoutDescriptors inputLayout, ID3D11InputLayout** layout);	// abstract that better
public:///////////////////////////////////protected:
	ID3D11VertexShader* mVertexShader;
};

class PixelShader : public ShaderBase
{
public:
	PixelShader(const TCHAR* filename, const char* entryFunction);
	virtual ~PixelShader();
public:///////////////////////////////////protected:
	ID3D11PixelShader* mPixelShader;
};

class HullShader : public ShaderBase
{
public:
	HullShader(const TCHAR* filename, const char* entryFunction);
	virtual ~HullShader();
public:///////////////////////////////////protected:
	ID3D11HullShader* mHullShader;
};

class DomainShader : public ShaderBase
{
public:
	DomainShader(const TCHAR* filename, const char* entryFunction);
	virtual ~DomainShader();
public:///////////////////////////////////protected:
	ID3D11DomainShader* mDomainShader;
};

class GeometryShader : public ShaderBase
{
public:
	GeometryShader(const TCHAR* filename, const char* entryFunction);
	virtual ~GeometryShader();
public:///////////////////////////////////protected:
	ID3D11GeometryShader* mGeometryShader;
};

class ComputeShader : public ShaderBase
{
public:
	ComputeShader(const TCHAR* filename, const char* entryFunction);
	virtual ~ComputeShader();
public:///////////////////////////////////protected:
	ID3D11ComputeShader* mComputeShader;
};



////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/// Example with double buffering on why we should use at least 3 timer query in this case (2 still works)
///  2: current frame added to context commend buffer
///  1: frame currently in flight
///  0: frame previous to current one in flight, done, data should be available
#define V_GPU_TIMER_FRAMECOUNT 3

/// Maximum number of timer in a frame and timer graph
#define V_TIMER_MAX_COUNT 128

class DxGpuPerformance
{
private:
	struct DxGpuTimer;
public:
	struct TimerGraphNode;

	static void initialise();
	static void shutdown();

	/// Limitation as of today: each timer in a single frame must have different names
	static void startGpuTimer(const char* name, unsigned char r, unsigned char g, unsigned char  b);
	static void endGpuTimer(const char* name);

	/// To call when starting to build render commands
	static void startFrame();
	/// To call after the back buffer swap
	static void endFrame();

	// TODO: add a reset function for when resources needs to all be re-allocated

	/// Some public structure that can be used to print out a frame timer graph
	struct TimerGraphNode
	{
		std::string name;
		float r, g, b;
		DxGpuTimer* timer = nullptr;
		std::vector<TimerGraphNode*> subGraph;	// will result in lots of allocations but that will do for now...
		TimerGraphNode* parent = nullptr;

		// Converted and extracted data
		float mBeginMs;
		float mEndMs;
		float mLastDurationMs;

	private:
		friend class DxGpuPerformance;
		// A bit too much to store all that raw data but convenient for now
		UINT64 mBeginTick;
		UINT64 mEndTick;
		UINT64 mLastDurationTick;
		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
	};
	typedef std::vector<TimerGraphNode*> GpuTimerGraph;

	/// Returns the root node of the performance timer graph. This first root node contains nothing valid appart from childrens sub graphs.
	static const TimerGraphNode* getLastUpdatedTimerGraphRootNode();

private:
	DxGpuPerformance() = delete;
	DxGpuPerformance(DxGpuPerformance&) = delete;

	/// The actual timer data storing the dx11 queries
	struct DxGpuTimer
	{
		DxGpuTimer();
		~DxGpuTimer();

		void initialize();
		void release();

		ID3D11Query* mDisjointQueries[V_GPU_TIMER_FRAMECOUNT];
		ID3D11Query* mBeginQueries[V_GPU_TIMER_FRAMECOUNT];
		ID3D11Query* mEndQueries[V_GPU_TIMER_FRAMECOUNT];

		/// The graph node associated to this timer
		TimerGraphNode* mNode[V_GPU_TIMER_FRAMECOUNT];

		bool mUsedThisFrame = false;	///! should be checked before querying data in case it is not used in some frames (also we only support one timer per name)
		bool mEnded = false;			///! sanity check to make sure a stared element is ended
	};

	typedef std::map<std::string, DxGpuTimer*> GpuTimerMap;
	static GpuTimerMap mTimers;			///! All the timers mapped using their name
	static int mMeasureTimerFrameId;	///! Last measured frame (appended timer to command buffer)
	static int mReadTimerFrameId;		///! Last frame we read the timer values from the api
	static int mLastReadTimerFrameId;	///! Last frame we read the timers and they are still valid for debug print on screen (data from previous finished frame)

	// Double buffer so that we can display the previous frame timers while the current frame is being processed
	static GpuTimerGraph mTimerGraphs[V_GPU_TIMER_FRAMECOUNT];	///! Timer graphs of the last frames
	static TimerGraphNode* mCurrentTimeGraph;					///! The current graph being filled up this frame

	// Basically, node object are not in container as this can result in invalid/stale pointer when reallocated. Instead we allocate in static arrays
	// and container point to the static array. With such an approach, all pointers will remain valid over the desired lifetime (several frames).
	static DxGpuTimer mTimerArray[V_TIMER_MAX_COUNT];
	static int mAllocatedTimers;
	static TimerGraphNode mTimerGraphNodeArray[V_GPU_TIMER_FRAMECOUNT][V_TIMER_MAX_COUNT];
	static int mAllocatedTimerGraphNodes[V_GPU_TIMER_FRAMECOUNT];
};

struct ScopedGpuTimer
{
	ScopedGpuTimer(const char* name, unsigned char r, unsigned char g, unsigned char b)
		: mName(name)
	{
		DxGpuPerformance::startGpuTimer(mName, r, g, b);
	}
	~ScopedGpuTimer()
	{
		release();
	}
	void release()
	{
		if (!released)
		{
			released = true;
			DxGpuPerformance::endGpuTimer(mName);
		}
	}
private:
	ScopedGpuTimer() = delete;
	ScopedGpuTimer(ScopedGpuTimer&) = delete;
	const char* mName;
	bool released = false;
};

#define GPU_SCOPED_TIMER(timerName, r, g, b) ScopedGpuTimer gpuTimer##timerName##(#timerName, r, g, b)
#define GPU_SCOPED_TIMEREVENT(teName, r, g, b) GPU_SCOPED_EVENT(teName);GPU_SCOPED_TIMER(teName, r, g, b);

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////





