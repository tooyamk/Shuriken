#pragma once

#include "Base.h"
#include "modules/graphics/ConstantBufferManager.h"

namespace aurora::modules::graphics::win_d3d11 {
	class BlendState;
	class DepthStencil;
	class DepthStencilState;
	class RasterizerState;

	class AE_MODULE_DLL Graphics : public IGraphicsModule {
	public:
		Graphics(Ref* loader, Application* app);
		virtual ~Graphics();

		virtual events::IEventDispatcher<GraphicsEvent>& AE_CALL getEventDispatcher() override;
		virtual const events::IEventDispatcher<GraphicsEvent>& AE_CALL getEventDispatcher() const override;

		virtual const std::string& AE_CALL getVersion() const override;
		virtual const GraphicsDeviceFeatures& AE_CALL getDeviceFeatures() const override;
		virtual IBlendState* AE_CALL createBlendState() override;
		virtual IConstantBuffer* AE_CALL createConstantBuffer() override;
		virtual IDepthStencil* AE_CALL createDepthStencil() override;
		virtual IDepthStencilState* AE_CALL createDepthStencilState() override;
		virtual IIndexBuffer* AE_CALL createIndexBuffer() override;
		virtual IProgram* AE_CALL createProgram() override;
		virtual IRasterizerState* AE_CALL createRasterizerState() override;
		virtual IRenderTarget* AE_CALL createRenderTarget() override;
		virtual IRenderView* AE_CALL createRenderView() override;
		virtual ISampler* AE_CALL createSampler() override;
		virtual ITexture1DResource* AE_CALL createTexture1DResource() override;
		virtual ITexture2DResource* AE_CALL createTexture2DResource() override;
		virtual ITexture3DResource* AE_CALL createTexture3DResource() override;
		virtual ITextureView* AE_CALL createTextureView() override;
		virtual IVertexBuffer* AE_CALL createVertexBuffer() override;
		virtual IPixelBuffer* AE_CALL createPixelBuffer() override;

		virtual void AE_CALL setBlendState(IBlendState* state, const Vec4f32& constantFactors, uint32_t sampleMask = (std::numeric_limits<uint32_t>::max)()) override;
		virtual void AE_CALL setDepthStencilState(IDepthStencilState* state, uint32_t stencilFrontRef, uint32_t stencilBackRef) override;
		virtual void AE_CALL setRasterizerState(IRasterizerState* state) override;
		
		virtual void AE_CALL beginRender() override;
		virtual void AE_CALL draw(const VertexBufferFactory* vertexFactory, IProgram* program, const ShaderParameterFactory* paramFactory, 
			const IIndexBuffer* indexBuffer, uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) override;
		virtual void AE_CALL endRender() override;
		virtual void AE_CALL present() override;

		virtual void AE_CALL setRenderTarget(IRenderTarget* rt) override;
		virtual void AE_CALL clear(ClearFlag flags, const Vec4f32& color, f32 depth, size_t stencil) override;

		bool AE_CALL createDevice(const GraphicsAdapter* adapter);

		inline void AE_CALL error(const std::string_view& msg) {
			_eventDispatcher.dispatchEvent(this, GraphicsEvent::ERR, (std::string_view*) & msg);
		}

		inline ID3D11Device5* AE_CALL getDevice() const {
			return _device;
		}

		inline ID3D11DeviceContext4* AE_CALL getContext() const {
			return _context;
		}

		inline D3D_FEATURE_LEVEL AE_CALL getFeatureLevel() const {
			return _featureLevel;
		}

		inline const std::string& AE_CALL getSupportShaderModel() const {
			return _shaderModel;
		}

		inline const D3D11_FEATURE_DATA_D3D11_OPTIONS& AE_CALL getInternalFeatures() const {
			return _internalFeatures;
		}

		inline ConstantBufferManager& AE_CALL getConstantBufferManager() {
			return _constantBufferManager;
		}

		inline void AE_CALL useVertexBuffers(UINT slot, UINT numBuffers, ID3D11Buffer*const* buffers, const UINT* strides, const UINT* offsets) {
			_context->IASetVertexBuffers(slot, numBuffers, buffers, strides, offsets);
		}

