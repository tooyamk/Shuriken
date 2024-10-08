#pragma once

#include "Base.h"
#include "srk/Lock.h"
#include "srk/events/EventDispatcher.h"
#include "srk/modules/graphics/ConstantBufferManager.h"
#include "srk/modules/windows/WindowModule.h"

namespace srk::modules::graphics::vulkan {
	class BlendState;
	class DepthStencil;
	class DepthStencilState;
	class RasterizerState;

	class SRK_MODULE_DLL Graphics : public IGraphicsModule {
	public:
		struct InternalFeatures {
			bool dymanicRasterizerState;
			bool dynamicDepthStencilState;
			bool customBorderColor;

			bool extendedDynamicState;
			bool extendedDynamicState2;
			bool extendedDynamicState3;
			bool vertexInputDynamicState;
			bool colorWriteEnable;
		};


		Graphics();
		virtual ~Graphics();

		void SRK_CALL operator delete(Graphics* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Graphics();
			::operator delete(p);
		}

		virtual IntrusivePtr<events::IEventDispatcher<GraphicsEvent>> SRK_CALL getEventDispatcher() override;
		//virtual const events::IEventDispatcher<GraphicsEvent>& SRK_CALL getEventDispatcher() const override;

		virtual std::string_view SRK_CALL getVersion() const override;
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

		bool SRK_CALL createDevice(Ref* loader, const CreateGrahpicsModuleDescriptor& desc);

		inline void SRK_CALL error(const std::string_view& msg) {
			_eventDispatcher->dispatchEvent(this, GraphicsEvent::ERR, (std::string_view*)&msg);
		}

		inline VkPhysicalDevice SRK_CALL getVkPhysicalDevice() const {
			return _vkStatus.physicalDevice;
		}

		inline VkDevice SRK_CALL getVkDevice() const {
			return _vkStatus.device;
		}

		inline bool SRK_CALL isDebug() const {
			return _isDebug;
		}

		inline const InternalFeatures& SRK_CALL getInternalFeatures() const {
			return _internalFeatures;
		}

		inline const Usage SRK_CALL getBufferCreateUsageMask() const {
			return _vkStatus.usage.bufferCreateUsageMask;
		}

		inline const Usage SRK_CALL getTexCreateUsageMask() const {
			return _vkStatus.usage.texCreateUsageMask;
		}

		inline VmaAllocator SRK_CALL getMemAllocator() const {
			return _vkStatus.memAllocator;
		}

		const VkAllocationCallbacks* SRK_CALL getVkAllocationCallbacks() const;

		inline VkCommandPool SRK_CALL getVkCommandPool() const {
			return _vkStatus.cmd.pool;
		}

		inline VkQueue SRK_CALL getVkGraphicsQueue() const {
			return _vkStatus.cmd.graphicsQueue;
		}

		VkFence SRK_CALL getFenceFromPool();

		inline void SRK_CALL putFenceToPool(VkFence fence) {
			if (!fence) return;

			std::scoped_lock lck(_fencesLock);

			_fencesPool.emplace_back(fence);
		}

		InternalCommandBuffer beginOneTimeCommands();
		bool endOneTimeCommands(InternalCommandBuffer& buffer);
		static void transitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageSubresourceRange& range);

		inline ConstantBufferManager& SRK_CALL getConstantBufferManager() {
			return _constantBufferManager;
		}

		int32_t findProperties(uint32_t memoryTypeBits, VkMemoryPropertyFlags flags);

		static VkCompareOp SRK_CALL convertCompareOp(ComparisonFunc func);
		static VkStencilOp SRK_CALL convertStencilOp(StencilOp func);
		static VkBlendFactor SRK_CALL convertBlendFactor(BlendFactor factor);
		static VkBlendOp SRK_CALL convertBlendOp(BlendOp op);
		static VkPolygonMode SRK_CALL convertFillMode(FillMode mode);
		static VkCullModeFlagBits SRK_CALL convertCullMode(CullMode mode);
		static VkFrontFace SRK_CALL convertFrontFace(FrontFace mode);
		static VkShaderStageFlagBits SRK_CALL convertProgramStage(ProgramStage stage);
		static VkFormat SRK_CALL convertVertexFormat(VertexFormat fmt);
		static VkImageType SRK_CALL convertTextureType(TextureType type);
		static VkFormat SRK_CALL convertTextureFormat(TextureFormat fmt);
		static VkSampleCountFlagBits SRK_CALL convertSampleCount(SampleCount sc);
		static VkSamplerAddressMode SRK_CALL convertSamplerAddressMode(SamplerAddressMode mode);

	private:
		struct FenceDeleter {
			FenceDeleter(Graphics* g) : _g(g) {}
			FenceDeleter(const FenceDeleter& other) : _g(other._g) {}
			FenceDeleter(FenceDeleter&& other) : _g(other._g) {}

