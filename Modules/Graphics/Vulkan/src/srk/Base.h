#pragma once

#include "srk/modules/graphics/IGraphicsModule.h"
#include "srk/IApplication.h"
#include "vulkan/vulkan.h"

#if SRK_OS == SRK_OS_WINDOWS
#	include "vulkan/vulkan_win32.h"
#elif SRK_OS == SRK_OS_LINUX
#	include <X11/Xutil.h>
#	include "vulkan/vulkan_xlib.h"
#endif

namespace srk::modules::graphics::vulkan {
}
