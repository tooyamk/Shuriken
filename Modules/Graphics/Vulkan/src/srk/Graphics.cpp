#define VMA_IMPLEMENTATION

#include "Graphics.h"
#include "CreateModule.h"
#include "BlendState.h"
#include "ConstantBuffer.h"
#include "DepthStencilState.h"
#include "IndexBuffer.h"
#include "Program.h"
#include "RasterizerState.h"
#include "Sampler.h"
#include "Texture1DResource.h"
#include "Texture2DResource.h"
#include "Texture3DResource.h"
#include "VertexBuffer.h"
#include "srk/StringUtility.h"
#include "srk/modules/graphics/GraphicsAdapter.h"

namespace srk::modules::graphics::vulkan {
	VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		using namespace std::string_view_literals;

		auto g = (Graphics*)pUserData;
		printaln(L"vk-debug : "sv, std::string_view(pCallbackData->pMessage));
		return VK_FALSE;
	}

	Graphics::Graphics() :
		_isDebug(false),
		_curIsBackBuffer(true),
		_backBufferSampleCount(1),
		_eventDispatcher(new events::EventDispatcher<GraphicsEvent>()) {
		_constantBufferManager.createShareConstantBufferCallback = std::bind(&Graphics::_createdShareConstantBuffer, this);
		_constantBufferManager.createExclusiveConstantBufferCallback = std::bind(&Graphics::_createdExclusiveConstantBuffer, this, std::placeholders::_1);
		memset(&_internalFeatures, 0, sizeof(InternalFeatures));

		memset(&_pipelineDynamicStateCreateInfo, 0, sizeof(_pipelineDynamicStateCreateInfo));
		_pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	}

	Graphics::~Graphics() {
		_release();
	}

	bool Graphics::createDevice(Ref* loader, const CreateGrahpicsModuleDescriptor& desc) {
		using namespace std::string_view_literals;

		if (_vkStatus.device) return false;

		if (desc.window) {
#if SRK_OS == SRK_OS_WINDOWS
			_hwnd = (HWND)desc.window->getNative("HWND"sv);
			if (!_hwnd) return false;
#endif
		} else {
			if (!desc.offscreen) return false;
		}

		if (desc.adapter) {
			if (desc.createProcessInfoHandler) desc.createProcessInfoHandler("specific adapter create device...");
			return _createDevice(loader, desc, desc.adapter);
		} else {
			if (desc.createProcessInfoHandler) desc.createProcessInfoHandler("search adapter create device...");

			std::vector<GraphicsAdapter> adapters;
			GraphicsAdapter::query(adapters);
			std::vector<uint32_t> indices;
			GraphicsAdapter::autoSort(adapters, indices);

			for (auto& idx : indices) {
				if (desc.createProcessInfoHandler) desc.createProcessInfoHandler("found adapter create device...");
				if (_createDevice(loader, desc, &adapters[idx])) return true;
			}

			if (desc.createProcessInfoHandler) desc.createProcessInfoHandler("search adapter create device failed");

			return false;
		}
	}

	bool Graphics::_createDevice(Ref* loader, const CreateGrahpicsModuleDescriptor& desc, const GraphicsAdapter* adapter) {
		using namespace srk::enum_operators;

		auto success = false;
		
		do {
			if (!_createVkInstance(desc.debug)) break;
			if (!_createVkSurface(*desc.window)) break;
			if (!_getVkPhysicalDevice(adapter)) break;
			if (!_createVkDevice()) break;
			if (!_createMemAllocator()) break;
			if (!_createVkCommandPool()) break;
			if (!_createVkSwapchain()) break;

			{
				_dynamicStates.clear();
				_dynamicStates.emplace_back(VK_DYNAMIC_STATE_VIEWPORT);
				_dynamicStates.emplace_back(VK_DYNAMIC_STATE_SCISSOR);
				_dynamicStates.emplace_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);

				_pipelineDynamicStateCreateInfo.dynamicStateCount = _dynamicStates.size();
				_pipelineDynamicStateCreateInfo.pDynamicStates = _dynamicStates.data();
			}

			{
				VkPipelineCacheCreateInfo pipelineCacheCreateInfo;
				memset(&pipelineCacheCreateInfo, 0, sizeof(pipelineCacheCreateInfo));
				pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
				pipelineCacheCreateInfo.initialDataSize = 0;
				pipelineCacheCreateInfo.pInitialData = nullptr;
				vkCreatePipelineCache(_vkStatus.device, &pipelineCacheCreateInfo, getVkAllocationCallbacks(), &_vkStatus.pipeline.cache);
			}

			{
				VkPhysicalDeviceProperties physicalDeviceProperties;
				vkGetPhysicalDeviceProperties(_vkStatus.physicalDevice, &physicalDeviceProperties);

				/*_deviceVersion = "Vulkan ";
				_deviceVersion += StringUtility::toString(VK_API_VERSION_MAJOR(physicalDeviceProperties.apiVersion));
				_deviceVersion += '.';
				_deviceVersion += StringUtility::toString(VK_VERSION_MINOR(physicalDeviceProperties.apiVersion));
				_deviceVersion += '.';
				_deviceVersion += StringUtility::toString(VK_VERSION_PATCH(physicalDeviceProperties.apiVersion));
				_deviceVersion += '.';
				_deviceVersion += StringUtility::toString(VK_API_VERSION_VARIANT(physicalDeviceProperties.apiVersion));*/

				_deviceFeatures.stencilIndependentMask = true;
				_deviceFeatures.stencilIndependentRef = true;
				_deviceFeatures.simultaneousRenderTargetCount = physicalDeviceProperties.limits.maxColorAttachments;
				_deviceFeatures.maxSamplerAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;
				_deviceFeatures.samplerAddressModes.emplace_back(SamplerAddressMode::REPEAT);
				_deviceFeatures.samplerAddressModes.emplace_back(SamplerAddressMode::CLAMP_EDGE);
				_deviceFeatures.samplerAddressModes.emplace_back(SamplerAddressMode::CLAMP_BORDER);
				_deviceFeatures.samplerAddressModes.emplace_back(SamplerAddressMode::MIRROR_REPEAT);
				_deviceFeatures.samplerAddressModes.emplace_back(SamplerAddressMode::MIRROR_CLAMP_EDGE);

				_vkStatus.usage.bufferCreateUsageMask = Usage::MAP_READ_WRITE | Usage::UPDATE | Usage::COPY_SRC_DST;
				_vkStatus.usage.texCreateUsageMask = Usage::MAP_READ_WRITE | Usage::UPDATE | Usage::COPY_SRC_DST | Usage::RENDERABLE;
			}

			_defaultBlendState = new BlendState(*this, true);
			_defaultDepthStencilState = new DepthStencilState(*this, true);
			_defaultRasterizerState = new RasterizerState(*this, true);

			setBlendState(nullptr);
			setDepthStencilState(nullptr);
			setRasterizerState(nullptr);

			success = true;
		} while (false);
		
		if (!success) _release();

		return success;
	}

	bool Graphics::_getVkPhysicalDevice(const GraphicsAdapter* adapter) {
		VkPhysicalDevice physicalDevice = nullptr;
		do {
			uint32_t physicalDeviceCount = 0;
			if (auto err = vkEnumeratePhysicalDevices(_vkStatus.instance, &physicalDeviceCount, nullptr); err != VK_SUCCESS || !physicalDeviceCount) break;

			std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
			if (vkEnumeratePhysicalDevices(_vkStatus.instance, &physicalDeviceCount, physicalDevices.data()) != VK_SUCCESS) break;

			VkPhysicalDeviceProperties physicalDeviceProperties;
			if (adapter) {
				for (auto& device : physicalDevices) {
					vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);
					if (physicalDeviceProperties.vendorID == adapter->vendorId && physicalDeviceProperties.deviceID == adapter->deviceId) {
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

			//see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDynamicState.html
			auto& data = _vkStatus.enabledDeviceExtensions;
			for (auto& extension : deviceExtensions) {
				if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, extension.extensionName)) {
					swapchainExtFound = true;
					data.names[data.count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
					continue;
				}
				if (!strcmp("VK_KHR_portability_subset", extension.extensionName)) {
					data.names[data.count++] = "VK_KHR_portability_subset";
					continue;
				}
				if (!strcmp("VK_EXT_custom_border_color", extension.extensionName)) {
					data.names[data.count++] = "VK_EXT_custom_border_color";
					_internalFeatures.customBorderColor = true;
					continue;
				}
				if (!strcmp("VK_EXT_extended_dynamic_state", extension.extensionName)) {
					data.names[data.count++] = "VK_EXT_extended_dynamic_state";
					_internalFeatures.extendedDynamicState = true;
					continue;
				}
				if (!strcmp("VK_EXT_extended_dynamic_state2", extension.extensionName)) {
					data.names[data.count++] = "VK_EXT_extended_dynamic_state2";
					_internalFeatures.extendedDynamicState2 = true;
					continue;
				}
				if (!strcmp("VK_EXT_extended_dynamic_state3", extension.extensionName)) {
					data.names[data.count++] = "VK_EXT_extended_dynamic_state3";
					_internalFeatures.extendedDynamicState3 = true;
					continue;
				}
				if (!strcmp("VK_EXT_vertex_input_dynamic_state", extension.extensionName)) {
					data.names[data.count++] = "VK_EXT_vertex_input_dynamic_state";
					_internalFeatures.vertexInputDynamicState = true;
					continue;
				}
				if (!strcmp("VK_EXT_color_write_enable", extension.extensionName)) {
					data.names[data.count++] = "VK_EXT_color_write_enable";
					_internalFeatures.colorWriteEnable = true;
					continue;
				}
			}
		} while (false);

		if (!swapchainExtFound) return false;

		_vkStatus.physicalDevice = physicalDevice;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &_vkStatus.physicalDeviceMemoryProperties);

		return true;
	}

	bool Graphics::_createVkInstance(bool debug) {
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

		VkApplicationInfo applicationInfo;
		memset(&applicationInfo, 0, sizeof(applicationInfo));
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pApplicationName = "";
		applicationInfo.applicationVersion = 0;
		applicationInfo.pEngineName = "";
		applicationInfo.engineVersion = 0;
		applicationInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo instanceCreateInfo;
		memset(&instanceCreateInfo, 0, sizeof(instanceCreateInfo));
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pApplicationInfo = &applicationInfo;
		instanceCreateInfo.enabledLayerCount = enabledLayerCount;
		instanceCreateInfo.ppEnabledLayerNames = enabledLayerNames;
		instanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;
		instanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames;

		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo;
		if (debug) {
			memset(&debugMessengerCreateInfo, 0, sizeof(debugMessengerCreateInfo));
			debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugMessengerCreateInfo.pfnUserCallback = debugMessengerCallback;
			debugMessengerCreateInfo.pUserData = this;
			instanceCreateInfo.pNext = &debugMessengerCreateInfo;
		}

		if (vkCreateInstance(&instanceCreateInfo, nullptr, &_vkStatus.instance) != VK_SUCCESS) return false;

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

		_vkStatus.apiVersion = VK_API_VERSION_1_0;
		auto enumerateInstanceVersion = PFN_vkEnumerateInstanceVersion(vkGetInstanceProcAddr(_vkStatus.instance, "vkEnumerateInstanceVersion"));
		if (enumerateInstanceVersion) enumerateInstanceVersion(&_vkStatus.apiVersion);

		_deviceVersion = "Vulkan ";
		_deviceVersion += StringUtility::toString(VK_API_VERSION_MAJOR(_vkStatus.apiVersion));
		_deviceVersion += '.';
		_deviceVersion += StringUtility::toString(VK_VERSION_MINOR(_vkStatus.apiVersion));
		_deviceVersion += '.';
		_deviceVersion += StringUtility::toString(VK_VERSION_PATCH(_vkStatus.apiVersion));
		_deviceVersion += '.';
		_deviceVersion += StringUtility::toString(VK_API_VERSION_VARIANT(_vkStatus.apiVersion));

		return true;
	}

	bool Graphics::_createVkSurface(windows::IWindow& win) {
		auto err = VK_ERROR_UNKNOWN;

#if SRK_OS == SRK_OS_WINDOWS
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
		memset(&surfaceCreateInfo, 0, sizeof(surfaceCreateInfo));
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.hinstance = GetModuleHandleW(nullptr);
		surfaceCreateInfo.hwnd = _hwnd;

		err = vkCreateWin32SurfaceKHR(_vkStatus.instance, &surfaceCreateInfo, nullptr, &_vkStatus.surface);
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

		_vkStatus.queueFamilyIndices.graphics = graphicsQueueFamilyIndex;
		_vkStatus.queueFamilyIndices.present = presentQueueFamilyIndex;

		float32_t queuePriorities[1] = { 0.0f };

		VkDeviceQueueCreateInfo deviceQueueCreateInfos[2];
		auto& queue = deviceQueueCreateInfos[0];
		memset(&queue, 0, sizeof(queue));
		queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue.queueFamilyIndex = graphicsQueueFamilyIndex;
		queue.queueCount = 1;
		queue.pQueuePriorities = queuePriorities;

		VkPhysicalDeviceFeatures physicalDeviceFeatures;
		memset(&physicalDeviceFeatures, 0, sizeof(physicalDeviceFeatures));
		physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo deviceCreateInfo;
		memset(&deviceCreateInfo, 0, sizeof(deviceCreateInfo));
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos;
		deviceCreateInfo.enabledLayerCount = 0;
		deviceCreateInfo.ppEnabledLayerNames = nullptr;
		deviceCreateInfo.enabledExtensionCount = _vkStatus.enabledDeviceExtensions.count;
		deviceCreateInfo.ppEnabledExtensionNames = (const char* const*)_vkStatus.enabledDeviceExtensions.names;
		deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

		if (graphicsQueueFamilyIndex != presentQueueFamilyIndex) {
			queue = deviceQueueCreateInfos[1];
			memset(&queue, 0, sizeof(queue));
			queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue.queueFamilyIndex = presentQueueFamilyIndex;
			queue.queueCount = 1;
			queue.pQueuePriorities = queuePriorities;
			deviceCreateInfo.queueCreateInfoCount = 2;
		}

		return vkCreateDevice(_vkStatus.physicalDevice, &deviceCreateInfo, nullptr, &_vkStatus.device) == VK_SUCCESS;
	}

	bool Graphics::_createMemAllocator() {
		VmaAllocatorCreateInfo allocatorCreateInfo;
		memset(&allocatorCreateInfo, 0, sizeof(allocatorCreateInfo));
		allocatorCreateInfo.vulkanApiVersion = VK_MAKE_VERSION(1, 1, 0);//_vkStatus.apiVersion;

		allocatorCreateInfo.physicalDevice = _vkStatus.physicalDevice;
		allocatorCreateInfo.device = _vkStatus.device;
		allocatorCreateInfo.instance = _vkStatus.instance;

		return vmaCreateAllocator(&allocatorCreateInfo, &_vkStatus.memAllocator) == VK_SUCCESS;
	}

	bool Graphics::_createVkCommandPool() {
		VkCommandPoolCreateInfo commandPoolCreateInfo;
		memset(&commandPoolCreateInfo, 0, sizeof(commandPoolCreateInfo));
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCreateInfo.queueFamilyIndex = _vkStatus.queueFamilyIndices.graphics;

		if (vkCreateCommandPool(_vkStatus.device, &commandPoolCreateInfo, getVkAllocationCallbacks(), &_vkStatus.cmd.pool) != VK_SUCCESS) return false;

		vkGetDeviceQueue(_vkStatus.device, _vkStatus.queueFamilyIndices.graphics, 0, &_vkStatus.cmd.graphicsQueue);

		return true;
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

			auto oldSwapChain = _vkStatus.swapChain.swapChain;

			VkSwapchainCreateInfoKHR swapchainCreateInfo;
			memset(&swapchainCreateInfo, 0, sizeof(swapchainCreateInfo));
			swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapchainCreateInfo.surface = _vkStatus.surface;
			swapchainCreateInfo.minImageCount = imageCount;
			swapchainCreateInfo.imageFormat = selectedSurfaceFormat.format;
			swapchainCreateInfo.imageColorSpace = selectedSurfaceFormat.colorSpace;
			swapchainCreateInfo.imageExtent = imageExtent;
			swapchainCreateInfo.imageArrayLayers = 1;
			swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			uint32_t queueFamilyIndices[] = { _vkStatus.queueFamilyIndices.graphics, _vkStatus.queueFamilyIndices.present };
			if (_vkStatus.queueFamilyIndices.graphics == _vkStatus.queueFamilyIndices.present) {
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

			if (vkCreateSwapchainKHR(_vkStatus.device, &swapchainCreateInfo, getVkAllocationCallbacks(), &_vkStatus.swapChain.swapChain) != VK_SUCCESS) return false;
			_vkStatus.swapChain.format = selectedSurfaceFormat.format;

			_cleanupSwapChain(&oldSwapChain);
		}

		{
			uint32_t swapChainImageCount;
			if (vkGetSwapchainImagesKHR(_vkStatus.device, _vkStatus.swapChain.swapChain, &swapChainImageCount, nullptr) != VK_SUCCESS) return false;
			_vkStatus.swapChain.images.resize(swapChainImageCount);
			if (vkGetSwapchainImagesKHR(_vkStatus.device, _vkStatus.swapChain.swapChain, &swapChainImageCount, _vkStatus.swapChain.images.data()) != VK_SUCCESS) return false;

			_vkStatus.swapChain.imageViews.resize(_vkStatus.swapChain.images.size());
			VkImageViewCreateInfo imageViewCreateInfo;
			memset(&imageViewCreateInfo, 0, sizeof(imageViewCreateInfo));
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
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
			for (size_t i = 0; i < _vkStatus.swapChain.images.size(); ++i) {
				imageViewCreateInfo.image = _vkStatus.swapChain.images[i];
				
				if (vkCreateImageView(_vkStatus.device, &imageViewCreateInfo, getVkAllocationCallbacks(), &_vkStatus.swapChain.imageViews[i]) != VK_SUCCESS) _vkStatus.swapChain.imageViews[i] = nullptr;
			}
		}

		return true;
	}

	bool Graphics::_checkAndUpdateVkPipeline(IProgram* program, const IVertexAttributeGetter* vertexAttributeGetter, const IShaderParameterGetter* shaderParamGetter) {
		using namespace std::string_view_literals;

		if (!program || program->getGraphics() != this) return false;
		auto p = (Program*)program->getNative();;
		if (!p) return false;

		auto needUpdate = false;
		if (_vkStatus.pipeline.pipeline) {
			if (_vkStatus.pipeline.rasterizerFeatureValue != _vkStatus.rasterizer.featureValue) {
				needUpdate = true;
			}
		} else {
			needUpdate = true;
		}

		if (!needUpdate) return true;

		uint32_t vertexInputAttributeDescriptionCount = p->getInfo().vertices.size();
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions(vertexInputAttributeDescriptionCount);
		
		uint32_t vertexInputBindingDescriptionCount = vertexInputAttributeDescriptionCount;
		std::vector<VkVertexInputBindingDescription> vertexInputBindingDescription(vertexInputBindingDescriptionCount);
		
		auto rst = p->use(vertexAttributeGetter, 
			vertexInputBindingDescription.data(), vertexInputBindingDescriptionCount,
			vertexInputAttributeDescriptions.data(), vertexInputAttributeDescriptionCount,
			shaderParamGetter);
		if (!rst) return false;

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo;
		memset(&graphicsPipelineCreateInfo, 0, sizeof(graphicsPipelineCreateInfo));
		graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

		auto& pipelineShaderStageCreateInfos = p->getVkPipelineShaderStageCreateInfos();
		graphicsPipelineCreateInfo.stageCount = pipelineShaderStageCreateInfos.size();
		graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos.data();

		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo;
		memset(&pipelineVertexInputStateCreateInfo, 0, sizeof(pipelineVertexInputStateCreateInfo));
		pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = vertexInputBindingDescriptionCount;
		pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindingDescription.data();
		pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = vertexInputAttributeDescriptionCount;
		pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();
		graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;

		VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo;
		memset(&pipelineInputAssemblyStateCreateInfo, 0, sizeof(pipelineInputAssemblyStateCreateInfo));
		pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_TRUE;
		graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;

		graphicsPipelineCreateInfo.pTessellationState = nullptr;

		/*VkViewport viewport;
		viewport.x = _vkStatus.viewport.pos[0];
		viewport.y = _vkStatus.viewport.pos[1];
		viewport.width = _vkStatus.viewport.size[0];
		viewport.height = _vkStatus.viewport.size[1];
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor;
		scissor.offset = { _vkStatus.scissor.pos[0], _vkStatus.scissor.pos[1] };
		scissor.extent = { _vkStatus.scissor.size[0], _vkStatus.scissor.size[1] };

		VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo;
		memset(&pipelineViewportStateCreateInfo, 0, sizeof(pipelineViewportStateCreateInfo));
		pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		pipelineViewportStateCreateInfo.viewportCount = 1;
		pipelineViewportStateCreateInfo.pViewports = &viewport;
		pipelineViewportStateCreateInfo.scissorCount = 1;
		pipelineViewportStateCreateInfo.pScissors = &scissor;
		graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;*/
		graphicsPipelineCreateInfo.pViewportState = nullptr;

		graphicsPipelineCreateInfo.pRasterizationState = &_vkStatus.rasterizer.info;

		VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo;
		memset(&pipelineMultisampleStateCreateInfo, 0, sizeof(pipelineMultisampleStateCreateInfo));
		pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		pipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
		pipelineMultisampleStateCreateInfo.minSampleShading = 1.0f; // Optional
		pipelineMultisampleStateCreateInfo.pSampleMask = nullptr; // Optional
		pipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE; // Optional
		pipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE; // Optional
		graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;

		graphicsPipelineCreateInfo.pDepthStencilState = &_vkStatus.depthStencil.info;
		graphicsPipelineCreateInfo.pColorBlendState = &_vkStatus.blend.info;
		graphicsPipelineCreateInfo.pDynamicState = &_pipelineDynamicStateCreateInfo;

		VkAttachmentDescription attachmentDesc;
		memset(&attachmentDesc, 0, sizeof(attachmentDesc));
		attachmentDesc.format = _vkStatus.swapChain.format;
		attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		
		VkAttachmentReference attachmentRef;
		attachmentRef.attachment = 0;
		attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDesc;
		memset(&subpassDesc, 0, sizeof(subpassDesc));
		subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDesc.colorAttachmentCount = 1;
		subpassDesc.pColorAttachments = &attachmentRef;

		VkRenderPassCreateInfo renderPassCreateInfo;
		memset(&renderPassCreateInfo, 0, sizeof(renderPassCreateInfo));
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &attachmentDesc;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDesc;

		VkRenderPass renderPass;
		if (vkCreateRenderPass(_vkStatus.device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
			printaln(L"failed to create render pass!"sv);
		}

		graphicsPipelineCreateInfo.layout = p->getVkPipelineLayout();
		graphicsPipelineCreateInfo.renderPass = renderPass;
		graphicsPipelineCreateInfo.subpass = 0;
		graphicsPipelineCreateInfo.basePipelineHandle = nullptr;
		graphicsPipelineCreateInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(_vkStatus.device, _vkStatus.pipeline.cache, 1, &graphicsPipelineCreateInfo, getVkAllocationCallbacks(), &_vkStatus.pipeline.pipeline) != VK_SUCCESS) {
			_vkStatus.pipeline.pipeline = nullptr;
			return false;
		}
		
		return true;
	}

	IntrusivePtr<events::IEventDispatcher<GraphicsEvent>> Graphics::getEventDispatcher() {
		return _eventDispatcher;
	}

	//const events::IEventDispatcher<GraphicsEvent>& Graphics::getEventDispatcher() const {
	//	return _eventDispatcher;
	//}

	std::string_view Graphics::getVersion() const {
		return _deviceVersion;
	}

	const GraphicsDeviceFeatures& Graphics::getDeviceFeatures() const {
		return _deviceFeatures;
	}

	IntrusivePtr<IBlendState> Graphics::createBlendState() {
		return new BlendState(*this, false);
	}

	IntrusivePtr<IConstantBuffer> Graphics::createConstantBuffer() {
		return new ConstantBuffer(*this);
	}

	IntrusivePtr<IDepthStencil> Graphics::createDepthStencil() {
		return nullptr;
	}

	IntrusivePtr<IDepthStencilState> Graphics::createDepthStencilState() {
		return new DepthStencilState(*this, false);
	}

	IntrusivePtr<IIndexBuffer> Graphics::createIndexBuffer() {
		return new IndexBuffer(*this);
	}

	IntrusivePtr<IProgram> Graphics::createProgram() {
		return new Program(*this);
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
		return new Sampler(*this);
	}

	IntrusivePtr<ITexture1DResource> Graphics::createTexture1DResource() {
		return new Texture1DResource(*this);
	}

	IntrusivePtr<ITexture2DResource> Graphics::createTexture2DResource() {
		return new Texture2DResource(*this);
	}

	IntrusivePtr<ITexture3DResource> Graphics::createTexture3DResource() {
		return new Texture3DResource(*this);
	}

	IntrusivePtr<ITextureView> Graphics::createTextureView() {
		return nullptr;
	}

	IntrusivePtr<IVertexBuffer> Graphics::createVertexBuffer() {
		return new VertexBuffer(*this);
	}

	const Vec2ui32& Graphics::getBackBufferSize() const {
		return _vkStatus.backSize;
	}

	void Graphics::setBackBufferSize(const Vec2ui32& size) {
		_resize(size);
	}

	Box2i32ui32 Graphics::getViewport() const {
		return _vkStatus.viewport;
	}

	void Graphics::setViewport(const Box2i32ui32& vp) {
	}

	Box2i32ui32 Graphics::getScissor() const {
		return _vkStatus.scissor;
	}

	void Graphics::setScissor(const Box2i32ui32& scissor) {
	}

	void Graphics::setBlendState(IBlendState* state, uint32_t sampleMask) {
		if (state && state->getGraphics() == this) {
			if (auto native = state->getNative(); native) {
				_setBlendState(*(BlendState*)native, sampleMask);
			} else {
				_setBlendState(*(BlendState*)_defaultBlendState->getNative(), sampleMask);
			}
		} else if (_defaultBlendState) {
			_setBlendState(*(BlendState*)_defaultBlendState->getNative(), sampleMask);
		}
	}

	void Graphics::_setBlendState(BlendState& state, uint32_t sampleMask) {
		state.update();
		auto& blend = _vkStatus.blend;
		if (blend.featureValue != state.getFeatureValue()) {
			blend.featureValue = state.getFeatureValue();
			blend.info = state.getInternalState();
		}
	}

	void Graphics::setDepthStencilState(IDepthStencilState* state) {
		if (state && state->getGraphics() == this) {
			if (auto native = state->getNative(); native) {
				_setDepthStencilState(*(DepthStencilState*)native);
			} else {
				_setDepthStencilState(*(DepthStencilState*)_defaultDepthStencilState->getNative());
			}
		} else if (_defaultBlendState) {
			_setDepthStencilState(*(DepthStencilState*)_defaultDepthStencilState->getNative());
		}
	}

	void Graphics::_setDepthStencilState(DepthStencilState& state) {
		state.update();
		auto& depthStencil = _vkStatus.depthStencil;
		if (depthStencil.featureValue.trySet(state.getFeatureValue())) depthStencil.info = state.getInternalState();
	}

	void Graphics::setRasterizerState(IRasterizerState* state) {
		if (state && state->getGraphics() == this) {
			if (auto native = state->getNative(); native) {
				_setRasterizerState(*(RasterizerState*)native);
			} else {
				_setRasterizerState(*(RasterizerState*)_defaultRasterizerState->getNative());
			}
		} else if (_defaultRasterizerState) {
			_setRasterizerState(*(RasterizerState*)_defaultRasterizerState->getNative());
		}
	}

	void Graphics::_setRasterizerState(RasterizerState& state) {
		state.update();
		auto& rasterizer = _vkStatus.rasterizer;
		if (rasterizer.featureValue.trySet(state.getFeatureValue())) rasterizer.info = state.getInternalState();
	}

	void Graphics::beginRender() {
	}

	void Graphics::draw(IProgram* program, const IVertexAttributeGetter* vertexAttributeGetter, const IShaderParameterGetter* shaderParamGetter,
		const IIndexBuffer* indexBuffer, uint32_t count, uint32_t offset) {
		if (!_checkAndUpdateVkPipeline(program, vertexAttributeGetter, shaderParamGetter)) return;
	}

	void Graphics::drawInstanced(IProgram* program, const IVertexAttributeGetter* vertexAttributeGetter, const IShaderParameterGetter* shaderParamGetter,
		const IIndexBuffer* indexBuffer, uint32_t instancedCount, uint32_t count, uint32_t offset) {
		if (!_checkAndUpdateVkPipeline(program, vertexAttributeGetter, shaderParamGetter)) return;
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

	VkFence Graphics::getFenceFromPool() {
		VkFence fence = nullptr;

		_fencesLock.lock();

		if (_fencesPool.empty()) {
			_fencesLock.unlock();

			VkFenceCreateInfo info;
			memset(&info, 0, sizeof(info));
			info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			if (vkCreateFence(_vkStatus.device, &info, getVkAllocationCallbacks(), &fence) != VK_SUCCESS) return nullptr;
		} else {
			fence = _fencesPool[_fencesPool.size() - 1];
			_fencesPool.pop_back();

			_fencesLock.unlock();

			vkResetFences(_vkStatus.device, 1, &fence);
		}

		return fence;
	}

	InternalCommandBuffer Graphics::beginOneTimeCommands() {
		VkCommandBufferAllocateInfo commandBufferAllocateInfo;
		memset(&commandBufferAllocateInfo, 0, sizeof(commandBufferAllocateInfo));
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandPool = _vkStatus.cmd.pool;
		commandBufferAllocateInfo.commandBufferCount = 1;

		InternalCommandBuffer buffer;
		if (!buffer.create(_vkStatus.device, _vkStatus.cmd.pool, commandBufferAllocateInfo)) return InternalCommandBuffer();

		VkCommandBufferBeginInfo commandBufferBeginInfo;
		memset(&commandBufferBeginInfo, 0, sizeof(commandBufferBeginInfo));
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		if (vkBeginCommandBuffer(buffer.getVkCommandBuffer(), &commandBufferBeginInfo) != VK_SUCCESS) return InternalCommandBuffer();

		return std::move(buffer);
	}

	bool Graphics::endOneTimeCommands(InternalCommandBuffer& buffer) {
		auto cb = buffer.getVkCommandBuffer();
		if (vkEndCommandBuffer(cb) != VK_SUCCESS) return false;

		auto fence = getFenceFromPool();
		if (!fence) return false;

		std::unique_ptr<VkFence_T, FenceDeleter> pf(fence, FenceDeleter(this));

		VkSubmitInfo submitInfo;
		memset(&submitInfo, 0, sizeof(submitInfo));
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cb;

		auto queue = _vkStatus.cmd.graphicsQueue;

		//if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) return false;
		if (vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS) return false;

		do {
			auto rst = vkGetFenceStatus(_vkStatus.device, fence);
			if (rst == VK_NOT_READY) {
				std::this_thread::yield();
				continue;
			}

			return rst == VK_SUCCESS;
		} while (true);
		//if (vkQueueWaitIdle(queue) != VK_SUCCESS) return false;
	}

	void Graphics::transitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageSubresourceRange& range) {
		if (oldLayout == newLayout) return;

		VkImageMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange = range;

		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		} else {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	int32_t Graphics::findProperties(uint32_t memoryTypeBits, VkMemoryPropertyFlags flags) {
		for (uint32_t i = 0; i < _vkStatus.physicalDeviceMemoryProperties.memoryTypeCount; ++i) {
			if ((memoryTypeBits & (1 << i)) && (_vkStatus.physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & flags) == flags) return i;
		}

		return -1;
	}

	void Graphics::_release() {
		if (_vkStatus.device) vkDeviceWaitIdle(_vkStatus.device);

		auto allocator = getVkAllocationCallbacks();

		for (auto f : _fencesPool) {
			vkDestroyFence(_vkStatus.device, f, allocator);
		}
		_fencesPool.clear();

		if (_vkStatus.cmd.pool) {
			vkDestroyCommandPool(_vkStatus.device, _vkStatus.cmd.pool, allocator);
			_vkStatus.cmd.pool = nullptr;
		}

		if (_vkStatus.pipeline.cache) {
			vkDestroyPipelineCache(_vkStatus.device, _vkStatus.pipeline.cache, allocator);
			_vkStatus.pipeline.cache = nullptr;
		}

		_cleanupSwapChain(nullptr);

		if (_vkStatus.memAllocator) {
			vmaDestroyAllocator(_vkStatus.memAllocator);
			_vkStatus.memAllocator = nullptr;
		}

		if (_vkStatus.device) {
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
		_vkStatus.cmd.graphicsQueue = nullptr;

		memset(&_internalFeatures, 0, sizeof(_internalFeatures));
		_deviceFeatures.reset();

		_vkStatus.apiVersion = 0;
		_deviceVersion = "Vulkan Unknown";
		_vkStatus.usage.bufferCreateUsageMask = Usage::NONE;
		_vkStatus.usage.texCreateUsageMask = Usage::NONE;
	}

	void Graphics::_resize(const Vec2ui32& size) {
	}

	void Graphics::_cleanupSwapChain(VkSwapchainKHR* swapChain) {
		vkDeviceWaitIdle(_vkStatus.device);

		for (auto& i : _vkStatus.swapChain.imageViews) {
			if (i) vkDestroyImageView(_vkStatus.device, i, getVkAllocationCallbacks());
		}
		_vkStatus.swapChain.imageViews.clear();

		_vkStatus.swapChain.images.clear();

		if (!swapChain) swapChain = &_vkStatus.swapChain.swapChain;
		if (*swapChain) {
			vkDestroySwapchainKHR(_vkStatus.device, *swapChain, getVkAllocationCallbacks());
			if (*swapChain == _vkStatus.swapChain.swapChain) _vkStatus.swapChain.swapChain = nullptr;
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

	VkPolygonMode Graphics::convertFillMode(FillMode mode) {
		switch (mode) {
		case FillMode::WIREFRAME:
			return VK_POLYGON_MODE_LINE;
		case FillMode::SOLID:
			return VK_POLYGON_MODE_FILL;
		default:
			return VK_POLYGON_MODE_FILL;
		}
	}

	VkCullModeFlagBits Graphics::convertCullMode(CullMode mode) {
		switch (mode) {
		case CullMode::NONE:
			return VK_CULL_MODE_NONE;
		case CullMode::FRONT:
			return VK_CULL_MODE_FRONT_BIT;
		case CullMode::BACK:
			return VK_CULL_MODE_BACK_BIT;
		default:
			return VK_CULL_MODE_NONE;
		}
	}

	VkFrontFace Graphics::convertFrontFace(FrontFace mode) {
		return mode == FrontFace::CW ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}

	VkShaderStageFlagBits Graphics::convertProgramStage(ProgramStage stage) {
		switch (stage) {
		case ProgramStage::CS:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		case ProgramStage::GS:
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		case ProgramStage::PS:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ProgramStage::VS:
			return VK_SHADER_STAGE_VERTEX_BIT;
		default:
			return (VkShaderStageFlagBits)0;
		}
	}

	VkFormat Graphics::convertVertexFormat(VertexFormat fmt) {
		switch (fmt.type) {
		case VertexType::I32:
		{
			switch (fmt.dimension) {
			case 1:
				return VK_FORMAT_R32_SINT;
			case 2:
				return VK_FORMAT_R32G32_SINT;
			case 3:
				return VK_FORMAT_R32G32B32_SINT;
			case 4:
				return VK_FORMAT_R32G32B32A32_SINT;
			default:
				return VK_FORMAT_UNDEFINED;
			}
		}
		case VertexType::UI32:
		{
			switch (fmt.dimension) {
			case 1:
				return VK_FORMAT_R32_UINT;
			case 2:
				return VK_FORMAT_R32G32_UINT;
			case 3:
				return VK_FORMAT_R32G32B32_UINT;
			case 4:
				return VK_FORMAT_R32G32B32A32_UINT;
			default:
				return VK_FORMAT_UNDEFINED;
			}
		}
		case VertexType::F32:
		{
			switch (fmt.dimension) {
			case 1:
				return VK_FORMAT_R32_SFLOAT;
			case 2:
				return VK_FORMAT_R32G32_SFLOAT;
			case 3:
				return VK_FORMAT_R32G32B32_SFLOAT;
			case 4:
				return VK_FORMAT_R32G32B32A32_SFLOAT;
			default:
				return VK_FORMAT_UNDEFINED;
			}
		}
		default:
			return VK_FORMAT_UNDEFINED;
		}
	}

	VkImageType Graphics::convertTextureType(TextureType type) {
		switch (type) {
		case TextureType::TEX1D:
			return VK_IMAGE_TYPE_1D;
		case TextureType::TEX2D:
			return VK_IMAGE_TYPE_2D;
		case TextureType::TEX3D:
			return VK_IMAGE_TYPE_3D;
		default:
			return VK_IMAGE_TYPE_2D;
		}
	}

	VkFormat Graphics::convertTextureFormat(TextureFormat fmt) {
		switch (fmt) {
		case TextureFormat::R8G8B8_UNORM:
			return VK_FORMAT_R8G8B8_UNORM;
		case TextureFormat::R8G8B8_UNORM_SRGB:
			return VK_FORMAT_R8G8B8_SRGB;
		case TextureFormat::R8G8B8_UINT:
			return VK_FORMAT_R8G8B8_UINT;
		case TextureFormat::R8G8B8_SNORM:
			return VK_FORMAT_R8G8B8_SNORM;
		case TextureFormat::R8G8B8_SINT:
			return VK_FORMAT_R8G8B8_SINT;
		case TextureFormat::R8G8B8A8_UNORM:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::R8G8B8A8_UNORM_SRGB:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case TextureFormat::R8G8B8A8_UINT:
			return VK_FORMAT_R8G8B8A8_UINT;
		case TextureFormat::R8G8B8A8_SNORM:
			return VK_FORMAT_R8G8B8A8_SNORM;
		case TextureFormat::R8G8B8A8_SINT:
			return VK_FORMAT_R8G8B8A8_SINT;
		default:
			return VK_FORMAT_UNDEFINED;
		}
	}

	VkSampleCountFlagBits Graphics::convertSampleCount(SampleCount sc) {
		switch (sc) {
		case 1:
			return VK_SAMPLE_COUNT_1_BIT;
		case 2:
			return VK_SAMPLE_COUNT_2_BIT;
		case 4:
			return VK_SAMPLE_COUNT_4_BIT;
		case 8:
			return VK_SAMPLE_COUNT_8_BIT;
		case 16:
			return VK_SAMPLE_COUNT_16_BIT;
		case 32:
			return VK_SAMPLE_COUNT_32_BIT;
		case 64:
			return VK_SAMPLE_COUNT_64_BIT;
		default:
			return (VkSampleCountFlagBits)0;
		}
	}

	VkSamplerAddressMode Graphics::convertSamplerAddressMode(SamplerAddressMode mode) {
		switch (mode) {
		case SamplerAddressMode::REPEAT:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case SamplerAddressMode::CLAMP_EDGE:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case SamplerAddressMode::CLAMP_BORDER:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case SamplerAddressMode::MIRROR_REPEAT:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case SamplerAddressMode::MIRROR_CLAMP_EDGE:
			return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		default:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}
	}

	const VkAllocationCallbacks* Graphics::getVkAllocationCallbacks() const {
		return _vkStatus.memAllocator->GetAllocationCallbacks();
	}

	IConstantBuffer* Graphics::_createdShareConstantBuffer() {
		return new ConstantBuffer(*this);
	}

	IConstantBuffer* Graphics::_createdExclusiveConstantBuffer(uint32_t numParameters) {
		auto cb = new ConstantBuffer(*this);
		cb->recordUpdateIds = new uint32_t[numParameters];
		memset(cb->recordUpdateIds, 0, sizeof(uint32_t) * numParameters);
		return cb;
	}
}