#include "Graphics.h"
#include "CreateModule.h"
#include "IndexBuffer.h"
#include "Program.h"
#include "Sampler.h"
#include "Texture1DResource.h"
#include "Texture2DResource.h"
#include "Texture3DResource.h"
#include "VertexBuffer.h"
#include "base/Application.h"
#include "modules/graphics/GraphicsAdapter.h"

namespace aurora::modules::graphics::win_d3d11 {
	Graphics::Graphics(Application* app) :
		_app(app),
		_refreshRate({0, 1}),
		_featureLevel(D3D_FEATURE_LEVEL_9_1),
		//_driverType(D3D_DRIVER_TYPE_NULL),
		_device(nullptr),
		_context(nullptr),
		_swapChain(nullptr),
		_backBufferTarget(nullptr),
		_deviceFeatures({ 0 }),
		_resizedListener(this, &Graphics::_resizedHandler) {
		_app.get()->getEventDispatcher().addEventListener(ApplicationEvent::RESIZED, _resizedListener, false);
		_constantBufferManager.createShareConstantBufferCallback = std::bind(&Graphics::_createdShareConstantBuffer, this);
		_constantBufferManager.createExclusiveConstantBufferCallback = std::bind(&Graphics::_createdExclusiveConstantBuffer, this, std::placeholders::_1);
	}

	Graphics::~Graphics() {
		_app.get()->getEventDispatcher().removeEventListener(ApplicationEvent::RESIZED, _resizedListener);
		_release();
	}

	bool Graphics::createDevice(const GraphicsAdapter* adapter) {
		if (_device || !_app.get()->Win_getHWnd()) return false;

		if (adapter) {
			return _createDevice(*adapter);
		} else {
			std::vector<GraphicsAdapter> adapters;
			GraphicsAdapter::query(adapters);
			std::vector<ui32> indices;
			GraphicsAdapter::autoSort(adapters, indices);

			for (auto& idx : indices) {
				if (_createDevice(adapters[idx])) return true;
			}
			return false;
		}
	}

	bool Graphics::_createDevice(const GraphicsAdapter& adapter) {
		DXObjGuard objs;

		IDXGIFactory2* dxgFctory = nullptr;

		if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory2), (void**)&dxgFctory))) return false;
		objs.add(dxgFctory);
		dxgFctory->MakeWindowAssociation(_app.get()->Win_getHWnd(), DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);

		IDXGIAdapter* dxgAdapter = nullptr;
		for (UINT i = 0;; ++i) {
			if (dxgFctory->EnumAdapters(i, &dxgAdapter) == DXGI_ERROR_NOT_FOUND) break;
			objs.add(dxgAdapter);

			DXGI_ADAPTER_DESC desc;
			memset(&desc, 0, sizeof(DXGI_ADAPTER_DESC));
			if (FAILED(dxgAdapter->GetDesc(&desc)) || desc.DeviceId != adapter.deviceId || desc.VendorId != adapter.vendorId) {
				dxgAdapter = nullptr;
				continue;
			} else {
				break;
			}
		}

		if (!dxgAdapter) return false;

		DXGI_FORMAT fmt = DXGI_FORMAT_R8G8B8A8_UNORM;

		ui32 maxResolutionArea = 0;
		f32 maxRefreshRate = 0.f;
		for (UINT i = 0;; ++i) {
			IDXGIOutput* output = nullptr;
			if (dxgAdapter->EnumOutputs(i, &output) == DXGI_ERROR_NOT_FOUND) break;
			objs.add(output);

			ui32 numSupportedModes = 0;
			if (FAILED(output->GetDisplayModeList(fmt, 0, &numSupportedModes, nullptr))) continue;

			auto supportedModes = new DXGI_MODE_DESC[numSupportedModes];
			memset(supportedModes, 0, sizeof(DXGI_MODE_DESC) * numSupportedModes);
			if (FAILED(output->GetDisplayModeList(fmt, 0, &numSupportedModes, supportedModes))) {
				delete[] supportedModes;
				continue;
			}

			for (ui32 i = 0; i < numSupportedModes; ++i) {
				auto& m = supportedModes[i];
				if (ui32 area = m.Width * m.Height; maxResolutionArea < area) {
					maxResolutionArea = area;
					_refreshRate.Numerator = 0;
					_refreshRate.Denominator = 1;
					maxRefreshRate = (f32)m.RefreshRate.Numerator / (f32)m.RefreshRate.Denominator;
				} else if (maxResolutionArea == area) {
					f32 rr = (f32)m.RefreshRate.Numerator / (f32)m.RefreshRate.Denominator;
					if (rr > maxRefreshRate) {
						maxRefreshRate = rr;

						_refreshRate.Numerator = m.RefreshRate.Numerator;
						_refreshRate.Denominator = m.RefreshRate.Denominator;
					}
				}
			}

			delete[] supportedModes;
		}

		if (maxResolutionArea == 0) return false;

		/*
		D3D_DRIVER_TYPE driverTypes[] =
		{
		   D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP,
		   D3D_DRIVER_TYPE_REFERENCE, D3D_DRIVER_TYPE_SOFTWARE
		};
		ui32 totalDriverTypes = ARRAYSIZE(driverTypes);
		*/

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0
		};
		ui32 totalFeatureLevels = ARRAYSIZE(featureLevels);

		ui32 creationFlags = 0;
