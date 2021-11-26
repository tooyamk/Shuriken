#include "Graphics.h"
#include "CreateModule.h"
#include "BlendState.h"
#include "DepthStencil.h"
#include "DepthStencilState.h"
#include "IndexBuffer.h"
#include "Program.h"
#include "RasterizerState.h"
#include "RenderTarget.h"
#include "RenderView.h"
#include "Sampler.h"
#include "Texture1DResource.h"
#include "Texture2DResource.h"
#include "Texture3DResource.h"
#include "TextureView.h"
#include "VertexBuffer.h"
#include "aurora/ProgramSource.h"
#include "aurora/modules/graphics/GraphicsAdapter.h"

namespace aurora::modules::graphics::d3d11 {
	Graphics::Graphics() :
		_isDebug(false),
		_curIsBackBuffer(true),
		_backBufferSampleCount(1),
		_refreshRate({0, 1}),
		_featureLevel(D3D_FEATURE_LEVEL_9_1),
		//_driverType(D3D_DRIVER_TYPE_NULL),
		_device(nullptr),
		_context(nullptr),
		_swapChain(nullptr),
		_backBufferView(nullptr),
		_backDepthStencil(nullptr),
		_d3dStatus({ 0 }),
		_numRTVs(0),
		_RTVs({ 0 }),
		_DSV(nullptr),
		_eventDispatcher(new events::EventDispatcher<GraphicsEvent>()) {
		_constantBufferManager.createShareConstantBufferCallback = std::bind(&Graphics::_createdShareConstantBuffer, this);
		_constantBufferManager.createExclusiveConstantBufferCallback = std::bind(&Graphics::_createdExclusiveConstantBuffer, this, std::placeholders::_1);
	}

	Graphics::~Graphics() {
		_release();
	}

	bool Graphics::createDevice(const CreateConfig& conf) {
		if (_device) return false;
		if (conf.app) {
			if (!conf.app->getNative(ApplicationNative::WINDOW)) return false;
		} else {
			if (!conf.offscreen) return false;
		}

		if (conf.adapter) {
			conf.createProcessInfo("specific adapter create device...");
			return _createDevice(conf);
		} else if (conf.offscreen) {
			conf.createProcessInfo("null adapter create offscreen device...");
			return _createDevice(conf);
		} else {
			conf.createProcessInfo("search adapter create device...");

			std::vector<GraphicsAdapter> adapters;
			GraphicsAdapter::query(adapters);
			std::vector<uint32_t> indices;
			GraphicsAdapter::autoSort(adapters, indices);

			auto conf2 = conf;

			for (auto& idx : indices) {
				conf2.adapter = &adapters[idx];
				conf.createProcessInfo("found adapter create device...");
				if (_createDevice(conf2)) return true;
			}

			conf.createProcessInfo("search adapter create device failed");

			return false;
		}
	}

