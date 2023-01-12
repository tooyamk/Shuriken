execute_process(COMMAND git -C ${SRC_DIR} clean -xfd)
execute_process(COMMAND git -C ${SRC_DIR} reset --hard)

set(file ${SRC_DIR}/CMakeLists.txt)
file(READ ${file} content)
string(REPLACE "find_package(Vulkan REQUIRED)" "" content "${content}")
string(REPLACE "include_directories(\${Vulkan_INCLUDE_DIR})" "" content "${content}")
file(WRITE ${file} "${content}")

set(file ${SRC_DIR}/include/vk_mem_alloc.h)
file(READ ${file} content)
string(REPLACE "(PFN_vkGetBufferMemoryRequirements2)vkGetBufferMemoryRequirements2;" "(PFN_vkGetBufferMemoryRequirements2)vkGetBufferMemoryRequirements2;\r\n        m_VulkanFunctions.vkGetBufferMemoryRequirements2KHR = [](VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {vkGetBufferMemoryRequirements(device, pInfo->buffer, &pMemoryRequirements->memoryRequirements); };" content "${content}")
string(REPLACE "(PFN_vkGetImageMemoryRequirements2)vkGetImageMemoryRequirements2;" "(PFN_vkGetImageMemoryRequirements2)vkGetImageMemoryRequirements2;\r\n        m_VulkanFunctions.vkGetImageMemoryRequirements2KHR = [](VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {vkGetImageMemoryRequirements(device, pInfo->image, &pMemoryRequirements->memoryRequirements); };" content "${content}")
file(WRITE ${file} "${content}")