#ifdef AE_DEBUG
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		/*
		for (ui32 i = 0; i < totalDriverTypes; ++i) {
			auto hr = D3D11CreateDevice(dxgAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, creationFlags,
				featureLevels, totalFeatureLevels,
				D3D11_SDK_VERSION, (ID3D11Device**)&_device, &_featureLevel, (ID3D11DeviceContext**)&_context);

			if (SUCCEEDED(hr)) {
				_driverType = driverTypes[i];
				break;
			}
		}
		*/

		if (FAILED(D3D11CreateDevice(dxgAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, creationFlags,
			featureLevels, totalFeatureLevels,
			D3D11_SDK_VERSION, (ID3D11Device**)&_device, nullptr, (ID3D11DeviceContext**)&_context))) {
			_release();
			return false;
		}


		//_driverType = D3D_DRIVER_TYPE_UNKNOWN;

		if (!_device) {
			_release();
			return false;
		}

		_deviceVersion = "D3D ";
		_featureLevel = _device->GetFeatureLevel();
		switch (_featureLevel) {
		case D3D_FEATURE_LEVEL_9_1:
		{
			_deviceVersion += "9.1";
			_shaderModel = "4.0.level.9.1";

			break;
		}
		case D3D_FEATURE_LEVEL_9_2:
		{
			_deviceVersion += "9.2";
			_shaderModel = "4.0.level.9.1";

			break;
		}
		case D3D_FEATURE_LEVEL_9_3:
		{
			_deviceVersion += "9.3";
			_shaderModel = "4.0.level.9.3";//??

			break;
		}
		case D3D_FEATURE_LEVEL_10_0:
		{
			_deviceVersion += "10.0";
			_shaderModel = "4.0";

			break;
		}
		case D3D_FEATURE_LEVEL_10_1:
		{
			_deviceVersion += "10.1";
			_shaderModel = "4.1";

			break;
		}
		case D3D_FEATURE_LEVEL_11_0:
		{
			_deviceVersion += "11.0";
			_shaderModel = "5.0";

			break;
		}
		default:
		{
			_deviceVersion += "11.1";
			_shaderModel = "5.0";

			break;
		}
		}

		Vec2i32 size;
		_app.get()->getInnerSize(size);

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		memset(&swapChainDesc, 0, sizeof(swapChainDesc));
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = (UINT)size[0];
		swapChainDesc.BufferDesc.Height = (UINT)size[1];
		swapChainDesc.BufferDesc.Format = fmt;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = _refreshRate.Numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = _refreshRate.Denominator;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = _app.get()->Win_getHWnd();
		swapChainDesc.Windowed = true;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		if (FAILED(dxgFctory->CreateSwapChain(_device, &swapChainDesc, (IDXGISwapChain**)&_swapChain))) {
			_release();
			return false;
		}

		_device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS, &_internalFeatures, sizeof(_internalFeatures));

		_deviceFeatures.supportSampler = true;
		_deviceFeatures.supportTextureView = true;
		_deviceFeatures.supportConstantBuffer = true;
		_deviceFeatures.supportPersisientMap = false;

		_resize((Vec2<UINT>&)size);

		return true;
	}

	const std::string& Graphics::getVersion() const {
		return _deviceVersion;
	}

	const GraphicsDeviceFeatures& Graphics::getDeviceFeatures() const {
		return _deviceFeatures;
	}

	IConstantBuffer* Graphics::createConstantBuffer() {
		return new ConstantBuffer(*this);
	}

	IIndexBuffer* Graphics::createIndexBuffer() {
		return new IndexBuffer(*this);
	}

	IProgram* Graphics::createProgram() {
		return new Program(*this);
	}

	ISampler* Graphics::createSampler() {
		return new Sampler(*this);
	}

	ITexture1DResource* Graphics::createTexture1DResource() {
		return new Texture1DResource(*this);
	}

	ITexture2DResource* Graphics::createTexture2DResource() {
		return new Texture2DResource(*this);
	}

	ITexture3DResource* Graphics::createTexture3DResource() {
		return new Texture3DResource(*this);
	}

	ITextureView* Graphics::createTextureView() {
		return new TextureView(*this, false);
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

	IConstantBuffer* Graphics::_createdShareConstantBuffer() {
		return new ConstantBuffer(*this);
	}

	IConstantBuffer* Graphics::_createdExclusiveConstantBuffer(ui32 numParameters) {
		auto cb = new ConstantBuffer(*this);
		cb->recordUpdateIds = new ui32[numParameters];
		memset(cb->recordUpdateIds, 0, sizeof(ui32) * numParameters);
		return cb;
	}

	void Graphics::_resizedHandler(events::Event<ApplicationEvent>& e) {
		Vec2i32 size;
		_app.get()->getInnerSize(size);
		_resize((Vec2<UINT>&)size);
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

		_refreshRate.Numerator = 0;
		_refreshRate.Denominator = 1;

		_featureLevel = D3D_FEATURE_LEVEL_9_1;
		_shaderModel = "";

		memset(&_internalFeatures, 0, sizeof(_internalFeatures));
		memset(&_deviceFeatures, 0, sizeof(_deviceFeatures));

		_deviceVersion = "D3D Unknown";
	}

	void Graphics::_resize(const Vec2<UINT>& size) {
		if (!_swapChain) return;

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		_swapChain->GetDesc(&swapChainDesc);

		bool sizeChange = swapChainDesc.BufferDesc.Width != size[0] || swapChainDesc.BufferDesc.Height != size[1];

		if (sizeChange || !_backBufferTarget) {
			swapChainDesc.BufferDesc.Width = size[0];
			swapChainDesc.BufferDesc.Height = size[1];

			if (_backBufferTarget) {
				_backBufferTarget->Release();
				_backBufferTarget = nullptr;
			}

			if (sizeChange) {
				if (FAILED(_swapChain->ResizeBuffers(swapChainDesc.BufferCount, size[0], size[1], swapChainDesc.BufferDesc.Format, swapChainDesc.Flags))) {
					println("swap chain ResizeBuffers error");
				}
			}

			ID3D11Texture2D* backBufferTexture = nullptr;
			if (FAILED(_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture))) {
				println("swap chain GetBuffer error");
			}

			if (FAILED(_device->CreateRenderTargetView(backBufferTexture, nullptr, (ID3D11RenderTargetView**)&_backBufferTarget))) {
				if (backBufferTexture) backBufferTexture->Release();
				println("swap chain CreateRenderTargetView error");
			}
			backBufferTexture->Release();

			_context->OMSetRenderTargets(1, (ID3D11RenderTargetView**)&_backBufferTarget, nullptr);

			D3D11_VIEWPORT vp;
			vp.Width = (FLOAT)size[0];
			vp.Height = (FLOAT)size[1];
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0.0f;
			vp.TopLeftY = 0.0f;

			_context->RSSetViewports(1, &vp);

			InvalidateRect(nullptr, nullptr, TRUE);
		}
	}

	DXGI_FORMAT Graphics::convertInternalFormat(TextureFormat fmt) {
		switch (fmt) {
		case TextureFormat::R8G8B8A8:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}
}