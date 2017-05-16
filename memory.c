
#include "vulkan.h"

void memory_init(const context_t* vk, const VkMemoryRequirements* reqs, VkMemoryPropertyFlags flags, device_memory_t* mem)
{
   int i;

   for (i = 0; i < vk->mem.memoryTypeCount; i++)
   {
      if ((reqs->memoryTypeBits & (1 << i)) && ((vk->mem.memoryTypes[i].propertyFlags & flags) == flags))
      {
         mem->flags = vk->mem.memoryTypes[i].propertyFlags;
         mem->size = reqs->size;
         mem->alignment = reqs->alignment;

         VkMemoryAllocateInfo memoryAllocateInfo =
         {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, NULL,
            reqs->size, i
         };
         vkAllocateMemory(vk->device, &memoryAllocateInfo, NULL, &mem->handle);

         if(flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            vkMapMemory(vk->device, mem->handle, 0, mem->size, 0, &mem->ptr);

         return;
      }
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

void buffer_memory_init(const context_t* vk, VkBuffer buffer, VkMemoryPropertyFlags flags, device_memory_t* mem)
{
   VkMemoryRequirements reqs;
   vkGetBufferMemoryRequirements(vk->device, buffer, &reqs);
   memory_init(vk, &reqs, flags, mem);
   vkBindBufferMemory(vk->device, buffer, mem->handle, 0);
}

void image_memory_init(const context_t* vk, VkImage image, VkMemoryPropertyFlags flags, device_memory_t* mem)
{
   VkMemoryRequirements reqs;
   vkGetImageMemoryRequirements(vk->device, image, &reqs);
   memory_init(vk, &reqs, flags, mem);
   vkBindImageMemory(vk->device, image, mem->handle, 0);
}

void memory_flush(const context_t* vk, const device_memory_t* mem)
{
   if(mem->flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
      return;

   VkMappedMemoryRange mappedUniformsMemoryRange =
   {
      VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, NULL, mem->handle, 0, mem->size
   };
   vkFlushMappedMemoryRanges(vk->device, 1, &mappedUniformsMemoryRange);
}