			inline void SRK_CALL operator()(VkFence f) const {
				_g->putFenceToPool(f);
			};

		private:
			Graphics* _g;
		};


		struct PipelineCluster {
			VkPipeline basePipeline;
			std::unordered_map<uint64_t, VkPipeline> pipelines;
		};


		bool _isDebug;
		bool _curIsBackBuffer;
		SampleCount _backBufferSampleCount;
		IntrusivePtr<Ref> _loader;
		IntrusivePtr<windows::IWindow> _win;

		InternalFeatures _internalFeatures;
		GraphicsDeviceFeatures _deviceFeatures;
		std::string _deviceVersion;

		std::vector<VkDynamicState> _dynamicStates;
		VkPipelineDynamicStateCreateInfo _pipelineDynamicStateCreateInfo;

		AtomicLock _fencesLock;
		std::vector<VkFence> _fencesPool;

		IntrusivePtr<BlendState> _defaultBlendState;
		IntrusivePtr<DepthStencilState> _defaultDepthStencilState;
		IntrusivePtr<RasterizerState> _defaultRasterizerState;

		struct {
			uint32_t apiVersion = 0;

			VkInstance instance = nullptr;
			VkPhysicalDevice physicalDevice = nullptr;
			VkSurfaceKHR surface = nullptr;
			VkDevice device = nullptr;
			VmaAllocator memAllocator = nullptr;

			VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
			VkPhysicalDeviceFeatures physicalDeviceFeatures;

			struct {
				uint32_t graphics = 0;
				uint32_t present = 0;
			} queueFamilyIndices;

			struct {
				uint32_t count = 0;
				const char* names[64];
			} enabledDeviceExtensions;

			struct {
				PFN_vkDestroyDebugUtilsMessengerEXT destroyUtilsMessengerEXT = nullptr;
				VkDebugUtilsMessengerEXT messenger = nullptr;
			} debug;

			struct {
				uint64_t featureValue = 0;
				VkPipelineColorBlendStateCreateInfo info;
			} blend;

			struct {
				DepthStencilFeature featureValue;
				VkPipelineDepthStencilStateCreateInfo info;
			} depthStencil;

			struct {
				RasterizerFeature featureValue;
				VkPipelineRasterizationStateCreateInfo info;
			} rasterizer;

			struct {
				VkPipelineCache cache = nullptr;
				/*VkPipeline pipeline = nullptr;
				DepthStencilFeature depthStencilFeatureValue;
				RasterizerFeature rasterizerFeatureValue;*/
				std::unordered_map<uint32_t, PipelineCluster> pipelines;
			} pipeline;

			struct {
				VkFormat format = VK_FORMAT_UNDEFINED;
				
				VkSwapchainKHR swapChain = nullptr;
				std::vector<VkImage> images;
				std::vector<VkImageView> imageViews;
			} swapChain;

			struct {
				VkCommandPool pool = nullptr;
				VkQueue graphicsQueue = nullptr;
			} cmd;

			struct {
				Usage bufferCreateUsageMask = Usage::NONE;
				Usage texCreateUsageMask = Usage::NONE;
			} usage;

			Vec2<uint32_t> backSize;
			Box2i32ui32 viewport;
			Box2i32ui32 scissor;
		} _vkStatus;

#if SRK_OS == SRK_OS_WINDOWS
		HWND _hwnd;
#endif

		ConstantBufferManager _constantBufferManager;

		IntrusivePtr<events::IEventDispatcher<GraphicsEvent>> _eventDispatcher;

		bool SRK_CALL _createDevice(Ref* loader, const CreateGrahpicsModuleDescriptor& desc, const GraphicsAdapter* adapter);
		bool SRK_CALL _getVkPhysicalDevice(const GraphicsAdapter* adapter);
		bool SRK_CALL _createVkInstance(bool debug);
		bool SRK_CALL _createVkSurface(windows::IWindow& win);
		bool SRK_CALL _createVkDevice();
		bool SRK_CALL _createMemAllocator();
		bool SRK_CALL _createVkCommandPool();
		bool SRK_CALL _createVkSwapchain(const Vec2ui32& size);

		void SRK_CALL _setBlendState(BlendState& state, uint32_t sampleMask);
		void SRK_CALL _setDepthStencilState(DepthStencilState& state);
		void SRK_CALL _setRasterizerState(RasterizerState& state);

		VkPipeline SRK_CALL _checkAndUpdateVkPipeline(IProgram* program, const IVertexAttributeGetter* vertexAttributeGetter, const IShaderParameterGetter* shaderParamGetter);

		void SRK_CALL _release();
		void SRK_CALL _resize(const Vec2ui32& size);

		void SRK_CALL _cleanupSwapChain(VkSwapchainKHR* swapChain);

		IConstantBuffer* SRK_CALL _createdShareConstantBuffer();
		IConstantBuffer* SRK_CALL _createdExclusiveConstantBuffer(uint32_t numParameters);
	};
}
