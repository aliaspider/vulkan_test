
#include "vulkan.h"

void memory_free(const context_t* vk, device_memory_t* mem)
{
   if(mem->ptr)
      vkUnmapMemory(vk->device, mem->handle);

   vkFreeMemory(vk->device, mem->handle, NULL);

   mem->ptr = NULL;
   mem->flags = 0;
   mem->handle = VK_NULL_HANDLE;
}

void memory_init(VkDevice device, const memory_init_info_t* init_info, device_memory_t* mem)
{

   VkMemoryRequirements reqs;

   if(init_info->buffer)
      vkGetBufferMemoryRequirements(device, init_info->buffer, &reqs);
   else
      vkGetImageMemoryRequirements(device, init_info->image, &reqs);

   mem->size = reqs.size;
   mem->alignment = reqs.alignment;

   const VkMemoryType* type = init_info->memory_types;
   {
      uint32_t bits = reqs.memoryTypeBits;
      while(bits)
      {
         if((bits & 1) && (type->propertyFlags & init_info->req_flags) == init_info->req_flags)
            break;

         bits >>= 1;
         type++;
      }
   }

   mem->flags = type->propertyFlags;

   {
      const VkMemoryAllocateInfo info =
      {
         VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
         .allocationSize = mem->size,
         .memoryTypeIndex = type - init_info->memory_types
      };
      vkAllocateMemory(device, &info, NULL, &mem->handle);
   }

   if(init_info->req_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
      vkMapMemory(device, mem->handle, 0, mem->size, 0, &mem->ptr);
   else
      mem->ptr = NULL;

   if(init_info->buffer)
      vkBindBufferMemory(device, init_info->buffer, mem->handle, 0);
   else
      vkBindImageMemory(device, init_info->image, mem->handle, 0);
}

void memory_flush(VkDevice device, const device_memory_t* mem)
{
   if(mem->flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
      return;

   VkMappedMemoryRange mappedUniformsMemoryRange =
   {
      VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
      .memory = mem->handle,
      .offset = 0,
      .size = mem->size
   };
   vkFlushMappedMemoryRanges(device, 1, &mappedUniformsMemoryRange);
}
