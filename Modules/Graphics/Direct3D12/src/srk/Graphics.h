#pragma once

#include "Base.h"
#include "srk/events/EventDispatcher.h"
#include "srk/modules/graphics/ConstantBufferManager.h"
#include "srk/modules/windows/WindowModule.h"

namespace srk::modules::graphics::d3d12 {
	/*class BlendState;
	class DepthStencil;
	class DepthStencilState;
	class RasterizerState;*/

	class SRK_MODULE_DLL Graphics : public IGraphicsModule {
	public:
		Graphics();
		virtual ~Graphics();

		void operator delete(Graphics* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Graphics();
			::operator delete(p);
		}

		virtual IntrusivePtr<events::IEventDispatcher<GraphicsEvent>> SRK_CALL getEventDispatcher() override;
		//virtual const events::IEventDispatcher<GraphicsEvent>& SRK_CALL getEventDispatcher() const override;

		virtual const std::string& SRK_CALL getVersion() const override;
		virtual const GraphicsDeviceFeatures& SRK_CALL getDeviceFeatures() const override;
		virtual IntrusivePtr<IBlendState> SRK_CALL createBlendState() override;
		virtual IntrusivePtr<IConstantBuffer> SRK_CALL createConstantBuffer() override;
		virtual IntrusivePtr<IDepthStencil> SRK_CALL createDepthStencil() override;
		virtual IntrusivePtr<IDepthStencilState> SRK_CALL createDepthStencilState() override;
		virtual IntrusivePtr<IIndexBuffer> SRK_CALL createIndexBuffer() override;
		virtual IntrusivePtr<IProgram> SRK_CALL createProgram() override;
		virtual IntrusivePtr<IRasterizerState> SRK_CALL createRasterizerState() override;
		virtual IntrusivePtr<IRenderTarget> SRK_CALL createRenderTarget() override;
		virtual IntrusivePtr<IRenderView> SRK_CALL createRenderView() override;
		virtual IntrusivePtr<ISampler> SRK_CALL createSampler() override;
		virtual IntrusivePtr<ITexture1DResource> SRK_CALL createTexture1DResource() override;
		virtual IntrusivePtr<ITexture2DResource> SRK_CALL createTexture2DResource() override;
		virtual IntrusivePtr<ITexture3DResource> SRK_CALL createTexture3DResource() override;
		virtual IntrusivePtr<ITextureView> SRK_CALL createTextureView() override;
		virtual IntrusivePtr<IVertexBuffer> SRK_CALL createVertexBuffer() override;

		virtual const Vec2ui32& SRK_CALL getBackBufferSize() const override;
		virtual void SRK_CALL setBackBufferSize(const Vec2ui32& size) override;
		virtual Box2i32ui32 SRK_CALL getViewport() const override;
		virtual void SRK_CALL setViewport(const Box2i32ui32& vp) override;
		virtual Box2i32ui32 SRK_CALL getScissor() const override;
		virtual void SRK_CALL setScissor(const Box2i32ui32& scissor) override;
		virtual void SRK_CALL setBlendState(IBlendState* state, uint32_t sampleMask = (std::numeric_limits<uint32_t>::max)()) override;
		virtual void SRK_CALL setDepthStencilState(IDepthStencilState* state) override;
		virtual void SRK_CALL setRasterizerState(IRasterizerState* state) override;
		
		virtual void SRK_CALL beginRender() override;
		virtual void SRK_CALL draw(IProgram* program, const IVertexAttributeGetter* vertexAttributeGetter, const IShaderParameterGetter* shaderParamGetter,
			const IIndexBuffer* indexBuffer, uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) override;
		virtual void SRK_CALL drawInstanced(IProgram* program, const IVertexAttributeGetter* vertexAttributeGetter, const IShaderParameterGetter* shaderParamGetter,
			const IIndexBuffer* indexBuffer, uint32_t instancedCount, uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) override;
		virtual void SRK_CALL endRender() override;
		virtual void SRK_CALL flush() override;
		virtual void SRK_CALL present() override;

		virtual void SRK_CALL setRenderTarget(IRenderTarget* rt) override;
		virtual void SRK_CALL clear(ClearFlag flags, const Vec4f32& color, float32_t depth, size_t stencil) override;

		bool SRK_CALL createDevice(Ref* loader, const CreateGrahpicsModuleDesc& desc);

		inline void SRK_CALL error(const std::string_view& msg) {
			_eventDispatcher->dispatchEvent(this, GraphicsEvent::ERR, (std::string_view*)&msg);
		}

		inline bool SRK_CALL isDebug() const {
			return _isDebug;
		}

		inline ID3D12Device* SRK_CALL getDevice() const {
			return _device;
		}

		/*inline ID3D11DeviceContext4* SRK_CALL getContext() const {
			return _context;
		}*/

		inline D3D_FEATURE_LEVEL SRK_CALL getFeatureLevel() const {
			return _featureLevel;
		}

		inline const std::string& SRK_CALL getSupportShaderModel() const {
			return _shaderModel;
		}

