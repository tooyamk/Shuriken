#pragma once

#include "aurora/modules/graphics/IGraphicsModule.h"
#include "aurora/IApplication.h"
#include "vulkan/vulkan.h"

#if AE_OS == AE_OS_WINDOWS
#	include "vulkan/vulkan_win32.h"
#endif

namespace aurora::modules::graphics::vulkan {
}