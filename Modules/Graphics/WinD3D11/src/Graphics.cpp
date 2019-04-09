#include "Graphics.h"
#include "base/Application.h"
#include "CreateModule.h"

namespace aurora::modules::graphics::win_d3d11 {
	Graphics::Graphics(Application* app) :
		_app(app->ref<Application>()),
		_isFullscreen(false),
		_width(0),
		_height(0),
		_driverType(D3D_DRIVER_TYPE_NULL),
		_featureLevel(D3D_FEATURE_LEVEL_11_0),
		_device(nullptr),
		_context(nullptr),
		_swapChain(nullptr),
		_backBufferTarget(nullptr),
		_resizedListener(this, &Graphics::_resizedHandler),
		_fullscreenTogglingListener(this, &Graphics::_fullscreenTogglingHandler) {
		_app->getEventDispatcher().addEventListener(ApplicationEvent::RESIZED, _resizedListener, false);
		_app->getEventDispatcher().addEventListener(ApplicationEvent::FULLSCREEN_TOGGLING, _fullscreenTogglingListener, false);
	}

	Graphics::~Graphics() {
		_app->getEventDispatcher().removeEventListener(ApplicationEvent::RESIZED, _resizedListener);
		_app->getEventDispatcher().removeEventListener(ApplicationEvent::FULLSCREEN_TOGGLING, _fullscreenTogglingListener);
		_release();
		Ref::setNull(_app);
	}