		template<ProgramStage Stage>
		inline void AE_CALL useShader(ID3D11DeviceChild* shader, ID3D11ClassInstance*const* classInstances, UINT numClassInstances) {
			if constexpr (Stage == ProgramStage::CS) {
				_context->CSSetShader((ID3D11ComputeShader*)shader, classInstances, numClassInstances);
			} else if constexpr (Stage == ProgramStage::DS) {
				_context->DSSetShader((ID3D11DomainShader*)shader, classInstances, numClassInstances);
			} else if constexpr (Stage == ProgramStage::GS) {
				_context->GSSetShader((ID3D11GeometryShader*)shader, classInstances, numClassInstances);
			} else if constexpr (Stage == ProgramStage::HS) {
				_context->HSSetShader((ID3D11HullShader*)shader, classInstances, numClassInstances);
			} else if constexpr (Stage == ProgramStage::PS) {
				_context->PSSetShader((ID3D11PixelShader*)shader, classInstances, numClassInstances);
			} else if constexpr (Stage == ProgramStage::VS) {
				_context->VSSetShader((ID3D11VertexShader*)shader, classInstances, numClassInstances);
			}
		}

		template<ProgramStage Stage>
		inline void AE_CALL useShaderResources(UINT slot, UINT numViews, ID3D11ShaderResourceView*const* views) {
			if constexpr (Stage == ProgramStage::CS) {
				_context->CSSetShaderResources(slot, numViews, views);
				_programUsingSlotsCS.shaderResources.emplace_back(slot, numViews);
			} else if constexpr (Stage == ProgramStage::DS) {
				_context->DSSetShaderResources(slot, numViews, views);
				_programUsingSlotsDS.shaderResources.emplace_back(slot, numViews);
			} else if constexpr (Stage == ProgramStage::GS) {
				_context->GSSetShaderResources(slot, numViews, views);
				_programUsingSlotsGS.shaderResources.emplace_back(slot, numViews);
			} else if constexpr (Stage == ProgramStage::HS) {
				_context->HSSetShaderResources(slot, numViews, views);
				_programUsingSlotsHS.shaderResources.emplace_back(slot, numViews);
			} else if constexpr (Stage == ProgramStage::PS) {
				_context->PSSetShaderResources(slot, numViews, views);
				_programUsingSlotsPS.shaderResources.emplace_back(slot, numViews);
			} else if constexpr (Stage == ProgramStage::VS) {
				_context->VSSetShaderResources(slot, numViews, views);
				_programUsingSlotsVS.shaderResources.emplace_back(slot, numViews);
			}
		}

		template<ProgramStage Stage>
		inline void AE_CALL clearShaderResources() {
			void(ID3D11DeviceContext::*fn)(UINT, UINT, ID3D11ShaderResourceView*const*);
			ProgramUsingSlots* slots;
			if constexpr (Stage == ProgramStage::CS) {
				fn = &ID3D11DeviceContext::CSSetShaderResources;
				slots = &_programUsingSlotsCS;
			} else if constexpr (Stage == ProgramStage::DS) {
				fn = &ID3D11DeviceContext::DSSetShaderResources;
				slots = &_programUsingSlotsDS;
			} else if constexpr (Stage == ProgramStage::GS) {
				fn = &ID3D11DeviceContext::GSSetShaderResources;
				slots = &_programUsingSlotsGS;
			} else if constexpr (Stage == ProgramStage::HS) {
				fn = &ID3D11DeviceContext::HSSetShaderResources;
				slots = &_programUsingSlotsHS;
			} else if constexpr (Stage == ProgramStage::PS) {
				fn = &ID3D11DeviceContext::PSSetShaderResources;
				slots = &_programUsingSlotsPS;
			} else if constexpr (Stage == ProgramStage::VS) {
				fn = &ID3D11DeviceContext::VSSetShaderResources;
				slots = &_programUsingSlotsVS;
			} else {
				return;
			}

			for (const auto& slot : slots->shaderResources) {
				_checkProgramClearData(slot.num);
				(_context->*fn)(slot.start, slot.num, (ID3D11ShaderResourceView*const*)_programClearData.data());
			}
		}

