#include "Graphics.h"
#include "CreateModule.h"
#include "BlendState.h"
#include "IndexBuffer.h"
#include "Program.h"
#include "RasterizerState.h"
#include "Sampler.h"
#include "Texture1DResource.h"
#include "Texture2DResource.h"
#include "Texture3DResource.h"
#include "VertexBuffer.h"
#include "base/Application.h"
#include "modules/graphics/GraphicsAdapter.h"

namespace aurora::modules::graphics::win_d3d11 {
	Graphics::Graphics(Ref* loader, Application* app) :
		_loader(loader),
		_app(app),
		_refreshRate({0, 1}),
		_featureLevel(D3D_FEATURE_LEVEL_9_1),
		//_driverType(D3D_DRIVER_TYPE_NULL),
		_device(nullptr),
		_context(nullptr),
		_swapChain(nullptr),
		_backBufferTarget(nullptr),
		_deviceFeatures({ 0 }),
		_d3dStatus({ 0 }),
		_resizedListener(this, &Graphics::_resizedHandler) {
		_resizedListener.ref();
		_app->getEventDispatcher().addEventListener(ApplicationEvent::RESIZED, _resizedListener);
		_constantBufferManager.createShareConstantBufferCallback = std::bind(&Graphics::_createdShareConstantBuffer, this);
		_constantBufferManager.createExclusiveConstantBufferCallback = std::bind(&Graphics::_createdExclusiveConstantBuffer, this, std::placeholders::_1);
	}

	Graphics::~Graphics() {
		_app->getEventDispatcher().removeEventListener(ApplicationEvent::RESIZED, _resizedListener);
		_release();
	}