		/*inline const D3D11_FEATURE_DATA_D3D11_OPTIONS& SRK_CALL getInternalFeatures() const {
			return _internalFeatures;
		}*/

		inline ConstantBufferManager& SRK_CALL getConstantBufferManager() {
			return _constantBufferManager;
		}

		/*inline void SRK_CALL useVertexBuffers(UINT slot, UINT numBuffers, ID3D11Buffer*const* buffers, const UINT* strides, const UINT* offsets) {
			_context->IASetVertexBuffers(slot, numBuffers, buffers, strides, offsets);
		}

		template<ProgramStage Stage>
		inline void SRK_CALL useShader(ID3D11DeviceChild* shader, ID3D11ClassInstance*const* classInstances, UINT numClassInstances) {
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
		inline void SRK_CALL useShaderResources(UINT slot, UINT numViews, ID3D11ShaderResourceView*const* views) {
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
		inline void SRK_CALL clearShaderResources() {
			if constexpr (Stage == ProgramStage::CS) {
				_clearD3D11DeviceChildren<ID3D11ShaderResourceView>(_programUsingSlotsCS.shaderResources, &ID3D11DeviceContext::CSSetShaderResources);
			} else if constexpr (Stage == ProgramStage::DS) {
				_clearD3D11DeviceChildren<ID3D11ShaderResourceView>(_programUsingSlotsDS.shaderResources, &ID3D11DeviceContext::DSSetShaderResources);
			} else if constexpr (Stage == ProgramStage::GS) {
				_clearD3D11DeviceChildren<ID3D11ShaderResourceView>(_programUsingSlotsGS.shaderResources, &ID3D11DeviceContext::GSSetShaderResources);
			} else if constexpr (Stage == ProgramStage::HS) {
				_clearD3D11DeviceChildren<ID3D11ShaderResourceView>(_programUsingSlotsHS.shaderResources, &ID3D11DeviceContext::HSSetShaderResources);
			} else if constexpr (Stage == ProgramStage::PS) {
				_clearD3D11DeviceChildren<ID3D11ShaderResourceView>(_programUsingSlotsPS.shaderResources, &ID3D11DeviceContext::PSSetShaderResources);
			} else if constexpr (Stage == ProgramStage::VS) {
				_clearD3D11DeviceChildren<ID3D11ShaderResourceView>(_programUsingSlotsVS.shaderResources, &ID3D11DeviceContext::VSSetShaderResources);
			}
		}

		template<ProgramStage Stage>
		inline void SRK_CALL useConstantBuffers(UINT slot, UINT numBuffers, ID3D11Buffer*const* buffers) {
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
		inline void SRK_CALL clearConstantBuffers() {
			if constexpr (Stage == ProgramStage::CS) {
				_clearD3D11DeviceChildren<ID3D11Buffer>(_programUsingSlotsCS.constantBuffers, &ID3D11DeviceContext::CSSetConstantBuffers);
			} else if constexpr (Stage == ProgramStage::DS) {
				_clearD3D11DeviceChildren<ID3D11Buffer>(_programUsingSlotsDS.constantBuffers, &ID3D11DeviceContext::DSSetConstantBuffers);
			} else if constexpr (Stage == ProgramStage::GS) {
				_clearD3D11DeviceChildren<ID3D11Buffer>(_programUsingSlotsGS.constantBuffers, &ID3D11DeviceContext::GSSetConstantBuffers);
			} else if constexpr (Stage == ProgramStage::HS) {
				_clearD3D11DeviceChildren<ID3D11Buffer>(_programUsingSlotsHS.constantBuffers, &ID3D11DeviceContext::HSSetConstantBuffers);
			} else if constexpr (Stage == ProgramStage::PS) {
				_clearD3D11DeviceChildren<ID3D11Buffer>(_programUsingSlotsPS.constantBuffers, &ID3D11DeviceContext::PSSetConstantBuffers);
			} else if constexpr (Stage == ProgramStage::VS) {
				_clearD3D11DeviceChildren<ID3D11Buffer>(_programUsingSlotsVS.constantBuffers, &ID3D11DeviceContext::VSSetConstantBuffers);
			}
		}

		template<ProgramStage Stage>
		inline void SRK_CALL useSamplers(UINT slot, UINT numSamplers, ID3D11SamplerState*const* samplers) {
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
		inline void SRK_CALL clearSamplers() {
			if constexpr (Stage == ProgramStage::CS) {
				_clearD3D11DeviceChildren<ID3D11SamplerState>(_programUsingSlotsCS.samplers, &ID3D11DeviceContext::CSSetSamplers);
			} else if constexpr (Stage == ProgramStage::DS) {
				_clearD3D11DeviceChildren<ID3D11SamplerState>(_programUsingSlotsDS.samplers, &ID3D11DeviceContext::DSSetSamplers);
			} else if constexpr (Stage == ProgramStage::GS) {
				_clearD3D11DeviceChildren<ID3D11SamplerState>(_programUsingSlotsGS.samplers, &ID3D11DeviceContext::GSSetSamplers);
			} else if constexpr (Stage == ProgramStage::HS) {
				_clearD3D11DeviceChildren<ID3D11SamplerState>(_programUsingSlotsHS.samplers, &ID3D11DeviceContext::HSSetSamplers);
			} else if constexpr (Stage == ProgramStage::PS) {
				_clearD3D11DeviceChildren<ID3D11SamplerState>(_programUsingSlotsPS.samplers, &ID3D11DeviceContext::PSSetSamplers);
			} else if constexpr (Stage == ProgramStage::VS) {
				_clearD3D11DeviceChildren<ID3D11SamplerState>(_programUsingSlotsVS.samplers, &ID3D11DeviceContext::VSSetSamplers);
			}
		}*/

