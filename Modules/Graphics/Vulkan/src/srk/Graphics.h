#pragma once

#include "Base.h"
#include "srk/events/EventDispatcher.h"
#include "srk/modules/graphics/ConstantBufferManager.h"
#include "srk/modules/windows/IWindowModule.h"

namespace srk::modules::graphics::vulkan {
	class BlendState;
	class DepthStencil;
	class DepthStencilState;
	class RasterizerState;

	class SRK_MODULE_DLL Graphics : public IGraphicsModule {
	public:
		struct CreateConfig {
			Ref* loader = nullptr;
			windows::IWindow* win = nullptr;
			GraphicsAdapter* adapter = nullptr;
			SampleCount sampleCount = 1;
			std::string driverType;
			std::function<void(const std::string_view&)>* createProcessInfoHandler = nullptr;
			bool debug = false;
			bool offscreen = false;

			inline void SRK_CALL createProcessInfo(const std::string_view& msg) const {
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
		virtual IntrusivePtr<IPixelBuffer> SRK_CALL createPixelBuffer() override;

		virtual const Vec2ui32& SRK_CALL getBackBufferSize() const override;
		virtual void SRK_CALL setBackBufferSize(const Vec2ui32& size) override;
		virtual Box2i32ui32 SRK_CALL getViewport() const override;
		virtual void SRK_CALL setViewport(const Box2i32ui32& vp) override;
		virtual void SRK_CALL setBlendState(IBlendState* state, const Vec4f32& constantFactors, uint32_t sampleMask = (std::numeric_limits<uint32_t>::max)()) override;
		virtual void SRK_CALL setDepthStencilState(IDepthStencilState* state, uint32_t stencilFrontRef, uint32_t stencilBackRef) override;
		virtual void SRK_CALL setRasterizerState(IRasterizerState* state) override;
		
		virtual void SRK_CALL beginRender() override;
		virtual void SRK_CALL draw(const IVertexBufferGetter* vertexBufferGetter, IProgram* program, const IShaderParameterGetter* shaderParamGetter,
			const IIndexBuffer* indexBuffer, uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) override;
		virtual void SRK_CALL drawInstanced(const IVertexBufferGetter* vertexBufferGetter, IProgram* program, const IShaderParameterGetter* shaderParamGetter,
			const IIndexBuffer* indexBuffer, uint32_t instancedCount, uint32_t count = (std::numeric_limits<uint32_t>::max)(), uint32_t offset = 0) override;
		virtual void SRK_CALL endRender() override;
		virtual void SRK_CALL flush() override;
		virtual void SRK_CALL present() override;

		virtual void SRK_CALL setRenderTarget(IRenderTarget* rt) override;
		virtual void SRK_CALL clear(ClearFlag flags, const Vec4f32& color, float32_t depth, size_t stencil) override;

		bool SRK_CALL createDevice(const CreateConfig& conf);

		inline void SRK_CALL error(const std::string_view& msg) {
			_eventDispatcher->dispatchEvent(this, GraphicsEvent::ERR, (std::string_view*)&msg);
		}

		inline bool SRK_CALL isDebug() const {
			return _isDebug;
		}

		inline ConstantBufferManager& SRK_CALL getConstantBufferManager() {
			return _constantBufferManager;
		}

	private:
		bool _isDebug;
		bool _curIsBackBuffer;
		SampleCount _backBufferSampleCount;
		IntrusivePtr<Ref> _loader;
		IntrusivePtr<windows::IWindow> _win;

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

			Vec2<uint32_t> backSize;
			Box2i32ui32 vp;
		} _vulkanStatus;

		ConstantBufferManager _constantBufferManager;

		IntrusivePtr<events::IEventDispatcher<GraphicsEvent>> _eventDispatcher;

		bool SRK_CALL _createDevice(const CreateConfig& conf);
		bool SRK_CALL _vulkanInit(const CreateConfig& conf);
		bool SRK_CALL _vulkanCreateSurface(windows::IWindow& win);
		bool SRK_CALL _vulkanInitSwapchain();
		bool SRK_CALL _vulkanCreateDevice(uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex);
		void SRK_CALL _release();
		void SRK_CALL _resize(const Vec2ui32& size);
	};
}
