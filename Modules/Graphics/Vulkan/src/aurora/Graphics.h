#pragma once

#include "Base.h"
#include "aurora/modules/graphics/ConstantBufferManager.h"

namespace aurora::modules::graphics::vulkan {
	class BlendState;
	class DepthStencil;
	class DepthStencilState;
	class RasterizerState;

	class AE_MODULE_DLL Graphics : public IGraphicsModule {
	public:
		struct CreateConfig {
			Ref* loader = nullptr;
			IApplication* app = nullptr;
			GraphicsAdapter* adapter = nullptr;
			SampleCount sampleCount = 1;
			std::string driverType;
			std::function<void(const std::string_view&)>* createProcessInfoHandler = nullptr;
			bool debug = false;
			bool offscreen = false;

			inline void AE_CALL createProcessInfo(const std::string_view& msg) const {
				if (createProcessInfoHandler && *createProcessInfoHandler) (*createProcessInfoHandler)(msg);
			}
		};


		Graphics();
		virtual ~Graphics();

		void operator delete(Graphics* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Graphics();
			::operator delete(p);
		}

		virtual IntrusivePtr<events::IEventDispatcher<GraphicsEvent>> AE_CALL getEventDispatcher() override;
		//virtual const events::IEventDispatcher<GraphicsEvent>& AE_CALL getEventDispatcher() const override;

		virtual const std::string& AE_CALL getVersion() const override;
		virtual const GraphicsDeviceFeatures& AE_CALL getDeviceFeatures() const override;
		virtual IntrusivePtr<IBlendState> AE_CALL createBlendState() override;
		virtual IntrusivePtr<IConstantBuffer> AE_CALL createConstantBuffer() override;
		virtual IntrusivePtr<IDepthStencil> AE_CALL createDepthStencil() override;
		virtual IntrusivePtr<IDepthStencilState> AE_CALL createDepthStencilState() override;
		virtual IntrusivePtr<IIndexBuffer> AE_CALL createIndexBuffer() override;
		virtual IntrusivePtr<IProgram> AE_CALL createProgram() override;
		virtual IntrusivePtr<IRasterizerState> AE_CALL createRasterizerState() override;
		virtual IntrusivePtr<IRenderTarget> AE_CALL createRenderTarget() override;
		virtual IntrusivePtr<IRenderView> AE_CALL createRenderView() override;
		virtual IntrusivePtr<ISampler> AE_CALL createSampler() override;
		virtual IntrusivePtr<ITexture1DResource> AE_CALL createTexture1DResource() override;
		virtual IntrusivePtr<ITexture2DResource> AE_CALL createTexture2DResource() override;
		virtual IntrusivePtr<ITexture3DResource> AE_CALL createTexture3DResource() override;
		virtual IntrusivePtr<ITextureView> AE_CALL createTextureView() override;
		virtual IntrusivePtr<IVertexBuffer> AE_CALL createVertexBuffer() override;
		virtual IntrusivePtr<IPixelBuffer> AE_CALL createPixelBuffer() override;

		virtual const Vec2ui32& AE_CALL getBackBufferSize() const override;
		virtual void AE_CALL setBackBufferSize(const Vec2ui32& size) override;
		virtual Box2i32ui32 AE_CALL getViewport() const override;
		virtual void AE_CALL setViewport(const Box2i32ui32& vp) override;
		virtual void AE_CALL setBlendState(IBlendState* state, const Vec4f32& constantFactors, uint32_t sampleMask = (std::numeric_limits<uint32_t>::max)()) override;
		virtual void AE_CALL setDepthStencilState(IDepthStencilState* state, uint32_t stencilFrontRef, uint32_t stencilBackRef) override;
		virtual void AE_CALL setRasterizerState(IRasterizerState* state) override;
		
		virtual void AE_CALL beginRender() override;
		virtual void AE_CALL draw(const IVertexBufferGetter* vertexBufferGetter, IProgram* program, const IShaderParameterGetter* shaderParamGetter,
			const IIndexBuffer* indexBuffer, uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) override;
		virtual void AE_CALL drawInstanced(const IVertexBufferGetter* vertexBufferGetter, IProgram* program, const IShaderParameterGetter* shaderParamGetter,
			const IIndexBuffer* indexBuffer, uint32_t instancedCount, uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) override;
		virtual void AE_CALL endRender() override;
		virtual void AE_CALL flush() override;
		virtual void AE_CALL present() override;

		virtual void AE_CALL setRenderTarget(IRenderTarget* rt) override;
		virtual void AE_CALL clear(ClearFlag flags, const Vec4f32& color, float32_t depth, size_t stencil) override;

		bool AE_CALL createDevice(const CreateConfig& conf);

		inline void AE_CALL error(const std::string_view& msg) {
			_eventDispatcher->dispatchEvent(this, GraphicsEvent::ERR, (std::string_view*)&msg);
		}

		inline bool AE_CALL isDebug() const {
			return _isDebug;
		}

		inline ConstantBufferManager& AE_CALL getConstantBufferManager() {
			return _constantBufferManager;
		}

	private:
		bool _isDebug;
		bool _curIsBackBuffer;
		SampleCount _backBufferSampleCount;
		IntrusivePtr<Ref> _loader;
		IntrusivePtr<IApplication> _app;

		GraphicsDeviceFeatures _deviceFeatures;
		std::string _deviceVersion;

		struct {
			VkInstance instance;
			VkPhysicalDevice physicalDevice;
			VkSurfaceKHR surface;
			VkDevice device;

			struct {
				uint32_t count;
				const char* names[64];
			} enabledDeviceExtensions;

			struct {
				PFN_vkDestroyDebugUtilsMessengerEXT destroyUtilsMessengerEXT;
				VkDebugUtilsMessengerEXT messenger;
			} debug;

			Vec2<UINT> backSize;
			Box2i32ui32 vp;
		} _vulkanStatus;

		ConstantBufferManager _constantBufferManager;

		IntrusivePtr<events::IEventDispatcher<GraphicsEvent>> _eventDispatcher;

		bool AE_CALL _createDevice(const CreateConfig& conf);
		bool AE_CALL _vulkanInit(const CreateConfig& conf);
		bool AE_CALL _vulkanCreateSurface(IApplication& app);
		bool AE_CALL _vulkanInitSwapchain();
		bool AE_CALL _vulkanCreateDevice(uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex);
		void AE_CALL _release();
		void AE_CALL _resize(const Vec2ui32& size);
	};
}