	bool Graphics::createDevice() {
		if (_device) return false;

		D3D_DRIVER_TYPE driverTypes[] =
		{
		   D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP,
		   D3D_DRIVER_TYPE_REFERENCE, D3D_DRIVER_TYPE_SOFTWARE
		};
		ui32 totalDriverTypes = ARRAYSIZE(driverTypes);

		D3D_FEATURE_LEVEL featureLevels[] =
		{
		   D3D_FEATURE_LEVEL_11_0,
		   D3D_FEATURE_LEVEL_10_1,
		   D3D_FEATURE_LEVEL_10_0
		};
		ui32 totalFeatureLevels = ARRAYSIZE(featureLevels);

		i32 w, h;
		_app->getInnerSize(w, h);

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		memset(&swapChainDesc, 0, sizeof(swapChainDesc));
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = w;
		swapChainDesc.BufferDesc.Height = h;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = _app->Win_getHWND();
		swapChainDesc.Windowed = _app->isWindowed();
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		ui32 creationFlags = 0;
#ifdef AE_DEBUG
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT hr;

		for (ui32 i = 0; i < totalDriverTypes; ++i) {
			hr = D3D11CreateDeviceAndSwapChain(0, driverTypes[i], 0, creationFlags,
				featureLevels, totalFeatureLevels,
				D3D11_SDK_VERSION, &swapChainDesc, &_swapChain,
				&_device, &_featureLevel, &_context);

			if (SUCCEEDED(hr)) {
				_driverType = driverTypes[i];
				break;
			}
		}

		if (FAILED(hr)) {
			_release();
			return false;
		}

		///*
		IDXGIOutput* output = nullptr;
		if (FAILED(_swapChain->GetContainingOutput(&output))) {
			_release();
			return false;
		}

		ui32 numSupportedModes = 0;
		if (FAILED(output->GetDisplayModeList(swapChainDesc.BufferDesc.Format, 0, &numSupportedModes, nullptr))) {
			output->Release();
			_release();
			return false;
		}

		auto supportedModes = new DXGI_MODE_DESC[numSupportedModes];
		memset(supportedModes, 0, sizeof(DXGI_MODE_DESC) * numSupportedModes);
		if (FAILED(output->GetDisplayModeList(swapChainDesc.BufferDesc.Format, 0, &numSupportedModes, supportedModes))) {
			output->Release();
			_release();
			return false;
		}
		output->Release();

		f32 maxRefreshRate = 0.f;
		for (ui32 i = 0; i < numSupportedModes; ++i) {
			auto& m = supportedModes[i];
			f32 rr = (f32)m.RefreshRate.Numerator / (f32)m.RefreshRate.Denominator;
			if (rr > maxRefreshRate) {
				maxRefreshRate = rr;
				_refreshRate.Numerator = m.RefreshRate.Numerator;
				_refreshRate.Denominator = m.RefreshRate.Denominator;
			}
			//println("%d  %d  %d  %d", m->Width, m->Height, m->RefreshRate.Numerator, m->RefreshRate.Denominator);
		}

		delete[] supportedModes;

		IDXGIDevice * DXGDevice;
		if (FAILED(_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&DXGDevice))) {
			_release();
			return false;
		}
		IDXGIAdapter * DXGAdapter;
		if (FAILED(DXGDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&DXGAdapter))) {
			DXGDevice->Release();
			_release();
			return false;
		}
		IDXGIFactory * DXGFactory;
		if (FAILED(DXGAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&DXGFactory))) {
			DXGAdapter->Release();
			DXGDevice->Release();
			_release();
			return false;
		}
		DXGFactory->MakeWindowAssociation(_app->Win_getHWND(), DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
		DXGFactory->Release();
		DXGAdapter->Release();
		DXGDevice->Release();

		_isFullscreen = _app->isWindowed();
		_toggleFullscreen(!_app->isWindowed(), w, h);
		_resize(w, h);

		return true;
	}

	IIndexBuffer* Graphics::createIndexBuffer() {
		return new IndexBuffer(*this);
	}

	IProgram* Graphics::createProgram() {
		return new Program(*this);
	}

	IVertexBuffer* Graphics::createVertexBuffer() {
		return new VertexBuffer(*this);
	}

	void Graphics::beginRender() {
		f32 clearColor[] = { 0.0f, 0.0f, 0.25f, 1.0f };
		_context->ClearRenderTargetView(_backBufferTarget, clearColor);
	}

	void Graphics::endRender() {
	}

	void Graphics::present() {
		_swapChain->Present(0, 0);
	}

	void Graphics::clear() {
	}

	void Graphics::_resizedHandler(events::Event<ApplicationEvent>& e) {
		i32 w, h;
		_app->getInnerSize(w, h);
		_resize(w, h);
	}

	void Graphics::_fullscreenTogglingHandler(events::Event<ApplicationEvent>& e) {
		auto wh = (i32*)e.getData();
		//auto hr = _swapChain->SetFullscreenState(!_app->isWindowed(), nullptr);
		_toggleFullscreen(!_app->isWindowed(), wh[0], wh[1]);
	}

	void Graphics::_release() {
		if (_backBufferTarget) {
			_backBufferTarget->Release();
			_backBufferTarget = nullptr;
		}
		if (_swapChain) {
			_swapChain->Release();
			_swapChain = nullptr;
		}
		if (_context) {
			_context->Release();
			_context = nullptr;
		}
		if (_device) {
			_device->Release();
			_device = nullptr;
		}
	}

	/*
	void Graphics::_resize(bool fullscreen, UINT w, UINT h) {
		if (!_swapChain || _sizeChanging) return;

		bool fullscreenChange = _isFullscreen != fullscreen;
		if (!fullscreenChange && _width == w && _height == h) return;
		_isFullscreen = fullscreen;
		_width = w;
		_height = h;
		_sizeChanging = true;

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		_swapChain->GetDesc(&swapChainDesc);

		if (fullscreenChange) {
			DXGI_MODE_DESC mode;
			memset(&mode, 0, sizeof(mode));

			mode.Format = swapChainDesc.BufferDesc.Format;
			mode.Width = w;
			mode.Height = h;
			mode.RefreshRate.Numerator = _refreshRate.Numerator;
			mode.RefreshRate.Denominator = _refreshRate.Denominator;
			mode.ScanlineOrdering = swapChainDesc.BufferDesc.ScanlineOrdering;
			mode.Scaling = swapChainDesc.BufferDesc.Scaling;

			println("1");
			//_swapChain->ResizeTarget(&mode);

			println("2");
			auto hr = _swapChain->SetFullscreenState(fullscreen, nullptr);

			//mode.RefreshRate.Numerator = 0;
			//mode.RefreshRate.Denominator = 0;
			println("3");
			_swapChain->ResizeTarget(&mode);
		}

		if (_backBufferTarget) {
			_backBufferTarget->Release();
			_backBufferTarget = nullptr;
		}

		auto hr = _swapChain->ResizeBuffers(swapChainDesc.BufferCount, w, h, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);
		//if (fullscreenChange) _swapChain->SetFullscreenState(fullscreen, nullptr);

		ID3D11Texture2D* backBufferTexture;
		hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture);

		if (FAILED(hr)) {
			println("errr  1");
		}

		hr = _device->CreateRenderTargetView(backBufferTexture, nullptr, &_backBufferTarget);
		if (backBufferTexture) backBufferTexture->Release();

		if (FAILED(hr)) {
			println("errr  2");
		}

		_context->OMSetRenderTargets(1, &_backBufferTarget, nullptr);

		D3D11_VIEWPORT vp;
		vp.Width = f32(w);
		vp.Height = f32(h);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;

		_context->RSSetViewports(1, &vp);
		_sizeChanging = false;
		println("fuckkkkkkkkkkkkkkkk  full  %d   %d    %d", fullscreen, w, h);
	}
	*/

	void Graphics::_toggleFullscreen(bool fullscreen, UINT w, UINT h) {
		if (!_swapChain) return;

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		_swapChain->GetDesc(&swapChainDesc);

		DXGI_MODE_DESC mode;
		memset(&mode, 0, sizeof(mode));

		mode.Format = swapChainDesc.BufferDesc.Format;
		mode.Width = w;
		mode.Height = h;
		mode.RefreshRate.Numerator = _refreshRate.Numerator;
		mode.RefreshRate.Denominator = _refreshRate.Denominator;
		mode.ScanlineOrdering = swapChainDesc.BufferDesc.ScanlineOrdering;
		mode.Scaling = swapChainDesc.BufferDesc.Scaling;
		
		println("1");
		_swapChain->ResizeTarget(&mode);

		println("2");
		auto hr = _swapChain->SetFullscreenState(fullscreen, nullptr);

		mode.RefreshRate.Numerator = 0;
		mode.RefreshRate.Denominator = 0;
		println("3");
		_swapChain->ResizeTarget(&mode);
		
		println("screennnnnnnnnnnnnnnn  full  %d   %d    %d", fullscreen, w, h);
	}

	void Graphics::_resize(UINT w, UINT h) {
		if (!_swapChain) return;

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		_swapChain->GetDesc(&swapChainDesc);

		if (_backBufferTarget) {
			_backBufferTarget->Release();
			_backBufferTarget = nullptr;
		}

		auto hr = _swapChain->ResizeBuffers(swapChainDesc.BufferCount, w, h, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);
		//if (fullscreenChange) _swapChain->SetFullscreenState(fullscreen, nullptr);

		ID3D11Texture2D* backBufferTexture;
		hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture);

		if (FAILED(hr)) {
			println("errr  1");
		}

		hr = _device->CreateRenderTargetView(backBufferTexture, nullptr, &_backBufferTarget);
		if (backBufferTexture) backBufferTexture->Release();

		if (FAILED(hr)) {
			println("errr  2");
		}

		_context->OMSetRenderTargets(1, &_backBufferTarget, nullptr);

		D3D11_VIEWPORT vp;
		vp.Width = f32(w);
		vp.Height = f32(h);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;

		_context->RSSetViewports(1, &vp);
		_sizeChanging = false;
		println("resizeeeeeeeeeeee  full  %d   %d    %d", !_app->isWindowed(), w, h);
	}
}