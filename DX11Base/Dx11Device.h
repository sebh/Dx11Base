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
	D3D11_BUFFER_DESC mBufferDesc;
	ID3D11Buffer* mBuffer;

private:
	RenderBuffer();
	RenderBuffer(RenderBuffer&);
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
private:
	ID3D11HullShader* mHullShader;
};

class DomainShader : public ShaderBase
{
public:
	DomainShader(const TCHAR* filename, const char* entryFunction);
	virtual ~DomainShader();
private:
	ID3D11DomainShader* mDomainShader;
};

class GeometryShader : public ShaderBase
{
public:
	GeometryShader(const TCHAR* filename, const char* entryFunction);
	virtual ~GeometryShader();
private:
	ID3D11GeometryShader* mGeometryShader;
};

class ComputeShader : public ShaderBase
{
public:
	ComputeShader(const TCHAR* filename, const char* entryFunction);
	virtual ~ComputeShader();
private:
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
		TimerGraphNode* mNode = nullptr;// TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO this should also be using V_GPU_TIMER_FRAMECOUNT, right now the last read frame matches this behavior

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





