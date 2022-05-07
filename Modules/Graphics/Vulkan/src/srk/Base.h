#pragma once

#include "srk/modules/graphics/IGraphicsModule.h"
#include "srk/IApplication.h"
#include "vulkan/vulkan.h"

#if SRK_OS == SRK_OS_WINDOWS
#	include "vulkan/vulkan_win32.h"
#endif

namespace srk::modules::graphics::vulkan {
}