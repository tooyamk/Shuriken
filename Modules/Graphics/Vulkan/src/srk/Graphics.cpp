#include "Graphics.h"
#include "CreateModule.h"
#include "BlendState.h"
#include "RasterizerState.h"
#include "srk/ProgramSource.h"
#include "srk/modules/graphics/GraphicsAdapter.h"



namespace srk::modules::graphics::vulkan {
	VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		return 1;
	}

	Graphics::Graphics() :
		_vulkanStatus({ 0 }),
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
		if (conf.win) {
			if (!conf.win->getNative(WindowNative::WINDOW)) return false;
#if SRK_OS == SRK_OS_WINDOWS
			auto app = conf.win->getApplication();
			if (!app || !app->getNative()) return false;
#endif
		} else {
			if (!conf.offscreen) return false;
		}

		if (conf.adapter) {
			conf.createProcessInfo("specific adapter create device...");
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
		auto success = false;

		do {
			if (!_vulkanInit(conf)) break;
			if (!_vulkanCreateSurface(*conf.win)) break;
			if (!_vulkanInitSwapchain()) break;

			VkPhysicalDeviceProperties physicalDeviceProperties;
			vkGetPhysicalDeviceProperties(_vulkanStatus.physicalDevice, &physicalDeviceProperties);

			_deviceFeatures.simultaneousRenderTargetCount = physicalDeviceProperties.limits.maxColorAttachments;

			success = true;
		} while (false);
		
		if (!success) _release();

		return success;
	}

	bool Graphics::_vulkanInit(const CreateConfig& conf) {
		using namespace std::literals;

		uint32_t enabledLayerCount = 0;
		const char* enabledLayerNames[1];

		uint32_t enabledInstanceExtensionCount = 0;
		const char* enabledInstanceExtensionNames[64];

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

		auto surfaceExtFound = false, platformSurfaceExtFound = false;
		do {
			uint32_t instanceExtensionCount;
			if (auto err = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr); err != VK_SUCCESS || !instanceExtensionCount) break;

			std::vector<VkExtensionProperties> instanceExtensions(instanceExtensionCount);
			if (vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensions.data()) != VK_SUCCESS) break;

			for (auto& extension : instanceExtensions) {
				if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, extension.extensionName)) {
					surfaceExtFound = true;
					enabledInstanceExtensionNames[enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
					continue;
				}

#if SRK_OS == SRK_OS_WINDOWS
				if (!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, extension.extensionName)) {
					platformSurfaceExtFound = true;
					enabledInstanceExtensionNames[enabledInstanceExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
					continue;
				}
#endif

				if (!strcmp(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, extension.extensionName)) {
					enabledInstanceExtensionNames[enabledInstanceExtensionCount++] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
					continue;
				}

				if (conf.debug && !strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, extension.extensionName)) {
					enabledInstanceExtensionNames[enabledInstanceExtensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
					continue;
				}
			}
		} while (false);

		if (!surfaceExtFound || !platformSurfaceExtFound) return false;

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
		.enabledExtensionCount = enabledInstanceExtensionCount,
		.ppEnabledExtensionNames = enabledInstanceExtensionNames,
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

		if (vkCreateInstance(&instanceInfo, nullptr, &_vulkanStatus.instance) != VK_SUCCESS) return false;

		VkPhysicalDevice physicalDevice = nullptr;
		do {
			uint32_t physicalDeviceCount = 0;
			if (auto err = vkEnumeratePhysicalDevices(_vulkanStatus.instance, &physicalDeviceCount, nullptr); err != VK_SUCCESS || !physicalDeviceCount) break;

			std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
			if (vkEnumeratePhysicalDevices(_vulkanStatus.instance, &physicalDeviceCount, physicalDevices.data()) != VK_SUCCESS) break;

			VkPhysicalDeviceProperties physicalDeviceProperties;
			if (conf.adapter) {
				for (auto& device : physicalDevices) {
					vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);
					if (physicalDeviceProperties.vendorID == conf.adapter->vendorId && physicalDeviceProperties.deviceID == conf.adapter->deviceId) {
						physicalDevice = device;
						break;
					}
				}
			}
			else {
				uint32_t countDeviceType[VK_PHYSICAL_DEVICE_TYPE_CPU + 1];
				memset(countDeviceType, 0, sizeof(countDeviceType));

				for (auto& device : physicalDevices) {
					vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);
					if (physicalDeviceProperties.deviceType <= VK_PHYSICAL_DEVICE_TYPE_CPU) ++countDeviceType[physicalDeviceProperties.deviceType];
				}

				auto searchForDeviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
				if (countDeviceType[VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU]) {
					searchForDeviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
				}
				else if (countDeviceType[VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU]) {
					searchForDeviceType = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
				}
				else if (countDeviceType[VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU]) {
					searchForDeviceType = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
				}
				else if (countDeviceType[VK_PHYSICAL_DEVICE_TYPE_CPU]) {
					searchForDeviceType = VK_PHYSICAL_DEVICE_TYPE_CPU;
				}
				else if (countDeviceType[VK_PHYSICAL_DEVICE_TYPE_OTHER]) {
					searchForDeviceType = VK_PHYSICAL_DEVICE_TYPE_OTHER;
				}

				for (auto& device : physicalDevices) {
					vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);
					if (physicalDeviceProperties.deviceType <= searchForDeviceType) {
						physicalDevice = device;
						break;
					}
				}
			}
		} while (false);

		if (!physicalDevice) return false;

		auto swapchainExtFound = false;
		do {
			uint32_t deviceExtensionCount = 0;
			if (auto err = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, nullptr); err != VK_SUCCESS || !deviceExtensionCount) break;

			std::vector<VkExtensionProperties> deviceExtensions(deviceExtensionCount);
			if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, deviceExtensions.data()) != VK_SUCCESS) break;

			auto& data = _vulkanStatus.enabledDeviceExtensions;
			for (auto& extension : deviceExtensions) {
				if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, extension.extensionName)) {
					swapchainExtFound = true;
					data.names[data.count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
				}
				if (!strcmp("VK_KHR_portability_subset", extension.extensionName)) data.names[data.count++] = "VK_KHR_portability_subset";
			}
		} while (false);

		if (!swapchainExtFound) return false;

		if (conf.debug) {
			auto createDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_vulkanStatus.instance, "vkCreateDebugUtilsMessengerEXT");
			auto destroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_vulkanStatus.instance, "vkDestroyDebugUtilsMessengerEXT");
			auto submitDebugUtilsMessageEXT = vkGetInstanceProcAddr(_vulkanStatus.instance, "vkSubmitDebugUtilsMessageEXT");
			auto cmdBeginDebugUtilsLabelEXT = vkGetInstanceProcAddr(_vulkanStatus.instance, "vkCmdBeginDebugUtilsLabelEXT");
			auto cmdEndDebugUtilsLabelEXT = vkGetInstanceProcAddr(_vulkanStatus.instance, "vkCmdEndDebugUtilsLabelEXT");
			auto cmdInsertDebugUtilsLabelEXT = vkGetInstanceProcAddr(_vulkanStatus.instance, "vkCmdInsertDebugUtilsLabelEXT");
			auto setDebugUtilsObjectNameEXT = vkGetInstanceProcAddr(_vulkanStatus.instance, "vkSetDebugUtilsObjectNameEXT");

			if (createDebugUtilsMessengerEXT && destroyDebugUtilsMessengerEXT && submitDebugUtilsMessageEXT &&
				cmdBeginDebugUtilsLabelEXT && cmdEndDebugUtilsLabelEXT && cmdInsertDebugUtilsLabelEXT && setDebugUtilsObjectNameEXT) {
				if (createDebugUtilsMessengerEXT(_vulkanStatus.instance, &debugMessengerCreateInfo, nullptr, &_vulkanStatus.debug.messenger) == VK_SUCCESS) {
					_vulkanStatus.debug.destroyUtilsMessengerEXT = destroyDebugUtilsMessengerEXT;
				}
			}
		}

		_vulkanStatus.physicalDevice = physicalDevice;

		return true;
	}

	bool Graphics::_vulkanCreateSurface(IWindow& win) {
		auto err = VK_ERROR_UNKNOWN;

#if SRK_OS == SRK_OS_WINDOWS
		VkWin32SurfaceCreateInfoKHR createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.hinstance = (HINSTANCE)win.getApplication()->getNative();
		createInfo.hwnd = (HWND)win.getNative(WindowNative::WINDOW);

		err = vkCreateWin32SurfaceKHR(_vulkanStatus.instance, &createInfo, nullptr, &_vulkanStatus.surface);
#endif

		return err == VK_SUCCESS;
	}

	bool Graphics::_vulkanInitSwapchain() {
		constexpr auto max = (std::numeric_limits<uint32_t>::max)();
		uint32_t graphicsQueueFamilyIndex = max, presentQueueFamilyIndex = max;
		do {
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(_vulkanStatus.physicalDevice, &queueFamilyCount, nullptr);
			if (queueFamilyCount < 1) return false;

			std::vector<VkQueueFamilyProperties> queueProps(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(_vulkanStatus.physicalDevice, &queueFamilyCount, queueProps.data());

			auto getPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vkGetInstanceProcAddr(_vulkanStatus.instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
			if (!getPhysicalDeviceSurfaceSupportKHR) return false;

			std::vector<VkBool32> supportsPresent(queueFamilyCount);
			for (decltype(queueFamilyCount) i = 0; i < queueFamilyCount; ++i) getPhysicalDeviceSurfaceSupportKHR(_vulkanStatus.physicalDevice, i, _vulkanStatus.surface, &supportsPresent.data()[i]);

			for (decltype(queueFamilyCount) i = 0; i < queueFamilyCount; ++i) {
				if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
					if (graphicsQueueFamilyIndex == max) graphicsQueueFamilyIndex = i;

					if (supportsPresent[i] == VK_TRUE) {
						graphicsQueueFamilyIndex = i;
						presentQueueFamilyIndex = i;

						break;
					}
				}
			}

			if (presentQueueFamilyIndex == max) {
				for (decltype(queueFamilyCount) i = 0; i < queueFamilyCount; ++i) {
					if (supportsPresent[i] == VK_TRUE) {
						presentQueueFamilyIndex = i;

						break;
					}
				}
			}

			if (graphicsQueueFamilyIndex == max || presentQueueFamilyIndex == max) return false;
		} while (false);

		if (!_vulkanCreateDevice(graphicsQueueFamilyIndex, presentQueueFamilyIndex)) return false;

		return true;
	}

	bool Graphics::_vulkanCreateDevice(uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex) {
		float32_t queuePriorities[1] = { 0.0f };

		VkDeviceQueueCreateInfo queues[2];
		auto& queue = queues[0];
		queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue.pNext = nullptr;
		queue.queueFamilyIndex = graphicsQueueFamilyIndex;
		queue.queueCount = 1;
		queue.pQueuePriorities = queuePriorities;
		queue.flags = 0;

		VkDeviceCreateInfo device = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = nullptr,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = queues,
			.enabledLayerCount = 0,
			.ppEnabledLayerNames = nullptr,
			.enabledExtensionCount = _vulkanStatus.enabledDeviceExtensions.count,
			.ppEnabledExtensionNames = (const char* const*)_vulkanStatus.enabledDeviceExtensions.names,
			.pEnabledFeatures = nullptr,
		};

		if (graphicsQueueFamilyIndex != presentQueueFamilyIndex) {
			queue = queues[1];
			queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue.pNext = nullptr;
			queue.queueFamilyIndex = presentQueueFamilyIndex;
			queue.queueCount = 1;
			queue.pQueuePriorities = queuePriorities;
			queue.flags = 0;
			device.queueCreateInfoCount = 2;
		}

		return vkCreateDevice(_vulkanStatus.physicalDevice, &device, nullptr, &_vulkanStatus.device) == VK_SUCCESS;
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
		return new RasterizerState(*this, false);
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
		if (_vulkanStatus.device) {
			vkDeviceWaitIdle(_vulkanStatus.device);
			vkDestroyDevice(_vulkanStatus.device, nullptr);
			_vulkanStatus.device = nullptr;
		}

		if (_vulkanStatus.debug.destroyUtilsMessengerEXT) {
			_vulkanStatus.debug.destroyUtilsMessengerEXT(_vulkanStatus.instance, _vulkanStatus.debug.messenger, nullptr);
			_vulkanStatus.debug.destroyUtilsMessengerEXT = nullptr;
			_vulkanStatus.debug.messenger = nullptr;
		}

		if (_vulkanStatus.surface) {
			vkDestroySurfaceKHR(_vulkanStatus.instance, _vulkanStatus.surface, nullptr);
			_vulkanStatus.surface = nullptr;
		}

		if (_vulkanStatus.instance != nullptr) {
			vkDestroyInstance(_vulkanStatus.instance, nullptr);
			_vulkanStatus.instance = nullptr;
		}

		_vulkanStatus.enabledDeviceExtensions.count = 0;
		_vulkanStatus.physicalDevice = nullptr;

		_deviceFeatures.reset();

		_deviceVersion = "Vulkan Unknown";
	}

	void Graphics::_resize(const Vec2ui32& size) {
	}
}