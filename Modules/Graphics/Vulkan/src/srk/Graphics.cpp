#include "Graphics.h"
#include "CreateModule.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "RasterizerState.h"
#include "srk/ProgramSource.h"
#include "srk/modules/graphics/GraphicsAdapter.h"

namespace srk::modules::graphics::vulkan {
	VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		auto g = (Graphics*)pUserData;
		return VK_FALSE;
	}

	Graphics::Graphics() :
		_isDebug(false),
		_curIsBackBuffer(true),
		_backBufferSampleCount(1),
		_eventDispatcher(new events::EventDispatcher<GraphicsEvent>()) {
	}

	Graphics::~Graphics() {
		_release();
	}

	bool Graphics::createDevice(const CreateConfig& conf) {
		if (conf.win) {
			if (!conf.win->getNative(windows::WindowNative::WINDOW)) return false;
#if SRK_OS == SRK_OS_WINDOWS
			//if (!conf.win->getNative(windows::WindowNative::MODULE)) return false;
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
			if (!_createVkInstance(conf.debug)) break;
			if (!_createVkSurface(*conf.win)) break;
			if (!_getVkPhysicalDevice(conf)) break;
			if (!_createVkDevice()) break;
			if (!_createVkSwapchain()) break;

			VkPhysicalDeviceProperties physicalDeviceProperties;
			vkGetPhysicalDeviceProperties(_vkStatus.physicalDevice, &physicalDeviceProperties);

			_deviceFeatures.stencilIndependentMask = true;
			_deviceFeatures.stencilIndependentRef = true;
			_deviceFeatures.simultaneousRenderTargetCount = physicalDeviceProperties.limits.maxColorAttachments;

			success = true;
		} while (false);
		
		if (!success) _release();

		return success;
	}

	bool Graphics::_getVkPhysicalDevice(const CreateConfig& conf) {
		VkPhysicalDevice physicalDevice = nullptr;
		do {
			uint32_t physicalDeviceCount = 0;
			if (auto err = vkEnumeratePhysicalDevices(_vkStatus.instance, &physicalDeviceCount, nullptr); err != VK_SUCCESS || !physicalDeviceCount) break;

			std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
			if (vkEnumeratePhysicalDevices(_vkStatus.instance, &physicalDeviceCount, physicalDevices.data()) != VK_SUCCESS) break;

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

			auto& data = _vkStatus.enabledDeviceExtensions;
			for (auto& extension : deviceExtensions) {
				if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, extension.extensionName)) {
					swapchainExtFound = true;
					data.names[data.count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
				}
				if (!strcmp("VK_KHR_portability_subset", extension.extensionName)) data.names[data.count++] = "VK_KHR_portability_subset";
			}
		} while (false);

		if (!swapchainExtFound) return false;

		_vkStatus.physicalDevice = physicalDevice;

		return true;
	}

	bool Graphics::_createVkInstance(bool debug) {
		using namespace std::literals;

		uint32_t enabledLayerCount = 0;
		const char* enabledLayerNames[1];

		uint32_t enabledInstanceExtensionCount = 0;
		const char* enabledInstanceExtensionNames[64];

		if (debug) {
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

				if (debug && !strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, extension.extensionName)) {
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
		if (debug) {
			debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugMessengerCreateInfo.pNext = nullptr;
			debugMessengerCreateInfo.flags = 0;
			debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugMessengerCreateInfo.pfnUserCallback = debugMessengerCallback;
			debugMessengerCreateInfo.pUserData = this;
			instanceInfo.pNext = &debugMessengerCreateInfo;
		}

		if (vkCreateInstance(&instanceInfo, nullptr, &_vkStatus.instance) != VK_SUCCESS) return false;

		if (debug) {
			auto createDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_vkStatus.instance, "vkCreateDebugUtilsMessengerEXT");
			auto destroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_vkStatus.instance, "vkDestroyDebugUtilsMessengerEXT");
			auto submitDebugUtilsMessageEXT = vkGetInstanceProcAddr(_vkStatus.instance, "vkSubmitDebugUtilsMessageEXT");
			auto cmdBeginDebugUtilsLabelEXT = vkGetInstanceProcAddr(_vkStatus.instance, "vkCmdBeginDebugUtilsLabelEXT");
			auto cmdEndDebugUtilsLabelEXT = vkGetInstanceProcAddr(_vkStatus.instance, "vkCmdEndDebugUtilsLabelEXT");
			auto cmdInsertDebugUtilsLabelEXT = vkGetInstanceProcAddr(_vkStatus.instance, "vkCmdInsertDebugUtilsLabelEXT");
			auto setDebugUtilsObjectNameEXT = vkGetInstanceProcAddr(_vkStatus.instance, "vkSetDebugUtilsObjectNameEXT");

			if (createDebugUtilsMessengerEXT && destroyDebugUtilsMessengerEXT && submitDebugUtilsMessageEXT &&
				cmdBeginDebugUtilsLabelEXT && cmdEndDebugUtilsLabelEXT && cmdInsertDebugUtilsLabelEXT && setDebugUtilsObjectNameEXT) {
				if (createDebugUtilsMessengerEXT(_vkStatus.instance, &debugMessengerCreateInfo, nullptr, &_vkStatus.debug.messenger) == VK_SUCCESS) {
					_vkStatus.debug.destroyUtilsMessengerEXT = destroyDebugUtilsMessengerEXT;
				}
			}
		}

		return true;
	}

	bool Graphics::_createVkSurface(windows::IWindow& win) {
		auto err = VK_ERROR_UNKNOWN;

#if SRK_OS == SRK_OS_WINDOWS
		VkWin32SurfaceCreateInfoKHR createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.hinstance = GetModuleHandleW(nullptr);
		createInfo.hwnd = (HWND)win.getNative(windows::WindowNative::WINDOW);

		err = vkCreateWin32SurfaceKHR(_vkStatus.instance, &createInfo, nullptr, &_vkStatus.surface);
#endif

		return err == VK_SUCCESS;
	}

	bool Graphics::_createVkDevice() {
		uint32_t graphicsQueueFamilyIndex, presentQueueFamilyIndex;
		{
			constexpr auto max = (std::numeric_limits<uint32_t>::max)();
			graphicsQueueFamilyIndex = max;
			presentQueueFamilyIndex = max;

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(_vkStatus.physicalDevice, &queueFamilyCount, nullptr);
			if (queueFamilyCount < 1) return false;

			std::vector<VkQueueFamilyProperties> queueProps(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(_vkStatus.physicalDevice, &queueFamilyCount, queueProps.data());

			auto getPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vkGetInstanceProcAddr(_vkStatus.instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
			if (!getPhysicalDeviceSurfaceSupportKHR) return false;

			std::vector<VkBool32> supportsPresent(queueFamilyCount);
			for (decltype(queueFamilyCount) i = 0; i < queueFamilyCount; ++i) getPhysicalDeviceSurfaceSupportKHR(_vkStatus.physicalDevice, i, _vkStatus.surface, &supportsPresent.data()[i]);

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
		}

		_vkStatus.graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
		_vkStatus.presentQueueFamilyIndex = presentQueueFamilyIndex;

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
			.enabledExtensionCount = _vkStatus.enabledDeviceExtensions.count,
			.ppEnabledExtensionNames = (const char* const*)_vkStatus.enabledDeviceExtensions.names,
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

		return vkCreateDevice(_vkStatus.physicalDevice, &device, nullptr, &_vkStatus.device) == VK_SUCCESS;
	}

	bool Graphics::_createVkSwapchain() {
		VkSurfaceFormatKHR selectedSurfaceFormat;
		{
			uint32_t formatCount;
			if (vkGetPhysicalDeviceSurfaceFormatsKHR(_vkStatus.physicalDevice, _vkStatus.surface, &formatCount, nullptr) != VK_SUCCESS) return false;
			if (formatCount < 1) return false;
			std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
			if (vkGetPhysicalDeviceSurfaceFormatsKHR(_vkStatus.physicalDevice, _vkStatus.surface, &formatCount, surfaceFormats.data()) != VK_SUCCESS) return false;

			selectedSurfaceFormat.format = VK_FORMAT_UNDEFINED;
			if (formatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
				selectedSurfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
				selectedSurfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
			} else {
				for (const auto& fmt : surfaceFormats) {
					if (fmt.format == VK_FORMAT_R8G8B8A8_UNORM && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
						selectedSurfaceFormat = fmt;
						break;
					}
				}

				if (selectedSurfaceFormat.format == VK_FORMAT_UNDEFINED) selectedSurfaceFormat = surfaceFormats[0];
			}
		}

		{
			VkPresentModeKHR selectedPresentMode;
			{
				uint32_t presentModeCount;
				if (vkGetPhysicalDeviceSurfacePresentModesKHR(_vkStatus.physicalDevice, _vkStatus.surface, &presentModeCount, nullptr) != VK_SUCCESS) return false;
				if (presentModeCount < 1) return false;
				std::vector<VkPresentModeKHR> presentModes(presentModeCount);
				if (vkGetPhysicalDeviceSurfacePresentModesKHR(_vkStatus.physicalDevice, _vkStatus.surface, &presentModeCount, presentModes.data()) != VK_SUCCESS) return false;

				uint32_t selectedPresentModeScore = 0;
				for (const auto& mode : presentModes) {
					uint32_t score = 0;
					switch (mode) {
					case VK_PRESENT_MODE_IMMEDIATE_KHR:
						score = 2;
						break;
					case VK_PRESENT_MODE_MAILBOX_KHR:
						score = 5;
						break;
					case VK_PRESENT_MODE_FIFO_KHR:
						score = 4;
						break;
					case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
						score = 3;
						break;
					default:
						score = 1;
						break;
					}

					if (selectedPresentModeScore < score) {
						selectedPresentModeScore = score;
						selectedPresentMode = mode;
					}
				}
			}

			VkExtent2D imageExtent;
			uint32_t imageCount;
			VkSurfaceTransformFlagBitsKHR preTransform;
			{
				VkSurfaceCapabilitiesKHR caps;
				if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_vkStatus.physicalDevice, _vkStatus.surface, &caps) != VK_SUCCESS) return false;
				if (caps.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
					imageExtent = caps.maxImageExtent;
				} else {
					imageExtent = caps.currentExtent;
				}

				imageCount = caps.minImageCount + 1;
				if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) imageCount = caps.maxImageCount;

				preTransform = caps.currentTransform;
			}

			auto oldSwapChain = _vkStatus.swapChain;

			VkSwapchainCreateInfoKHR swapchainCreateInfo;
			swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapchainCreateInfo.pNext = nullptr;
			swapchainCreateInfo.flags = 0;
			swapchainCreateInfo.surface = _vkStatus.surface;
			swapchainCreateInfo.minImageCount = imageCount;
			swapchainCreateInfo.imageFormat = selectedSurfaceFormat.format;
			swapchainCreateInfo.imageColorSpace = selectedSurfaceFormat.colorSpace;
			swapchainCreateInfo.imageExtent = imageExtent;
			swapchainCreateInfo.imageArrayLayers = 1;
			swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			uint32_t queueFamilyIndices[] = { _vkStatus.graphicsQueueFamilyIndex, _vkStatus.presentQueueFamilyIndex };
			if (_vkStatus.graphicsQueueFamilyIndex == _vkStatus.presentQueueFamilyIndex) {
				swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				swapchainCreateInfo.queueFamilyIndexCount = 0;
				swapchainCreateInfo.pQueueFamilyIndices = nullptr;
			} else {
				swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				swapchainCreateInfo.queueFamilyIndexCount = 2;
				swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
			}

			swapchainCreateInfo.preTransform = preTransform;
			swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			swapchainCreateInfo.presentMode = selectedPresentMode;
			swapchainCreateInfo.clipped = VK_TRUE;
			swapchainCreateInfo.oldSwapchain = oldSwapChain;

			if (vkCreateSwapchainKHR(_vkStatus.device, &swapchainCreateInfo, nullptr, &_vkStatus.swapChain) != VK_SUCCESS) return false;

			_cleanupSwapChain(&oldSwapChain);
		}

		{
			uint32_t swapChainImageCount;
			if (vkGetSwapchainImagesKHR(_vkStatus.device, _vkStatus.swapChain, &swapChainImageCount, nullptr) != VK_SUCCESS) return false;
			_vkStatus.swapChainImages.resize(swapChainImageCount);
			if (vkGetSwapchainImagesKHR(_vkStatus.device, _vkStatus.swapChain, &swapChainImageCount, _vkStatus.swapChainImages.data()) != VK_SUCCESS) return false;

			_vkStatus.swapChainImageViews.resize(_vkStatus.swapChainImages.size());
			VkImageViewCreateInfo imageViewCreateInfo;
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.pNext = nullptr;
			imageViewCreateInfo.flags = 0;
			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.format = selectedSurfaceFormat.format;
			imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			imageViewCreateInfo.subresourceRange.levelCount = 1;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount = 1;
			for (size_t i = 0; i < _vkStatus.swapChainImages.size(); ++i) {
				imageViewCreateInfo.image = _vkStatus.swapChainImages[i];
				
				if (vkCreateImageView(_vkStatus.device, &imageViewCreateInfo, nullptr, &_vkStatus.swapChainImageViews[i]) != VK_SUCCESS) _vkStatus.swapChainImageViews[i] = nullptr;
			}
		}

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
		return nullptr;
	}

	IntrusivePtr<IDepthStencil> Graphics::createDepthStencil() {
		return nullptr;
	}

	IntrusivePtr<IDepthStencilState> Graphics::createDepthStencilState() {
		return new DepthStencilState(*this, false);
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
		return _vkStatus.backSize;
	}

	void Graphics::setBackBufferSize(const Vec2ui32& size) {
		_resize(size);
	}

	Box2i32ui32 Graphics::getViewport() const {
		return _vkStatus.vp;
	}

	void Graphics::setViewport(const Box2i32ui32& vp) {
	}

	void Graphics::setBlendState(IBlendState* state, const Vec4f32& constantFactors, uint32_t sampleMask) {
	}

	void Graphics::setDepthStencilState(IDepthStencilState* state) {
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
		_cleanupSwapChain(nullptr);

		if (_vkStatus.device) {
			vkDeviceWaitIdle(_vkStatus.device);
			vkDestroyDevice(_vkStatus.device, nullptr);
			_vkStatus.device = nullptr;
		}

		if (_vkStatus.debug.destroyUtilsMessengerEXT) {
			_vkStatus.debug.destroyUtilsMessengerEXT(_vkStatus.instance, _vkStatus.debug.messenger, nullptr);
			_vkStatus.debug.destroyUtilsMessengerEXT = nullptr;
			_vkStatus.debug.messenger = nullptr;
		}

		if (_vkStatus.surface) {
			vkDestroySurfaceKHR(_vkStatus.instance, _vkStatus.surface, nullptr);
			_vkStatus.surface = nullptr;
		}

		if (_vkStatus.instance != nullptr) {
			vkDestroyInstance(_vkStatus.instance, nullptr);
			_vkStatus.instance = nullptr;
		}

		_vkStatus.enabledDeviceExtensions.count = 0;
		_vkStatus.physicalDevice = nullptr;

		_deviceFeatures.reset();

		_deviceVersion = "Vulkan Unknown";
	}

	void Graphics::_resize(const Vec2ui32& size) {
	}

	void Graphics::_cleanupSwapChain(VkSwapchainKHR* swapChain) {
		for (auto& i : _vkStatus.swapChainImageViews) {
			if (i) vkDestroyImageView(_vkStatus.device, i, nullptr);
		}
		_vkStatus.swapChainImageViews.clear();

		_vkStatus.swapChainImages.clear();

		if (!swapChain) swapChain = &_vkStatus.swapChain;
		if (*swapChain) {
			vkDestroySwapchainKHR(_vkStatus.device, *swapChain, nullptr);
			if (*swapChain == _vkStatus.swapChain) _vkStatus.swapChain = nullptr;
		}
	}

	VkCompareOp Graphics::convertCompareOp(ComparisonFunc func) {
		switch (func) {
		case ComparisonFunc::NEVER:
			return VK_COMPARE_OP_NEVER;
		case ComparisonFunc::LESS:
			return VK_COMPARE_OP_LESS;
		case ComparisonFunc::EQUAL:
			return VK_COMPARE_OP_EQUAL;
		case ComparisonFunc::LESS_EQUAL:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case ComparisonFunc::GREATER:
			return VK_COMPARE_OP_GREATER;
		case ComparisonFunc::NOT_EQUAL:
			return VK_COMPARE_OP_NOT_EQUAL;
		case ComparisonFunc::GREATER_EQUAL:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case ComparisonFunc::ALWAYS:
			return VK_COMPARE_OP_ALWAYS;
		default:
			return VK_COMPARE_OP_NEVER;
		}
	}

	VkStencilOp Graphics::convertStencilOp(StencilOp op) {
		switch (op) {
		case StencilOp::KEEP:
			return VK_STENCIL_OP_KEEP;
		case StencilOp::ZERO:
			return VK_STENCIL_OP_ZERO;
		case StencilOp::REPLACE:
			return VK_STENCIL_OP_REPLACE;
		case StencilOp::INCR_CLAMP:
			return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		case StencilOp::DECR_CLAMP:
			return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		case StencilOp::INCR_WRAP:
			return VK_STENCIL_OP_INCREMENT_AND_WRAP;
		case StencilOp::DECR_WRAP:
			return VK_STENCIL_OP_DECREMENT_AND_WRAP;
		case StencilOp::INVERT:
			return VK_STENCIL_OP_INVERT;
		default:
			return VK_STENCIL_OP_KEEP;
		}
	}

	VkBlendFactor Graphics::convertBlendFactor(BlendFactor factor) {
		switch (factor) {
		case BlendFactor::ZERO:
			return VK_BLEND_FACTOR_ZERO;
		case BlendFactor::ONE:
			return VK_BLEND_FACTOR_ONE;
		case BlendFactor::SRC_COLOR:
			return VK_BLEND_FACTOR_SRC_COLOR;
		case BlendFactor::ONE_MINUS_SRC_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case BlendFactor::SRC_ALPHA:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case BlendFactor::ONE_MINUS_SRC_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case BlendFactor::DST_COLOR:
			return VK_BLEND_FACTOR_DST_COLOR;
		case BlendFactor::ONE_MINUS_DST_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case BlendFactor::DST_ALPHA:
			return VK_BLEND_FACTOR_DST_ALPHA;
		case BlendFactor::ONE_MINUS_DST_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case BlendFactor::SRC_ALPHA_SATURATE:
			return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		case BlendFactor::CONSTANT_COLOR:
			return VK_BLEND_FACTOR_CONSTANT_COLOR;
		case BlendFactor::ONE_MINUS_CONSTANT_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		case BlendFactor::SRC1_COLOR:
			return VK_BLEND_FACTOR_SRC1_COLOR;
		case BlendFactor::ONE_MINUS_SRC1_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		case BlendFactor::SRC1_ALPHA:
			return VK_BLEND_FACTOR_SRC1_ALPHA;
		case BlendFactor::ONE_MINUS_SRC1_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
		default:
			return VK_BLEND_FACTOR_ZERO;
		}
	}

	VkBlendOp Graphics::convertBlendOp(BlendOp op) {
		switch (op) {
		case BlendOp::ADD:
			return VK_BLEND_OP_ADD;
		case BlendOp::SUBTRACT:
			return VK_BLEND_OP_SUBTRACT;
		case BlendOp::REV_SUBTRACT:
			return VK_BLEND_OP_REVERSE_SUBTRACT;
		case BlendOp::MIN:
			return VK_BLEND_OP_MIN;
		case BlendOp::MAX:
			return VK_BLEND_OP_MAX;
		default:
			return VK_BLEND_OP_ADD;
		}
	}
}