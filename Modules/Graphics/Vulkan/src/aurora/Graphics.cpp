#include "Graphics.h"
#include "CreateModule.h"
#include "aurora/ProgramSource.h"
#include "aurora/modules/graphics/GraphicsAdapter.h"



namespace aurora::modules::graphics::vulkan {
	VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		return 1;
	}

	Graphics::Graphics() :
		_isDebug(false),
		_curIsBackBuffer(true),
		_backBufferSampleCount(1),
		_eventDispatcher(new events::EventDispatcher<GraphicsEvent>()) {
		_vulkanStatus.instance = nullptr;
	}

	Graphics::~Graphics() {
		_release();
	}

	bool Graphics::createDevice(const CreateConfig& conf) {
		using namespace std::literals;

		uint32_t enabledLayerCount = 0;
		const char* enabledLayerNames[1];

		uint32_t enabledExtensionCount = 0;
		const char* enabledExtensionNames[64];

		if (conf.debug) {
			do {
				uint32_t instanceLayerCount;
				if (auto err = vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr); err != VK_SUCCESS || !instanceLayerCount) break;

				std::vector<VkLayerProperties> instanceLayers(instanceLayerCount);
				if (vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayers.data()) != VK_SUCCESS) break;

				for (auto& layer : instanceLayers) {
					if (!strcmp("VK_LAYER_KHRONOS_validation", layer.layerName)) {
						enabledLayerNames[enabledLayerCount++] = "VK_LAYER_KHRONOS_validation";
						break;
					}
				}
			} while (false);
		}

		VkBool32 surfaceExtFound = 0, platformSurfaceExtFound = 0;
		do {
			uint32_t instanceExtensionCount;
			if (auto err = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr); err != VK_SUCCESS || !instanceExtensionCount) break;

			std::vector<VkExtensionProperties> instanceExtensions(instanceExtensionCount);
			if (vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensions.data()) != VK_SUCCESS) break;

			for (auto& extension : instanceExtensions) {
				if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, extension.extensionName)) {
					surfaceExtFound = 1;
					enabledExtensionNames[enabledExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
					continue;
				}

#if AE_OS == AE_OS_WINDOWS
				if (!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, extension.extensionName)) {
					platformSurfaceExtFound = 1;
					enabledExtensionNames[enabledExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
					continue;
				}
#endif

				if (!strcmp(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, extension.extensionName)) {
					enabledExtensionNames[enabledExtensionCount++] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
					continue;
				}

				if (conf.debug && !strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, extension.extensionName)) {
					enabledExtensionNames[enabledExtensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
					continue;
				}
			}
		} while (false);

		if (!surfaceExtFound || !platformSurfaceExtFound) {
			_release();
			return false;
		}

		const VkApplicationInfo appInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = nullptr,
		.pApplicationName = "",
		.applicationVersion = 0,
		.pEngineName = "",
		.engineVersion = 0,
		.apiVersion = VK_API_VERSION_1_0,
		};

		VkInstanceCreateInfo instanceInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = nullptr,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = enabledLayerCount,
		.ppEnabledLayerNames = enabledLayerNames,
		.enabledExtensionCount = enabledExtensionCount,
		.ppEnabledExtensionNames = enabledExtensionNames,
		};

		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo;
		if (conf.debug) {
			debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugMessengerCreateInfo.pNext = nullptr;
			debugMessengerCreateInfo.flags = 0;
			debugMessengerCreateInfo.messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugMessengerCreateInfo.pfnUserCallback = debugMessengerCallback;
			debugMessengerCreateInfo.pUserData = this;
			instanceInfo.pNext = &debugMessengerCreateInfo;
		}

		if (vkCreateInstance(&instanceInfo, nullptr, &_vulkanStatus.instance) != VK_SUCCESS) {
			_release();
			return false;
		}

		do {
			uint32_t physicalDeviceCount = 0;
			if (auto err = vkEnumeratePhysicalDevices(_vulkanStatus.instance, &physicalDeviceCount, nullptr); err != VK_SUCCESS || !physicalDeviceCount) break;

			std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
			if (vkEnumeratePhysicalDevices(_vulkanStatus.instance, &physicalDeviceCount, physicalDevices.data()) != VK_SUCCESS) break;

			/*uint32_t countDeviceType[VK_PHYSICAL_DEVICE_TYPE_CPU + 1];
			memset(countDeviceType, 0, sizeof(countDeviceType));

			VkPhysicalDeviceProperties physicalDeviceProperties;
			if (conf.adapter) {
				for (auto& device : physicalDevices) {
					vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);
					if (physicalDeviceProperties.vendorID == conf.adapter->vendorId && physicalDeviceProperties.deviceID == conf.adapter->deviceId) {
						break;
					}
				}
			} else {

			}
			for (auto& device : physicalDevices) {
				vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);
				if (physicalDeviceProperties.deviceType <= VK_PHYSICAL_DEVICE_TYPE_CPU) ++countDeviceType[physicalDeviceProperties.deviceType];
			}

			auto searchForDeviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
			if (countDeviceType[VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU]) {
				searchForDeviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
			} else if (countDeviceType[VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU]) {
				searchForDeviceType = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
			} else if (countDeviceType[VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU]) {
				searchForDeviceType = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
			} else if (countDeviceType[VK_PHYSICAL_DEVICE_TYPE_CPU]) {
				searchForDeviceType = VK_PHYSICAL_DEVICE_TYPE_CPU;
			} else if (countDeviceType[VK_PHYSICAL_DEVICE_TYPE_OTHER]) {
				searchForDeviceType = VK_PHYSICAL_DEVICE_TYPE_OTHER;
			}*/

			int a = 1;
		} while (false);

		return false;
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
		return nullptr;
	}

	IntrusivePtr<IConstantBuffer> Graphics::createConstantBuffer() {
		return nullptr;
	}

	IntrusivePtr<IDepthStencil> Graphics::createDepthStencil() {
		return nullptr;
	}

	IntrusivePtr<IDepthStencilState> Graphics::createDepthStencilState() {
		return nullptr;
	}

	IntrusivePtr<IIndexBuffer> Graphics::createIndexBuffer() {
		return nullptr;
	}

	IntrusivePtr<IProgram> Graphics::createProgram() {
		return nullptr;
	}

	IntrusivePtr<IRasterizerState> Graphics::createRasterizerState() {
		return nullptr;
	}

	IntrusivePtr<IRenderTarget> Graphics::createRenderTarget() {
		return nullptr;
	}

	IntrusivePtr<IRenderView> Graphics::createRenderView() {
		return nullptr;
	}

	IntrusivePtr<ISampler> Graphics::createSampler() {
		return nullptr;
	}

	IntrusivePtr<ITexture1DResource> Graphics::createTexture1DResource() {
		return nullptr;
	}

	IntrusivePtr<ITexture2DResource> Graphics::createTexture2DResource() {
		return nullptr;
	}

	IntrusivePtr<ITexture3DResource> Graphics::createTexture3DResource() {
		return nullptr;
	}

	IntrusivePtr<ITextureView> Graphics::createTextureView() {
		return nullptr;
	}

	IntrusivePtr<IVertexBuffer> Graphics::createVertexBuffer() {
		return nullptr;
	}

	IntrusivePtr<IPixelBuffer> Graphics::createPixelBuffer() {
		return nullptr;
	}

	const Vec2ui32& Graphics::getBackBufferSize() const {
		return _vulkanStatus.backSize;
	}

	void Graphics::setBackBufferSize(const Vec2ui32& size) {
		_resize(size);
	}

	Box2i32ui32 Graphics::getViewport() const {
		return _vulkanStatus.vp;
	}

	void Graphics::setViewport(const Box2i32ui32& vp) {
	}

	void Graphics::setBlendState(IBlendState* state, const Vec4f32& constantFactors, uint32_t sampleMask) {
	}

	void Graphics::setDepthStencilState(IDepthStencilState* state, uint32_t stencilFrontRef, uint32_t stencilBackRef) {
	}

	void Graphics::setRasterizerState(IRasterizerState* state) {
	}

	void Graphics::beginRender() {
	}

	void Graphics::draw(const IVertexBufferGetter* vertexBufferGetter, IProgram* program, const IShaderParameterGetter* shaderParamGetter,
		const IIndexBuffer* indexBuffer, uint32_t count, uint32_t offset) {
	}

	void Graphics::drawInstanced(const IVertexBufferGetter* vertexBufferGetter, IProgram* program, const IShaderParameterGetter* shaderParamGetter,
		const IIndexBuffer* indexBuffer, uint32_t instancedCount, uint32_t count, uint32_t offset) {
	}

	void Graphics::endRender() {
	}

	void Graphics::flush() {
	}

	void Graphics::present() {
	}

	void Graphics::setRenderTarget(IRenderTarget* rt) {
	}

	void Graphics::clear(ClearFlag flags, const Vec4f32& color, float32_t depth, size_t stencil) {
	}

	void Graphics::_release() {
		if (_vulkanStatus.instance != nullptr) {
			vkDestroyInstance(_vulkanStatus.instance, nullptr);
			_vulkanStatus.instance = nullptr;
		}

		_deviceFeatures.reset();

		_deviceVersion = "Vulkan Unknown";
	}

	void Graphics::_resize(const Vec2ui32& size) {
	}
}