		template<ProgramStage Stage>
		inline void AE_CALL useConstantBuffers(UINT slot, UINT numBuffers, ID3D11Buffer*const* buffers) {
			if constexpr (Stage == ProgramStage::CS) {
				_context->CSSetConstantBuffers(slot, numBuffers, buffers);
				_programUsingSlotsCS.constantBuffers.emplace_back(slot, numBuffers);
			} else if constexpr (Stage == ProgramStage::DS) {
				_context->DSSetConstantBuffers(slot, numBuffers, buffers);
				_programUsingSlotsDS.constantBuffers.emplace_back(slot, numBuffers);
			} else if constexpr (Stage == ProgramStage::GS) {
				_context->GSSetConstantBuffers(slot, numBuffers, buffers);
				_programUsingSlotsGS.constantBuffers.emplace_back(slot, numBuffers);
			} else if constexpr (Stage == ProgramStage::HS) {
				_context->HSSetConstantBuffers(slot, numBuffers, buffers);
				_programUsingSlotsHS.constantBuffers.emplace_back(slot, numBuffers);
			} else if constexpr (Stage == ProgramStage::PS) {
				_context->PSSetConstantBuffers(slot, numBuffers, buffers);
				_programUsingSlotsPS.constantBuffers.emplace_back(slot, numBuffers);
			} else if constexpr (Stage == ProgramStage::VS) {
				_context->VSSetConstantBuffers(slot, numBuffers, buffers);
				_programUsingSlotsVS.constantBuffers.emplace_back(slot, numBuffers);
			}
		}

		template<ProgramStage Stage>
		inline void AE_CALL clearConstantBuffers() {
			void(ID3D11DeviceContext::*fn)(UINT, UINT, ID3D11Buffer*const*);
			ProgramUsingSlots* slots;
			if constexpr (Stage == ProgramStage::CS) {
				fn = &ID3D11DeviceContext::CSSetConstantBuffers;
				slots = &_programUsingSlotsCS;
			} else if constexpr (Stage == ProgramStage::DS) {
				fn = &ID3D11DeviceContext::DSSetConstantBuffers;
				slots = &_programUsingSlotsDS;
			} else if constexpr (Stage == ProgramStage::GS) {
				fn = &ID3D11DeviceContext::GSSetConstantBuffers;
				slots = &_programUsingSlotsGS;
			} else if constexpr (Stage == ProgramStage::HS) {
				fn = &ID3D11DeviceContext::HSSetConstantBuffers;
				slots = &_programUsingSlotsHS;
			} else if constexpr (Stage == ProgramStage::PS) {
				fn = &ID3D11DeviceContext::PSSetConstantBuffers;
				slots = &_programUsingSlotsPS;
			} else if constexpr (Stage == ProgramStage::VS) {
				fn = &ID3D11DeviceContext::VSSetConstantBuffers;
				slots = &_programUsingSlotsVS;
			} else {
				return;
			}

			for (const auto& slot : slots->constantBuffers) {
				_checkProgramClearData(slot.num);
				(_context->*fn)(slot.start, slot.num, (ID3D11Buffer*const*)_programClearData.data());
			}
		}

		template<ProgramStage Stage>
		inline void AE_CALL useSamplers(UINT slot, UINT numSamplers, ID3D11SamplerState*const* samplers) {
			if constexpr (Stage == ProgramStage::CS) {
				_context->CSSetSamplers(slot, numSamplers, samplers);
				_programUsingSlotsCS.samplers.emplace_back(slot, numSamplers);
			} else if constexpr (Stage == ProgramStage::DS) {
				_context->DSSetSamplers(slot, numSamplers, samplers);
				_programUsingSlotsDS.samplers.emplace_back(slot, numSamplers);
			} else if constexpr (Stage == ProgramStage::GS) {
				_context->GSSetSamplers(slot, numSamplers, samplers);
				_programUsingSlotsGS.samplers.emplace_back(slot, numSamplers);
			} else if constexpr (Stage == ProgramStage::HS) {
				_context->HSSetSamplers(slot, numSamplers, samplers);
				_programUsingSlotsHS.samplers.emplace_back(slot, numSamplers);
			} else if constexpr (Stage == ProgramStage::PS) {
				_context->PSSetSamplers(slot, numSamplers, samplers);
				_programUsingSlotsPS.samplers.emplace_back(slot, numSamplers);
			} else if constexpr (Stage == ProgramStage::VS) {
				_context->VSSetSamplers(slot, numSamplers, samplers);
				_programUsingSlotsVS.samplers.emplace_back(slot, numSamplers);
			}
		}

