#pragma once

#include "Base.h"
#include "modules/graphics/ConstantBufferManager.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL Graphics : public IGraphicsModule {
	public:
		Graphics(Ref* loader, Application* app);
		virtual ~Graphics();

		virtual const std::string& AE_CALL getVersion() const override;
		virtual const GraphicsDeviceFeatures& AE_CALL getDeviceFeatures() const override;
		virtual IConstantBuffer* AE_CALL createConstantBuffer() override;
		virtual IIndexBuffer* AE_CALL createIndexBuffer() override;
		virtual IProgram* AE_CALL createProgram() override;
		virtual ISampler* AE_CALL createSampler() override;
		virtual ITexture1DResource* AE_CALL createTexture1DResource() override;
		virtual ITexture2DResource* AE_CALL createTexture2DResource() override;
		virtual ITexture3DResource* AE_CALL createTexture3DResource() override;
		virtual ITextureView* AE_CALL createTextureView() override;
		virtual IVertexBuffer* AE_CALL createVertexBuffer() override;
		
		virtual void AE_CALL beginRender() override;
		virtual void AE_CALL endRender() override;
		virtual void AE_CALL present() override;

		virtual void AE_CALL clear() override;

		bool AE_CALL createDevice(const GraphicsAdapter* adapter);

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

		template<ProgramStage stage>
		inline void AE_CALL useShader(ID3D11DeviceChild* shader, ID3D11ClassInstance *const* classInstances, UINT numClassInstances) {
			if constexpr (stage == ProgramStage::VS) {
				_context->VSSetShader((ID3D11VertexShader*)shader, classInstances, numClassInstances);
			} else if constexpr (stage == ProgramStage::PS) {
				_context->PSSetShader((ID3D11PixelShader*)shader, classInstances, numClassInstances);
			}
		}

		template<ProgramStage stage>
		inline void AE_CALL useShaderResources(UINT slot, UINT numViews, ID3D11ShaderResourceView*const* views) {
			if constexpr (stage == ProgramStage::VS) {
				_context->VSSetShaderResources(slot, numViews, views);
			} else if constexpr (stage == ProgramStage::PS) {
				_context->PSSetShaderResources(slot, numViews, views);
			}
		}

		template<ProgramStage stage>
		inline void AE_CALL useConstantBuffers(UINT slot, UINT numBuffers, ID3D11Buffer*const* buffers) {
			if constexpr (stage == ProgramStage::VS) {
				_context->VSSetConstantBuffers(slot, numBuffers, buffers);
			} else if constexpr (stage == ProgramStage::PS) {
				_context->PSSetConstantBuffers(slot, numBuffers, buffers);
			}
		}

		template<ProgramStage stage>
		inline void AE_CALL useSamplers(UINT slot, UINT numSamplers, ID3D11SamplerState*const* samplers) {
			if constexpr (stage == ProgramStage::VS) {
				_context->VSSetSamplers(slot, numSamplers, samplers);
			} else if constexpr (stage == ProgramStage::PS) {
				_context->PSSetSamplers(slot, numSamplers, samplers);
			}
		}

		static DXGI_FORMAT AE_CALL convertInternalFormat(TextureFormat fmt);

	private:
		RefPtr<Ref> _loader;
		RefPtr<Application> _app;

		DXGI_RATIONAL _refreshRate;
		D3D_FEATURE_LEVEL _featureLevel;
		std::string _shaderModel;
		//D3D_DRIVER_TYPE _driverType;
		ID3D11Device5* _device;
		ID3D11DeviceContext4* _context;
		IDXGISwapChain4* _swapChain;
		ID3D11RenderTargetView1* _backBufferTarget;
		D3D11_FEATURE_DATA_D3D11_OPTIONS _internalFeatures;

		GraphicsDeviceFeatures _deviceFeatures;
		std::string _deviceVersion;

		ConstantBufferManager _constantBufferManager;

		events::EventListener<ApplicationEvent, Graphics> _resizedListener;
		void AE_CALL _resizedHandler(events::Event<ApplicationEvent>& e);

		bool AE_CALL _createDevice(const GraphicsAdapter& adapter);

		void AE_CALL _release();
		void AE_CALL _resize(const Vec2<UINT>& size);

		IConstantBuffer* AE_CALL _createdShareConstantBuffer();
		IConstantBuffer* AE_CALL _createdExclusiveConstantBuffer(ui32 numParameters);
	};
}