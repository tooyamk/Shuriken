#pragma once

#include "Base.h"
#include "modules/graphics/ConstantBufferManager.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL Graphics : public IGraphicsModule {
	public:
		Graphics(Application* app);
		virtual ~Graphics();

		virtual bool AE_CALL createDevice(const GraphicsAdapter* adapter) override;

		virtual const std::string& AE_CALL getModuleVersion() const override;
		virtual const std::string& AE_CALL getDeviceVersion() const override;
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

		template<ProgramStage stage>
		inline void AE_CALL useShaderResources(UINT slot, UINT numViews, ID3D11ShaderResourceView*const* views) {}
		template<>
		inline void AE_CALL useShaderResources<ProgramStage::VS>(UINT slot, UINT numViews, ID3D11ShaderResourceView*const* views) {
			_context->VSSetShaderResources(slot, numViews, views);
		}
		template<>
		inline void AE_CALL useShaderResources<ProgramStage::PS>(UINT slot, UINT numViews, ID3D11ShaderResourceView*const* views) {
			_context->PSSetShaderResources(slot, numViews, views);
		}

		template<ProgramStage stage>
		inline void AE_CALL useConstantBuffers(UINT slot, UINT numBuffers, ID3D11Buffer*const* buffers) {}
		template<>
		inline void AE_CALL useConstantBuffers<ProgramStage::VS>(UINT slot, UINT numBuffers, ID3D11Buffer*const* buffers) {
			_context->VSSetConstantBuffers(slot, numBuffers, buffers);
		}
		template<>
		inline void AE_CALL useConstantBuffers<ProgramStage::PS>(UINT slot, UINT numBuffers, ID3D11Buffer*const* buffers) {
			_context->PSSetConstantBuffers(slot, numBuffers, buffers);
		}

		template<ProgramStage stage>
		inline void AE_CALL useSamplers(UINT slot, UINT numSamplers, ID3D11SamplerState*const* samplers) {}
		template<>
		inline void AE_CALL useSamplers<ProgramStage::VS>(UINT slot, UINT numSamplers, ID3D11SamplerState*const* samplers) {
			_context->VSSetSamplers(slot, numSamplers, samplers);
		}
		template<>
		inline void AE_CALL useSamplers<ProgramStage::PS>(UINT slot, UINT numSamplers, ID3D11SamplerState*const* samplers) {
			_context->PSSetSamplers(slot, numSamplers, samplers);
		}

		static DXGI_FORMAT AE_CALL convertInternalFormat(TextureFormat fmt);

	private:
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
		inline static const std::string _moduleVersion = "0.1.0";
		std::string _deviceVersion;

		ConstantBufferManager _constantBufferManager;

		events::EventListener<ApplicationEvent, Graphics> _resizedListener;
		void AE_CALL _resizedHandler(events::Event<ApplicationEvent>& e);

		bool AE_CALL _createDevice(const GraphicsAdapter& adapter);

		void AE_CALL _release();
		void AE_CALL _resize(const Vec2<UINT>& size);

		void AE_CALL _createdExclusiveConstantBuffer(IConstantBuffer* buffer, ui32 numParameters);
	};
}