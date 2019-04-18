#include "Graphics.h"
#include "IndexBuffer.h"
#include "Program.h"
#include "VertexBuffer.h"
#include "base/Application.h"
#include "CreateModule.h"
#include <d3d11shader.h>

namespace aurora::modules::graphics::win_d3d11 {
	Graphics::Graphics(Application* app) :
		_app(app->ref<Application>()),
		_refreshRate({0, 1}),
		_featureLevel(D3D_FEATURE_LEVEL_9_1),
		//_driverType(D3D_DRIVER_TYPE_NULL),
		_device(nullptr),
		_context(nullptr),
		_swapChain(nullptr),
		_backBufferTarget(nullptr),
		_resizedListener(this, &Graphics::_resizedHandler) {
		_app->getEventDispatcher().addEventListener(ApplicationEvent::RESIZED, _resizedListener, false);
	}

	Graphics::~Graphics() {
		_app->getEventDispatcher().removeEventListener(ApplicationEvent::RESIZED, _resizedListener);
		_release();
		Ref::setNull(_app);
	}

	bool Graphics::createDevice(const GraphicsAdapter* adapter) {
		if (_device || !_app->Win_getHWND()) return false;

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
		dxgFctory->MakeWindowAssociation(_app->Win_getHWND(), DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);

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
			auto fiwehfoew = dxgAdapter->EnumOutputs(i, &output);
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
				ui32 area = m.Width * m.Height;
				if (maxResolutionArea < area) {
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

		_featureLevel = _device->GetFeatureLevel();
		switch (_featureLevel) {
		case D3D_FEATURE_LEVEL_9_1:
		case D3D_FEATURE_LEVEL_9_2:
			_shaderModel = "4.0.level.9.1";
			break;
		case D3D_FEATURE_LEVEL_9_3:
			_shaderModel = "4.0.level.9.3";//??
			break;
		case D3D_FEATURE_LEVEL_10_0:
			_shaderModel = "4.0";
			break;
		case D3D_FEATURE_LEVEL_10_1:
			_shaderModel = "4.1";
			break;
		default:
			_shaderModel = "5.0";
			break;
		}

		i32 w, h;
		_app->getInnerSize(w, h);

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		memset(&swapChainDesc, 0, sizeof(swapChainDesc));
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = w;
		swapChainDesc.BufferDesc.Height = h;
		swapChainDesc.BufferDesc.Format = fmt;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = _refreshRate.Numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = _refreshRate.Denominator;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = _app->Win_getHWND();
		swapChainDesc.Windowed = true;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		if (FAILED(dxgFctory->CreateSwapChain(_device, &swapChainDesc, (IDXGISwapChain**)&_swapChain))) {
			_release();
			return false;
		}

		_resize(w, h);

		return true;
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

	void Graphics::refShareConstantBuffer(ui32 size) {
		auto itr = _sharedConstBufferPool.find(size);
		if (itr == _sharedConstBufferPool.end()) {
			_sharedConstBufferPool.emplace(std::piecewise_construct, std::forward_as_tuple(size), std::forward_as_tuple()).first->second.rc = 1;
		} else {
			++itr->second.rc;
		}
	}

	void Graphics::unrefShareConstantBuffer(ui32 size) {
		auto itr = _sharedConstBufferPool.find(size);
		if (itr != _sharedConstBufferPool.end()) {
			if (itr->second.rc == 1) _sharedConstBufferPool.erase(itr);
		}
	}

	ConstantBuffer* Graphics::popShareConstantBuffer(ui32 size) {
		auto itr = _sharedConstBufferPool.find(size);
		if (itr != _sharedConstBufferPool.end()) {
			auto& buffers = itr->second.buffers;
			auto len = buffers.size();
			ConstantBuffer* cb;
			if (len == 0) {
				cb = new ConstantBuffer(*this);
				cb->ref();
				cb->stroage(size);
			} else {
				cb = buffers[len - 1];
				buffers.pop_back();
			}

			return cb;
		}
		return nullptr;
	}

	void Graphics::pushShareConstantBuffer(ui32 size, ConstantBuffer& cb) {
		auto itr = _sharedConstBufferPool.find(size);
		if (itr != _sharedConstBufferPool.end()) itr->second.buffers.emplace_back(&cb);
	}

	void Graphics::_resizedHandler(events::Event<ApplicationEvent>& e) {
		i32 w, h;
		_app->getInnerSize(w, h);
		_resize(w, h);
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
	}

	void Graphics::_resize(UINT w, UINT h) {
		if (!_swapChain) return;

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		_swapChain->GetDesc(&swapChainDesc);

		bool sizeChange = swapChainDesc.BufferDesc.Width != w || swapChainDesc.BufferDesc.Height != h;

		if (sizeChange || !_backBufferTarget) {
			swapChainDesc.BufferDesc.Width = w;
			swapChainDesc.BufferDesc.Height = h;

			if (_backBufferTarget) {
				_backBufferTarget->Release();
				_backBufferTarget = nullptr;
			}

			if (sizeChange) {
				if (FAILED(_swapChain->ResizeBuffers(swapChainDesc.BufferCount, w, h, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags))) {
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
			vp.Width = f32(w);
			vp.Height = f32(h);
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0.0f;
			vp.TopLeftY = 0.0f;

			_context->RSSetViewports(1, &vp);

			InvalidateRect(nullptr, nullptr, TRUE);
		}
	}
}