	bool Graphics::createDevice(const GraphicsAdapter* adapter) {
		if (_device || !_app->Win_getHWnd()) return false;

		if (adapter) {
			return _createDevice(*adapter);
		} else {
			std::vector<GraphicsAdapter> adapters;
			GraphicsAdapter::query(adapters);
			std::vector<uint32_t> indices;
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
		dxgFctory->MakeWindowAssociation(_app->Win_getHWnd(), DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);

		IDXGIAdapter* dxgAdapter = nullptr;
		for (UINT i = 0;; ++i) {
			if (dxgFctory->EnumAdapters(i, &dxgAdapter) == DXGI_ERROR_NOT_FOUND) break;
			objs.add(dxgAdapter);

			DXGI_ADAPTER_DESC desc = { 0 };
			if (FAILED(dxgAdapter->GetDesc(&desc)) || desc.DeviceId != adapter.deviceId || desc.VendorId != adapter.vendorId) {
				dxgAdapter = nullptr;
				continue;
			} else {
				break;
			}
		}

		if (!dxgAdapter) return false;

		DXGI_FORMAT fmt = DXGI_FORMAT_R8G8B8A8_UNORM;

		uint32_t maxResolutionArea = 0;
		f32 maxRefreshRate = 0.f;
		for (UINT i = 0;; ++i) {
			IDXGIOutput* output = nullptr;
			if (dxgAdapter->EnumOutputs(i, &output) == DXGI_ERROR_NOT_FOUND) break;
			objs.add(output);

			uint32_t numSupportedModes = 0;
			if (FAILED(output->GetDisplayModeList(fmt, 0, &numSupportedModes, nullptr))) continue;

			auto supportedModes = new DXGI_MODE_DESC[numSupportedModes];
			memset(supportedModes, 0, sizeof(DXGI_MODE_DESC) * numSupportedModes);
			if (FAILED(output->GetDisplayModeList(fmt, 0, &numSupportedModes, supportedModes))) {
				delete[] supportedModes;
				continue;
			}

			for (uint32_t i = 0; i < numSupportedModes; ++i) {
				auto& m = supportedModes[i];
				if  (uint32_t area = m.Width * m.Height; maxResolutionArea < area) {
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
		uint32_t totalDriverTypes = ARRAYSIZE(driverTypes);
		*/

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0
		};
		uint32_t totalFeatureLevels = ARRAYSIZE(featureLevels);

		uint32_t creationFlags = 0;
#ifdef AE_DEBUG
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		/*
		for (uint32_t i = 0; i < totalDriverTypes; ++i) {
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
		_app->getInnerSize(size);

		DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = (UINT)size[0];
		swapChainDesc.BufferDesc.Height = (UINT)size[1];
		swapChainDesc.BufferDesc.Format = fmt;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = _refreshRate.Numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = _refreshRate.Denominator;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = _app->Win_getHWnd();
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
		_deviceFeatures.supportPixelBuffer = false;
		_deviceFeatures.supportConstantBuffer = true;
		_deviceFeatures.supportPersistentMap = false;
		_deviceFeatures.supportIndependentBlend = true;

		_resize((Vec2<UINT>&)size);

		return true;
	}

	const std::string& Graphics::getVersion() const {
		return _deviceVersion;
	}

	const GraphicsDeviceFeatures& Graphics::getDeviceFeatures() const {
		return _deviceFeatures;
	}

	IBlendState* Graphics::createBlendState() {
		return new BlendState(*this);
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

	IRasterizerState* Graphics::createRasterizerState() {
		return new RasterizerState(*this);
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

	IPixelBuffer* Graphics::createPixelBuffer() {
		return nullptr;
	}

	void Graphics::setBlendState(IBlendState* state, const Vec4f32& constantFactors, uint32_t sampleMask) {
		if (state && state->getGraphics() == this) {
			auto bs = (BlendState*)state;
			bs->update();
			auto& blend = _d3dStatus.blend;
			if (auto internalState = bs->getInternalState(); internalState &&
				(blend.featureValue != bs->getFeatureValue() || !memEqual<sizeof(blend.constantFactors)>(&blend.constantFactors, &constantFactors) || blend.sampleMask != sampleMask)) {
				blend.featureValue = bs->getFeatureValue();
				blend.constantFactors.set(constantFactors);
				blend.sampleMask = sampleMask;

				_context->OMSetBlendState(internalState, constantFactors.data, sampleMask);
			}
		}
	}

	void Graphics::setRasterizerState(IRasterizerState* state) {
		if (state && state->getGraphics() == this) {
			auto rs = (RasterizerState*)state;
			rs->update();
			auto& rasterizer = _d3dStatus.rasterizer;
			if (auto internalState = rs->getInternalState(); internalState && rasterizer.featureValue != rs->getFeatureValue()) {
				rasterizer.featureValue = rs->getFeatureValue();

				_context->RSSetState(internalState);
			}
		}
	}

	void Graphics::beginRender() {
	}

	void Graphics::draw(const VertexBufferFactory* vertexFactory, IProgram* program, const ShaderParameterFactory* paramFactory, 
		const IIndexBuffer* indexBuffer, uint32_t count, uint32_t offset) {
		if (vertexFactory && indexBuffer && program && program->getGraphics() == this && indexBuffer->getGraphics() == this && count > 0) {
			auto ib = (IndexBuffer*)indexBuffer->getNativeBuffer();
			if (!ib) return;
			auto internalIndexBuffer = ib->getInternalBuffer();
			auto fmt = ib->getInternalFormat();
			if (!internalIndexBuffer || fmt == DXGI_FORMAT_UNKNOWN) return;
			auto numIndexElements = ib->getNumElements();
			if (!numIndexElements || offset >= numIndexElements) return;

			auto p = (Program*)program;
			if (p->use(vertexFactory, paramFactory)) {
				uint32_t last = numIndexElements - offset;
				if (count > numIndexElements) count = numIndexElements;
				if (count > last) count = last;
				_context->IASetIndexBuffer((ID3D11Buffer*)internalIndexBuffer, fmt, 0);
				_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				_context->DrawIndexed(count, offset, 0);

				p->useEnd();

				_constantBufferManager.resetUsedShareConstantBuffers();
			}
		}
	}

	void Graphics::endRender() {
	}

	void Graphics::present() {
		_swapChain->Present(0, 0);
	}

	void Graphics::clear(ClearFlag flag, const Vec4f32& color, f32 depth, size_t stencil) {
		if ((flag & ClearFlag::COLOR) != ClearFlag::NONE) _context->ClearRenderTargetView(_backBufferTarget, color.data);
	}

	IConstantBuffer* Graphics::_createdShareConstantBuffer() {
		return new ConstantBuffer(*this);
	}

	IConstantBuffer* Graphics::_createdExclusiveConstantBuffer(uint32_t numParameters) {
		auto cb = new ConstantBuffer(*this);
		cb->recordUpdateIds = new uint32_t[numParameters];
		memset(cb->recordUpdateIds, 0, sizeof(uint32_t) * numParameters);
		return cb;
	}

	void Graphics::_resizedHandler(events::Event<ApplicationEvent>& e) {
		Vec2i32 size;
		_app->getInnerSize(size);
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

		auto& bufferDesc = swapChainDesc.BufferDesc;

		bool sizeChange = bufferDesc.Width != size[0] || bufferDesc.Height != size[1];

		if (sizeChange || !_backBufferTarget) {
			bufferDesc.Width = size[0];
			bufferDesc.Height = size[1];

			if (_backBufferTarget) {
				_backBufferTarget->Release();
				_backBufferTarget = nullptr;
			}

			if (sizeChange) {
				if (FAILED(_swapChain->ResizeBuffers(swapChainDesc.BufferCount, size[0], size[1], bufferDesc.Format, swapChainDesc.Flags))) {
					println("dx11 error : swap chain ResizeBuffers error");
				}
			}

			ID3D11Texture2D* backBufferTexture = nullptr;
			if (FAILED(_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture))) {
				println("dx11 error : swap chain GetBuffer error");
			}

			if (backBufferTexture && FAILED(_device->CreateRenderTargetView(backBufferTexture, nullptr, (ID3D11RenderTargetView**)&_backBufferTarget))) {
				if (backBufferTexture) backBufferTexture->Release();
				println("dx11 error : swap chain CreateRenderTargetView error");
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