	bool Graphics::_createDevice(const CreateConfig& conf) {
		_isDebug = conf.debug;

		DXObjGuard objs;

		IDXGIFactory2* dxgFctory = nullptr;
		IDXGIAdapter* dxgAdapter = nullptr;

		if (conf.adapter && (conf.app || !conf.offscreen)) {
			if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory2), (void**)&dxgFctory))) {
				conf.createProcessInfo("CreateDXGIFactory failed");
				return false;
			}
			objs.add(dxgFctory);
			dxgFctory->MakeWindowAssociation((HWND)conf.app->getNative(ApplicationNative::WINDOW), DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);

			for (UINT i = 0;; ++i) {
				if (dxgFctory->EnumAdapters(i, &dxgAdapter) == DXGI_ERROR_NOT_FOUND) break;
				objs.add(dxgAdapter);

				DXGI_ADAPTER_DESC desc = { 0 };
				if (FAILED(dxgAdapter->GetDesc(&desc)) || desc.DeviceId != conf.adapter->deviceId || desc.VendorId != conf.adapter->vendorId) {
					dxgAdapter = nullptr;
					continue;
				} else {
					break;
				}
			}
		}

		if (!dxgAdapter && !conf.offscreen) {
			conf.createProcessInfo("not found dxgAdapter");
			return false;
		}

		auto fmt = DXGI_FORMAT_R8G8B8A8_UNORM;

		if (dxgAdapter || !conf.offscreen) {
			uint32_t maxResolutionArea = 0;
			float32_t maxRefreshRate = 0.f;
			for (UINT i = 0;; ++i) {
				IDXGIOutput* output = nullptr;
				if (dxgAdapter->EnumOutputs(i, &output) == DXGI_ERROR_NOT_FOUND) {
					conf.createProcessInfo("adapter enum outputs end");
					break;
				}
				objs.add(output);

				conf.createProcessInfo("adapter output check mode...");

				UINT numSupportedModes = 0;
				if (FAILED(output->GetDisplayModeList(fmt, 0, &numSupportedModes, nullptr))) {
					conf.createProcessInfo("adapter output GetDisplayModeList failed, skip");
					continue;
				}

				auto supportedModes = new DXGI_MODE_DESC[numSupportedModes];
				memset(supportedModes, 0, sizeof(DXGI_MODE_DESC) * numSupportedModes);
				if (FAILED(output->GetDisplayModeList(fmt, 0, &numSupportedModes, supportedModes))) {
					delete[] supportedModes;
					conf.createProcessInfo("adapter output GetDisplayModeList whit supportedModes failed, skip");
					continue;
				}

				for (decltype(numSupportedModes) i = 0; i < numSupportedModes; ++i) {
					auto& m = supportedModes[i];
					if (uint32_t area = m.Width * m.Height; maxResolutionArea < area) {
						maxResolutionArea = area;
						_refreshRate.Numerator = 0;
						_refreshRate.Denominator = 1;
						maxRefreshRate = (float32_t)m.RefreshRate.Numerator / (float32_t)m.RefreshRate.Denominator;
					} else if (maxResolutionArea == area) {
						if (decltype(maxRefreshRate) rr = (float32_t)m.RefreshRate.Numerator / (float32_t)m.RefreshRate.Denominator; rr > maxRefreshRate) {
							maxRefreshRate = rr;

							_refreshRate.Numerator = m.RefreshRate.Numerator;
							_refreshRate.Denominator = m.RefreshRate.Denominator;
						}
					}
				}

				delete[] supportedModes;
			}

			if (maxResolutionArea == 0 && !conf.offscreen) {
				conf.createProcessInfo("not found suitable mode");
				return false;
			}
		}

		auto driverType = D3D_DRIVER_TYPE_UNKNOWN;
		if (conf.driverType == "hardware") {
			driverType = D3D_DRIVER_TYPE_HARDWARE;
		} else if (conf.driverType == "software") {
			driverType = D3D_DRIVER_TYPE_WARP;
		}

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
		UINT totalFeatureLevels = ARRAYSIZE(featureLevels);

		UINT creationFlags = 0;
		if (conf.debug) creationFlags |= D3D11_CREATE_DEVICE_DEBUG;

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

		if (FAILED(D3D11CreateDevice(driverType == D3D_DRIVER_TYPE_UNKNOWN ? dxgAdapter : nullptr, driverType, nullptr, creationFlags,
			featureLevels, totalFeatureLevels,
			D3D11_SDK_VERSION, (ID3D11Device**)&_device, nullptr, (ID3D11DeviceContext**)&_context))) {
			conf.createProcessInfo("CreateDevice failed");
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

		_device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS, &_internalFeatures, sizeof(_internalFeatures));

		_deviceFeatures.sampler = true;
		_deviceFeatures.nativeTextureView = true;
		_deviceFeatures.nativeRenderView = true;
		_deviceFeatures.pixelBuffer = false;
		_deviceFeatures.constantBuffer = true;
		_deviceFeatures.textureMap = true;
		_deviceFeatures.persistentMap = false;
		_deviceFeatures.independentBlend = true;
		_deviceFeatures.stencilIndependentRef = false;
		_deviceFeatures.stencilIndependentMask = false;
		_deviceFeatures.simultaneousRenderTargetCount = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
		_deviceFeatures.indexTypes.emplace_back(IndexType::UI16);
		_deviceFeatures.indexTypes.emplace_back(IndexType::UI32);
		_deviceFeatures.textureFormats.emplace_back(TextureFormat::R8G8B8A8);

		for (UINT i = 1; i <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; i <<= 1) {
			UINT numQualityLevels = 0;
			_device->CheckMultisampleQualityLevels(fmt, i, &numQualityLevels);
			if (numQualityLevels) _deviceFeatures.maxSampleCount = i;
		}

		auto size = conf.offscreen ? Vec2ui32::ZERO : conf.app->getCurrentClientSize();

		if (!conf.offscreen) {
			_backBufferSampleCount = conf.sampleCount > _deviceFeatures.maxSampleCount ? _deviceFeatures.maxSampleCount : conf.sampleCount;
			DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
			swapChainDesc.BufferCount = 1;
			swapChainDesc.BufferDesc.Width = size[0];
			swapChainDesc.BufferDesc.Height = size[1];
			swapChainDesc.BufferDesc.Format = fmt;
			swapChainDesc.BufferDesc.RefreshRate.Numerator = _refreshRate.Numerator;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = _refreshRate.Denominator;
			swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.OutputWindow = (HWND)conf.app->getNative(ApplicationNative::WINDOW);
			swapChainDesc.Windowed = true;
			swapChainDesc.SampleDesc.Count = _backBufferSampleCount;
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		
			if (FAILED(dxgFctory->CreateSwapChain(_device, &swapChainDesc, (IDXGISwapChain**)&_swapChain))) {
				conf.createProcessInfo("CreateSwapChain failed");
				_release();
				return false;
			}
		}

		{
			UINT n = 1;
			D3D11_VIEWPORT dvp;
			_context->RSGetViewports(&n, &dvp);

			_d3dStatus.vp.pos.set(dvp.TopLeftX, dvp.TopLeftY);
			_d3dStatus.vp.size.set(dvp.Width, dvp.Height);
		}

		_defaultBlendState = new BlendState(*this, true);
		_defaultDepthStencilState = new DepthStencilState(*this, true);
		_defaultRasterizerState = new RasterizerState(*this, true);
		_backDepthStencil = new DepthStencil(*this, true);

		_loader = conf.loader;
		_app = conf.app;

		_resize(size);

		conf.createProcessInfo("create device succeeded");

		return true;
	}

	IntrusivePtr<events::IEventDispatcher<GraphicsEvent>> Graphics::getEventDispatcher() {
		return _eventDispatcher;
	}

	//const events::IEventDispatcher<GraphicsEvent>& Graphics::getEventDispatcher() const {
	//	return _eventDispatcher;
	//}

	const std::string& Graphics::getVersion() const {
		return _deviceVersion;
	}

	const GraphicsDeviceFeatures& Graphics::getDeviceFeatures() const {
		return _deviceFeatures;
	}

	IntrusivePtr<IBlendState> Graphics::createBlendState() {
		return new BlendState(*this, false);
	}

	IntrusivePtr<IConstantBuffer> Graphics::createConstantBuffer() {
		return new ConstantBuffer(*this);
	}

	IntrusivePtr<IDepthStencil> Graphics::createDepthStencil() {
		return new DepthStencil(*this, false);
	}

	IntrusivePtr<IDepthStencilState> Graphics::createDepthStencilState() {
		return new DepthStencilState(*this, false);
	}

	IntrusivePtr<IIndexBuffer> Graphics::createIndexBuffer() {
		return new IndexBuffer(*this);
	}

	IntrusivePtr<IProgram> Graphics::createProgram() {
		return new Program(*this);
	}

	IntrusivePtr<IRasterizerState> Graphics::createRasterizerState() {
		return new RasterizerState(*this, false);
	}

	IntrusivePtr<IRenderTarget> Graphics::createRenderTarget() {
		return new RenderTarget(*this);
	}

	IntrusivePtr<IRenderView> Graphics::createRenderView() {
		return new RenderView(*this);
	}

	IntrusivePtr<ISampler> Graphics::createSampler() {
		return new Sampler(*this);
	}

	IntrusivePtr<ITexture1DResource> Graphics::createTexture1DResource() {
		return new Texture1DResource(*this);
	}

	IntrusivePtr<ITexture2DResource> Graphics::createTexture2DResource() {
		return new Texture2DResource(*this);
	}

	IntrusivePtr<ITexture3DResource> Graphics::createTexture3DResource() {
		return new Texture3DResource(*this);
	}

	IntrusivePtr<ITextureView> Graphics::createTextureView() {
		return new TextureView(*this);
	}

	IntrusivePtr<IVertexBuffer> Graphics::createVertexBuffer() {
		return new VertexBuffer(*this);
	}

	IntrusivePtr<IPixelBuffer> Graphics::createPixelBuffer() {
		return nullptr;
	}

	const Vec2ui32& Graphics::getBackBufferSize() const {
		return _d3dStatus.backSize;
	}

	void Graphics::setBackBufferSize(const Vec2ui32& size) {
		_resize(size);
	}

	Box2i32ui32 Graphics::getViewport() const {
		return _d3dStatus.vp;
	}

	void Graphics::setViewport(const Box2i32ui32& vp) {
		if (_context && _d3dStatus.vp != vp) {
			_d3dStatus.vp = vp;

			D3D11_VIEWPORT dvp;
			dvp.Width = vp.size[0];
			dvp.Height = vp.size[1];
			dvp.MinDepth = 0.0f;
			dvp.MaxDepth = 1.0f;
			dvp.TopLeftX = vp.pos[0];
			dvp.TopLeftY = vp.pos[1];

			_context->RSSetViewports(1, &dvp);
		}
	}

	void Graphics::setBlendState(IBlendState* state, const Vec4f32& constantFactors, uint32_t sampleMask) {
		if (state && state->getGraphics() == this) {
			if (auto native = state->getNative(); native) {
				_setBlendState(*(BlendState*)native, constantFactors, sampleMask);
			} else {
				_setBlendState(*(BlendState*)_defaultBlendState->getNative(), constantFactors, sampleMask);
			}
		} else if (_defaultBlendState) {
			_setBlendState(*(BlendState*)_defaultBlendState->getNative(), constantFactors, sampleMask);
		}
	}

	void Graphics::_setBlendState(BlendState& state, const Vec4f32& constantFactors, uint32_t sampleMask) {
		state.update();
		auto& blend = _d3dStatus.blend;
		if (auto internalState = state.getInternalState(); internalState &&
			(blend.featureValue != state.getFeatureValue() || blend.constantFactors != constantFactors || blend.sampleMask != sampleMask)) {
			blend.featureValue = state.getFeatureValue();
			blend.constantFactors.set(constantFactors);
			blend.sampleMask = sampleMask;

			_context->OMSetBlendState(internalState, constantFactors.data, sampleMask);
		}
	}

	void Graphics::setDepthStencilState(IDepthStencilState* state, uint32_t stencilFrontRef, uint32_t stencilBackRef) {
		if (state && state->getGraphics() == this) {
			if (auto native = state->getNative(); native) {
				_setDepthStencilState(*(DepthStencilState*)native, stencilFrontRef);
			} else {
				_setDepthStencilState(*(DepthStencilState*)_defaultDepthStencilState->getNative(), stencilFrontRef);
			}
		} else if (_defaultBlendState) {
			_setDepthStencilState(*(DepthStencilState*)_defaultDepthStencilState->getNative(), stencilFrontRef);
		}
	}

	void Graphics::_setDepthStencilState(DepthStencilState& state, uint32_t stencilRef) {
		state.update();
		auto& depthStencil = _d3dStatus.depthStencil;
		if (auto internalState = state.getInternalState(); internalState &&
			(depthStencil.featureValue != state.getFeatureValue() || depthStencil.stencilRef != stencilRef)) {
			depthStencil.featureValue = state.getFeatureValue();
			depthStencil.stencilRef = stencilRef;

			_context->OMSetDepthStencilState(internalState, stencilRef);
		}
	}

	void Graphics::setRasterizerState(IRasterizerState* state) {
		if (state && state->getGraphics() == this) {
			if (auto native = state->getNative(); native) {
				_setRasterizerState(*(RasterizerState*)native);
			} else {
				_setRasterizerState(*(RasterizerState*)_defaultRasterizerState->getNative());
			}
		} else if (_defaultRasterizerState) {
			_setRasterizerState(*(RasterizerState*)_defaultRasterizerState->getNative());
		}
	}

	void Graphics::_setRasterizerState(RasterizerState& state) {
		state.update();
		auto& rasterizer = _d3dStatus.rasterizer;
		if (auto internalState = state.getInternalState(); internalState && rasterizer.featureValue != state.getFeatureValue()) {
			rasterizer.featureValue = state.getFeatureValue();

			_context->RSSetState(internalState);
		}
	}

	void Graphics::beginRender() {
	}

	void Graphics::draw(const IVertexBufferGetter* vertexBufferGetter, IProgram* program, const IShaderParameterGetter* shaderParamGetter,
		const IIndexBuffer* indexBuffer, uint32_t count, uint32_t offset) {
		if (vertexBufferGetter && indexBuffer && program && program->getGraphics() == this && indexBuffer->getGraphics() == this && count > 0) {
			auto ib = (IndexBuffer*)indexBuffer->getNative();
			if (!ib) return;
			auto internalIndexBuffer = ib->getInternalBuffer();
			auto fmt = ib->getInternalFormat();
			if (!internalIndexBuffer || fmt == DXGI_FORMAT_UNKNOWN) return;
			auto numIndexElements = ib->getNumElements();
			if (!numIndexElements || offset >= numIndexElements) return;
			
			auto p = (Program*)program->getNative();
			if (p) {
				if (p->use(vertexBufferGetter, shaderParamGetter)) {
					uint32_t last = numIndexElements - offset;
					if (count > numIndexElements) count = numIndexElements;
					if (count > last) count = last;
					_context->IASetIndexBuffer((ID3D11Buffer*)internalIndexBuffer, fmt, 0);
					_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					_context->DrawIndexed(count, offset, 0);

					p->useEnd();

					_constantBufferManager.resetUsedShareConstantBuffers();
				}
				
				clearShaderResources<ProgramStage::CS>();
				clearShaderResources<ProgramStage::DS>();
				clearShaderResources<ProgramStage::GS>();
				clearShaderResources<ProgramStage::HS>();
				clearShaderResources<ProgramStage::PS>();
				clearShaderResources<ProgramStage::VS>();

				clearConstantBuffers<ProgramStage::CS>();
				clearConstantBuffers<ProgramStage::DS>();
				clearConstantBuffers<ProgramStage::GS>();
				clearConstantBuffers<ProgramStage::HS>();
				clearConstantBuffers<ProgramStage::PS>();
				clearConstantBuffers<ProgramStage::VS>();

				clearSamplers<ProgramStage::CS>();
				clearSamplers<ProgramStage::DS>();
				clearSamplers<ProgramStage::GS>();
				clearSamplers<ProgramStage::HS>();
				clearSamplers<ProgramStage::PS>();
				clearSamplers<ProgramStage::VS>();
			}
		}
	}

	void Graphics::drawInstanced(const IVertexBufferGetter* vertexBufferGetter, IProgram* program, const IShaderParameterGetter* shaderParamGetter,
		const IIndexBuffer* indexBuffer, uint32_t instancedCount, uint32_t count, uint32_t offset) {
		if (vertexBufferGetter && indexBuffer && program && program->getGraphics() == this && indexBuffer->getGraphics() == this && count > 0) {
			auto ib = (IndexBuffer*)indexBuffer->getNative();
			if (!ib) return;
			auto internalIndexBuffer = ib->getInternalBuffer();
			auto fmt = ib->getInternalFormat();
			if (!internalIndexBuffer || fmt == DXGI_FORMAT_UNKNOWN) return;
			auto numIndexElements = ib->getNumElements();
			if (!numIndexElements || offset >= numIndexElements) return;

			auto p = (Program*)program->getNative();
			if (p) {
				if (p->use(vertexBufferGetter, shaderParamGetter)) {
					uint32_t last = numIndexElements - offset;
					if (count > numIndexElements) count = numIndexElements;
					if (count > last) count = last;
					_context->IASetIndexBuffer((ID3D11Buffer*)internalIndexBuffer, fmt, 0);
					_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					_context->DrawIndexedInstanced(count, instancedCount, offset, 0, 0);

					p->useEnd();

					_constantBufferManager.resetUsedShareConstantBuffers();
				}

				clearShaderResources<ProgramStage::CS>();
				clearShaderResources<ProgramStage::DS>();
				clearShaderResources<ProgramStage::GS>();
				clearShaderResources<ProgramStage::HS>();
				clearShaderResources<ProgramStage::PS>();
				clearShaderResources<ProgramStage::VS>();

				clearConstantBuffers<ProgramStage::CS>();
				clearConstantBuffers<ProgramStage::DS>();
				clearConstantBuffers<ProgramStage::GS>();
				clearConstantBuffers<ProgramStage::HS>();
				clearConstantBuffers<ProgramStage::PS>();
				clearConstantBuffers<ProgramStage::VS>();

				clearSamplers<ProgramStage::CS>();
				clearSamplers<ProgramStage::DS>();
				clearSamplers<ProgramStage::GS>();
				clearSamplers<ProgramStage::HS>();
				clearSamplers<ProgramStage::PS>();
				clearSamplers<ProgramStage::VS>();
			}
		}
	}

	void Graphics::endRender() {
	}

	void Graphics::flush() {
		if (_context) _context->Flush();
	}

	void Graphics::present() {
		if (_swapChain) _swapChain->Present(0, 0);
	}

	void Graphics::setRenderTarget(IRenderTarget* rt) {
		auto setToBack = true;

		if (rt && rt->getGraphics() == this) {
			if (auto rtt = (RenderTarget*)rt->getNative(); rtt) {
				setToBack = false;
				_curIsBackBuffer = false;

				_numRTVs = 0;
				for (uint8_t i = 0, n = rtt->getNumRenderViews(); i < n; ++i) {
					_RTVs[i] = nullptr;
					if (auto rv = rtt->getRenderView(i); rv) {
						if (auto native = (RenderView*)rv->getNative(); native) {
							if (auto v = native->getInternalView(); v) {
								_RTVs[i] = v;
								_numRTVs = i + 1;
							}
						}
					}
				}
				
				_DSV = nullptr;
				if (auto ds = rtt->getDepthStencil(); ds) {
					if (auto native = (DepthStencil*)ds->getNative(); native) {
						_DSV = native->getInternalView();
					}
				}

				if (_context) _context->OMSetRenderTargets(_numRTVs, (ID3D11RenderTargetView**)_RTVs.data(), _DSV);
			}
		}

		if (setToBack && !_curIsBackBuffer) {
			_curIsBackBuffer = true;
			if (_context) {
				if (_backBufferView) {
					_context->OMSetRenderTargets(1, (ID3D11RenderTargetView**)&_backBufferView, _backDepthStencil->getInternalView());
				} else {
					_context->OMSetRenderTargets(0, nullptr, nullptr);
				}
			}
		}
	}

	void Graphics::clear(ClearFlag flags, const Vec4f32& color, float32_t depth, size_t stencil) {
		using namespace aurora::enum_operators;

		if (_context) {
			if ((flags & ClearFlag::COLOR) != ClearFlag::NONE) {
				if (_curIsBackBuffer) {
					if (_backBufferView) _context->ClearRenderTargetView(_backBufferView, color.data);
				} else {
					for (decltype(_numRTVs) i = 0; i < _numRTVs; ++i) {
						auto view = _RTVs[i];
						if (view) _context->ClearRenderTargetView(view, color.data);
					}
				}
			}
			if ( (flags & ClearFlag::DEPTH_STENCIL) != ClearFlag::NONE) {
				UINT clearFlags = 0;
				if ((flags & ClearFlag::DEPTH) != ClearFlag::NONE) clearFlags |= D3D11_CLEAR_DEPTH;
				if ((flags & ClearFlag::STENCIL) != ClearFlag::NONE) clearFlags |= D3D11_CLEAR_STENCIL;

				if (_curIsBackBuffer) {
					if (_backDepthStencil && _backDepthStencil->getInternalView()) _context->ClearDepthStencilView(_backDepthStencil->getInternalView(), clearFlags, depth, stencil);
				} else {
					if (_DSV) _context->ClearDepthStencilView(_DSV, clearFlags, depth, stencil);
				}
			}
		}
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

	void Graphics::_release() {
		if (_backBufferView) {
			_backBufferView->Release();
			_backBufferView = nullptr;
		}
		_backDepthStencil.reset();
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

		_isDebug = false;

		_refreshRate.Numerator = 0;
		_refreshRate.Denominator = 1;

		_featureLevel = D3D_FEATURE_LEVEL_9_1;
		_shaderModel = "";

		memset(&_internalFeatures, 0, sizeof(_internalFeatures));
		_deviceFeatures.reset();

		_deviceVersion = "D3D Unknown";
	}

	void Graphics::_resize(const Vec2ui32& size) {
		if (!_swapChain || size == Vec2ui32::ZERO || _d3dStatus.backSize == size) return;

		_d3dStatus.backSize = size;

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		_swapChain->GetDesc(&swapChainDesc);

		auto& bufferDesc = swapChainDesc.BufferDesc;

		auto sizeChange = bufferDesc.Width != size[0] || bufferDesc.Height != size[1];

		if (sizeChange || !_backBufferView || !_backDepthStencil->getInternalView()) {
			bufferDesc.Width = size[0];
			bufferDesc.Height = size[1];

			if (_backBufferView) {
				_backBufferView->Release();
				_backBufferView = nullptr;
			}

			if (sizeChange) {
				if (FAILED(_swapChain->ResizeBuffers(swapChainDesc.BufferCount, size[0], size[1], bufferDesc.Format, swapChainDesc.Flags))) {
					this->error("D3D swap chain ResizeBuffers error");
				}
			}

			ID3D11Texture2D* backBufferTex = nullptr;
			if (FAILED(_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTex))) {
				this->error("D3D swap chain GetBuffer error");
			}

			if (backBufferTex) {
				if (FAILED(_device->CreateRenderTargetView1(backBufferTex, nullptr, (ID3D11RenderTargetView1**)&_backBufferView))) {
					this->error("D3D swap chain CreateRenderTargetView error");
				}
				backBufferTex->Release();
			}

			_backDepthStencil->create(size, DepthStencilFormat::D24S8, _backBufferSampleCount);

			if (_curIsBackBuffer) {
				_context->OMSetRenderTargets(1, (ID3D11RenderTargetView**)&_backBufferView, _backDepthStencil->getInternalView());
			}

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

	D3D11_COMPARISON_FUNC Graphics::convertComparisonFunc(ComparisonFunc func) {
		switch (func) {
		case ComparisonFunc::NEVER:
			return D3D11_COMPARISON_NEVER;
		case ComparisonFunc::LESS:
			return D3D11_COMPARISON_LESS;
		case ComparisonFunc::EQUAL:
			return D3D11_COMPARISON_EQUAL;
		case ComparisonFunc::LESS_EQUAL:
			return D3D11_COMPARISON_LESS_EQUAL;
		case ComparisonFunc::GREATER:
			return D3D11_COMPARISON_GREATER;
		case ComparisonFunc::NOT_EQUAL:
			return D3D11_COMPARISON_NOT_EQUAL;
		case ComparisonFunc::GREATER_EQUAL:
			return D3D11_COMPARISON_GREATER_EQUAL;
		case ComparisonFunc::ALWAYS:
			return D3D11_COMPARISON_ALWAYS;
		default:
			return D3D11_COMPARISON_NEVER;
		}
	}
}