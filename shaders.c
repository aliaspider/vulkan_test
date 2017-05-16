
#include "vulkan.h"

void shaders_init(const context_t *vk, size_t vs_code_size, const uint32_t *vs_code, size_t fs_code_size, const uint32_t *fs_code, shaders_t* shaders)
{
   VkShaderModuleCreateInfo vertexShaderModuleCreateInfo =
   {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = vs_code_size,
      .pCode = vs_code
   };

   VkShaderModuleCreateInfo fragmentShaderModuleCreateInfo =
   {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = fs_code_size,
      .pCode = fs_code
   };

   vkCreateShaderModule(vk->device, &vertexShaderModuleCreateInfo, NULL, &shaders->vs);
   vkCreateShaderModule(vk->device, &fragmentShaderModuleCreateInfo, NULL, &shaders->fs);
}

void shaders_free(const context_t *vk, shaders_t *shaders)
{
   vkDestroyShaderModule(vk->device, shaders->vs, NULL);
   vkDestroyShaderModule(vk->device, shaders->fs, NULL);
   shaders->vs = VK_NULL_HANDLE;
   shaders->fs = VK_NULL_HANDLE;
}
