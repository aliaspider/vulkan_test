
#include <string.h>
#include "vulkan.h"

void uniform_buffer_init(const context_t *vk, uint32_t size, buffer_t* ubo)
{
   ubo->size = size;
   VkBufferCreateInfo uniformBufferCreateInfo =
   {
      VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, NULL, 0,
      ubo->size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_SHARING_MODE_EXCLUSIVE, 0, NULL
   };

   vkCreateBuffer(vk->device, &uniformBufferCreateInfo, NULL, &ubo->handle);
   buffer_memory_init(vk, ubo->handle, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &ubo->mem);
}

void vertex_buffer_init(const context_t *vk, uint32_t size, const void *data, buffer_t* vbo)
{
   vbo->size = size;
   VkBufferCreateInfo vertexBufferCreateInfo =
   {
      VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, NULL, 0,
      size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_SHARING_MODE_EXCLUSIVE, 0, NULL
   };

   vkCreateBuffer(vk->device, &vertexBufferCreateInfo, NULL, &vbo->handle);
   buffer_memory_init(vk, vbo->handle, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &vbo->mem);

   memcpy(vbo->mem.ptr, data, size);

   memory_flush(vk, &vbo->mem);

   vkUnmapMemory(vk->device, vbo->mem.handle);
   vbo->mem.ptr = NULL;
}

void buffer_free(const context_t *vk, buffer_t *buffer)
{
   memory_free(vk, &buffer->mem);
   vkDestroyBuffer(vk->device, buffer->handle, NULL);
   buffer->handle = VK_NULL_HANDLE;
}
