
#include "vulkan.h"

void memory_init(const context_t* vk, const VkMemoryRequirements* reqs, VkMemoryPropertyFlags req_flags, device_memory_t* mem)
{
   const VkMemoryType* type = vk->mem.memoryTypes;
   uint32_t bits = reqs->memoryTypeBits;

   while(bits)
   {
      if((bits & 1) && (type->propertyFlags & req_flags) == req_flags)
      {
         mem->flags = type->propertyFlags;
         mem->size = reqs->size;
         mem->alignment = reqs->alignment;

         {
            const VkMemoryAllocateInfo info =
            {
               VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
               .allocationSize = reqs->size,
               .memoryTypeIndex = type - vk->mem.memoryTypes
            };
            vkAllocateMemory(vk->device, &info, NULL, &mem->handle);
         }

         if(req_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            vkMapMemory(vk->device, mem->handle, 0, mem->size, 0, &mem->ptr);
         else
            mem->ptr = NULL;

         return;
      }
      bits >>= 1;
      type++;
   }
}

void memory_free(const context_t* vk, device_memory_t* mem)
{
   if(mem->ptr)
      vkUnmapMemory(vk->device, mem->handle);

   vkFreeMemory(vk->device, mem->handle, NULL);

   mem->ptr = NULL;
   mem->flags = 0;
   mem->handle = VK_NULL_HANDLE;
}

void buffer_memory_init(const context_t* vk, VkBuffer buffer, VkMemoryPropertyFlags req_flags, device_memory_t* mem)
{
   VkMemoryRequirements reqs;
   vkGetBufferMemoryRequirements(vk->device, buffer, &reqs);

   memory_init(vk, &reqs, req_flags, mem);

   vkBindBufferMemory(vk->device, buffer, mem->handle, 0);
}

void image_memory_init(const context_t* vk, VkImage image, VkMemoryPropertyFlags req_flags, device_memory_t* mem)
{
   VkMemoryRequirements reqs;
   vkGetImageMemoryRequirements(vk->device, image, &reqs);

   memory_init(vk, &reqs, req_flags, mem);

   vkBindImageMemory(vk->device, image, mem->handle, 0);
}

void memory_flush(const context_t* vk, const device_memory_t* mem)
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
   vkFlushMappedMemoryRanges(vk->device, 1, &mappedUniformsMemoryRange);
}
