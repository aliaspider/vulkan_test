
#include <string.h>
#include "vulkan.h"

void uniform_buffer_init(const context_t *vk, uint32_t size, buffer_t* ubo)
{
   ubo->size = size;

   {
      const VkBufferCreateInfo info =
      {
         VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, NULL, 0,
         ubo->size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
         VK_SHARING_MODE_EXCLUSIVE, 0, NULL
      };
      vkCreateBuffer(vk->device, &info, NULL, &ubo->handle);
   }

   buffer_memory_init(vk->device, vk->mem.memoryTypes, ubo->handle, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &ubo->mem);
}

void vertex_buffer_init(VkDevice device, vertex_buffer_init_info_t* init_info, buffer_t *vbo)
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

   buffer_memory_init(device, init_info->memory_types, vbo->handle, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &vbo->mem);

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
