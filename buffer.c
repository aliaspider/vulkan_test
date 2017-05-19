
#include <string.h>
#include "vulkan.h"

void buffer_init(VkDevice device, const buffer_init_info_t *init_info, buffer_t *buffer)
{
   buffer->size = init_info->size;

   {
      const VkBufferCreateInfo info =
      {
         VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
         .size = buffer->size,
         .usage = init_info->usage,
      };
      vkCreateBuffer(device, &info, NULL, &buffer->handle);
   }

   {
      memory_init_info_t info =
      {
         init_info->memory_types,
         .req_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
         .buffer = buffer->handle
      };
      memory_init(device, &info, &buffer->mem);
   }

   if(init_info->data)
   {
      memcpy(buffer->mem.ptr, init_info->data, init_info->size);
      memory_flush(device, &buffer->mem);
   }
}

void buffer_free(VkDevice device, buffer_t *buffer)
{
   memory_free(device, &buffer->mem);
   vkDestroyBuffer(device, buffer->handle, NULL);
   buffer->handle = VK_NULL_HANDLE;
}