		static DXGI_FORMAT SRK_CALL convertInternalFormat(TextureFormat fmt);
		//static D3D11_COMPARISON_FUNC SRK_CALL convertComparisonFunc(ComparisonFunc func);

	private:
		bool _isDebug;
		bool _curIsBackBuffer;
		SampleCount _backBufferSampleCount;
		IntrusivePtr<Ref> _loader;
		IntrusivePtr<windows::IWindow> _win;

		DXGI_RATIONAL _refreshRate;
		D3D_FEATURE_LEVEL _featureLevel;
		std::string _shaderModel;
		//D3D_DRIVER_TYPE _driverType;
		ID3D12Device* _device;
		ID3D12CommandQueue* _commandQueue;
		IDXGISwapChain4* _swapChain;
		//ID3D11RenderTargetView1* _backBufferView;
		//IntrusivePtr<DepthStencil> _backDepthStencil;
		//D3D11_FEATURE_DATA_D3D11_OPTIONS _internalFeatures;

		uint8_t _numRTVs;
		//std::array<ID3D11RenderTargetView1*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> _RTVs;
		//ID3D11DepthStencilView* _DSV;

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
		};

		ProgramUsingSlots _programUsingSlotsCS;
		ProgramUsingSlots _programUsingSlotsDS;
		ProgramUsingSlots _programUsingSlotsGS;
		ProgramUsingSlots _programUsingSlotsHS;
		ProgramUsingSlots _programUsingSlotsPS;
		ProgramUsingSlots _programUsingSlotsVS;
		std::vector<void*> _programClearData;

		/*IntrusivePtr<BlendState> _defaultBlendState;
		IntrusivePtr<DepthStencilState> _defaultDepthStencilState;
		IntrusivePtr<RasterizerState> _defaultRasterizerState;*/

		GraphicsDeviceFeatures _deviceFeatures;
		std::string _deviceVersion;

		struct {
			struct {
				uint32_t featureValue;
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

			Vec2<UINT> backSize;
			Box2i32ui32 viewport;
			Box2i32ui32 scissor;
		} _d3dStatus;


		ConstantBufferManager _constantBufferManager;

		IntrusivePtr<events::IEventDispatcher<GraphicsEvent>> _eventDispatcher;

		bool SRK_CALL _createDevice(Ref* loader, const CreateGrahpicsModuleDesc& desc, const GraphicsAdapter* adapter);

		/*void SRK_CALL _setBlendState(BlendState& state, const Vec4f32& constantFactors, uint32_t sampleMask);
		void SRK_CALL _setDepthStencilState(DepthStencilState& state, uint32_t stencilRef);
		void SRK_CALL _setRasterizerState(RasterizerState& state);*/

		inline void SRK_CALL _checkProgramClearData(UINT num) {
			if (_programClearData.size() < num) {
				auto oldSize = _programClearData.size();
				_programClearData.resize(num);
				memset(_programClearData.data() + oldSize, 0, sizeof(void*) * (num - oldSize));
			}
		}

		void SRK_CALL _release();
		void SRK_CALL _resize(const Vec2ui32& size);

		/*template<typename T>
		void SRK_CALL _clearD3D11DeviceChildren(std::vector<ProgramUsingSlots::Slot>& slots, void(ID3D11DeviceContext::*fn)(UINT, UINT, T*const*)) {
			if (auto size = slots.size(); size) {
				ProgramUsingSlots::Slot* s = &slots[0];
				for (size_t i = 1; i < size; ++i) {
					ProgramUsingSlots::Slot* s1 = &slots[i];
					if (s->start + s->num >= s1->start) {
						s->num = s1->start + s1->num - s->start;
					} else {
						_clearD3D11DeviceChildren<T>(*s, fn);
						s = s1;
					}
				}
				_clearD3D11DeviceChildren<T>(*s, fn);

				slots.clear();
			}
		}

		template<typename T>
		inline void SRK_CALL _clearD3D11DeviceChildren(const ProgramUsingSlots::Slot& slot, void(ID3D11DeviceContext::*fn)(UINT, UINT, T*const*)) {
			_checkProgramClearData(slot.num);
			(_context->*fn)(slot.start, slot.num, (T*const*)_programClearData.data());
		}*/

		IConstantBuffer* SRK_CALL _createdShareConstantBuffer();
		IConstantBuffer* SRK_CALL _createdExclusiveConstantBuffer(uint32_t numParameters);
	};
}