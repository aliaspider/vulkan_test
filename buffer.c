
#include <string.h>
#include "vulkan.h"

void uniform_buffer_init(VkDevice device, const uniform_buffer_init_info_t *init_info, buffer_t* ubo)
{
   ubo->size = init_info->size;

   {
      const VkBufferCreateInfo info =
      {
         VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, NULL, 0,
         ubo->size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
         VK_SHARING_MODE_EXCLUSIVE, 0, NULL
      };
      vkCreateBuffer(device, &info, NULL, &ubo->handle);
   }

   {
      memory_init_info_t info =
      {
         init_info->memory_types,
         .req_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
         .buffer = ubo->handle
      };
      memory_init(device, &info, &ubo->mem);
   }
}

void vertex_buffer_init(VkDevice device, const vertex_buffer_init_info_t *init_info, buffer_t *vbo)
{
   vbo->size = init_info->size;

   {
      const VkBufferCreateInfo info =
      {
         VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, NULL, 0,
         vbo->size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         VK_SHARING_MODE_EXCLUSIVE, 0, NULL
      };
      vkCreateBuffer(device, &info, NULL, &vbo->handle);
   }

   {
      memory_init_info_t info =
      {
         init_info->memory_types,
         .req_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
         .buffer = vbo->handle
      };
      memory_init(device, &info, &vbo->mem);
   }

   memcpy(vbo->mem.ptr, init_info->data, init_info->size);
   memory_flush(device, &vbo->mem);

   vkUnmapMemory(device, vbo->mem.handle);
   vbo->mem.ptr = NULL;
}

void buffer_free(const context_t *vk, buffer_t *buffer)
{
   memory_free(vk, &buffer->mem);
   vkDestroyBuffer(vk->device, buffer->handle, NULL);
   buffer->handle = VK_NULL_HANDLE;
}