		template<ProgramStage Stage>
		inline void AE_CALL clearSamplers() {
			void(ID3D11DeviceContext::*fn)(UINT, UINT, ID3D11SamplerState*const*);
			ProgramUsingSlots* slots;
			if constexpr (Stage == ProgramStage::CS) {
				fn = &ID3D11DeviceContext::CSSetSamplers;
				slots = &_programUsingSlotsCS;
			} else if constexpr (Stage == ProgramStage::DS) {
				fn = &ID3D11DeviceContext::DSSetSamplers;
				slots = &_programUsingSlotsDS;
			} else if constexpr (Stage == ProgramStage::GS) {
				fn = &ID3D11DeviceContext::GSSetSamplers;
				slots = &_programUsingSlotsGS;
			} else if constexpr (Stage == ProgramStage::HS) {
				fn = &ID3D11DeviceContext::HSSetSamplers;
				slots = &_programUsingSlotsHS;
			} else if constexpr (Stage == ProgramStage::PS) {
				fn = &ID3D11DeviceContext::PSSetSamplers;
				slots = &_programUsingSlotsPS;
			} else if constexpr (Stage == ProgramStage::VS) {
				fn = &ID3D11DeviceContext::VSSetSamplers;
				slots = &_programUsingSlotsVS;
			} else {
				return;
			}

			for (const auto& slot : slots->samplers) {
				_checkProgramClearData(slot.num);
				(_context->*fn)(slot.start, slot.num, (ID3D11SamplerState*const*)_programClearData.data());
			}
		}

		static DXGI_FORMAT AE_CALL convertInternalFormat(TextureFormat fmt);
		static D3D11_COMPARISON_FUNC AE_CALL convertComparisonFunc(ComparisonFunc func);

	private:
		bool _curIsBackBuffer;
		RefPtr<Ref> _loader;
		RefPtr<Application> _app;

		DXGI_RATIONAL _refreshRate;
		D3D_FEATURE_LEVEL _featureLevel;
		std::string _shaderModel;
		//D3D_DRIVER_TYPE _driverType;
		ID3D11Device5* _device;
		ID3D11DeviceContext4* _context;
		IDXGISwapChain4* _swapChain;
		ID3D11RenderTargetView1* _backBufferView;
		RefPtr<DepthStencil> _backDepthStencil;
		D3D11_FEATURE_DATA_D3D11_OPTIONS _internalFeatures;

		uint8_t _numRTVs;
		std::array<ID3D11RenderTargetView1*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> _RTVs;
		ID3D11DepthStencilView* _DSV;

		struct ProgramUsingSlots {
			struct Slot {
				Slot(UINT start, UINT num) :
					start(start),
					num(num) {
				}
				UINT start;
				UINT num;
			};
			std::vector<Slot> shaderResources;
			std::vector<Slot> constantBuffers;
			std::vector<Slot> samplers;

			inline void AE_CALL reset() {
				shaderResources.clear();
				constantBuffers.clear();
				samplers.clear();
			}
		};

		ProgramUsingSlots _programUsingSlotsCS;
		ProgramUsingSlots _programUsingSlotsDS;
		ProgramUsingSlots _programUsingSlotsGS;
		ProgramUsingSlots _programUsingSlotsHS;
		ProgramUsingSlots _programUsingSlotsPS;
		ProgramUsingSlots _programUsingSlotsVS;
		std::vector<void*> _programClearData;

		RefPtr<BlendState> _defaultBlendState;
		RefPtr<DepthStencilState> _defaultDepthStencilState;
		RefPtr<RasterizerState> _defaultRasterizerState;

		GraphicsDeviceFeatures _deviceFeatures;
		std::string _deviceVersion;

		struct {
			struct {
				uint64_t featureValue;
			} rasterizer;

			struct {
				uint64_t featureValue;
				Vec4f32 constantFactors;
				uint32_t sampleMask;
			} blend;

			struct {
				uint64_t featureValue;
				uint32_t stencilRef;
			} depthStencil;
		} _d3dStatus;

		ConstantBufferManager _constantBufferManager;

		events::EventDispatcher<GraphicsEvent> _eventDispatcher;

		events::EventListener<ApplicationEvent, Graphics> _resizedListener;
		void AE_CALL _resizedHandler(events::Event<ApplicationEvent>& e);

		bool AE_CALL _createDevice(const GraphicsAdapter& adapter);

		void AE_CALL _setBlendState(BlendState& state, const Vec4f32& constantFactors, uint32_t sampleMask);
		void AE_CALL _setDepthStencilState(DepthStencilState& state, uint32_t stencilRef);
		void AE_CALL _setRasterizerState(RasterizerState& state);

		inline void AE_CALL _checkProgramClearData(UINT num) {
			if (_programClearData.size() < num) {
				auto oldSize = _programClearData.size();
				_programClearData.resize(num);
				memset(_programClearData.data() + oldSize, 0, sizeof(void*) * (num - oldSize));
			}
		}

		void AE_CALL _release();
		void AE_CALL _resize(const Vec2<UINT>& size);

		IConstantBuffer* AE_CALL _createdShareConstantBuffer();
		IConstantBuffer* AE_CALL _createdExclusiveConstantBuffer(uint32_t numParameters);
	};
}