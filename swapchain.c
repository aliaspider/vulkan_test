
#include "vulkan.h"

void swapchain_init(VkDevice device, const swapchain_init_info_t *init_info, swapchain_t* chain)
{
   VkSwapchainCreateInfoKHR swapchainCreateInfo =
   {
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = init_info->surface,
      .minImageCount = 2,
      .imageFormat = VK_FORMAT_B8G8R8A8_SRGB,
      .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
      .imageExtent.width = init_info->width,
      .imageExtent.height = init_info->height,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = init_info->present_mode,
      .clipped = VK_TRUE
   };
   vkCreateSwapchainKHR(device, &swapchainCreateInfo, NULL, &chain->handle);

   /* init renderpass */
   VkAttachmentDescription attachmentDescriptions[] =
   {
      {
         0, VK_FORMAT_B8G8R8A8_SRGB, VK_SAMPLE_COUNT_1_BIT,
         VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
      }
   };

   VkAttachmentReference ColorAttachment = {0,  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

   VkSubpassDescription subpassDescriptions[] =
   {
      {
         .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
         .colorAttachmentCount = 1, &ColorAttachment,
      }
   };

   VkRenderPassCreateInfo renderPassCreateInfo =
   {
      VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = countof(attachmentDescriptions), attachmentDescriptions,
      .subpassCount = countof(subpassDescriptions), subpassDescriptions,
   };

   vkCreateRenderPass(device, &renderPassCreateInfo, NULL, &chain->renderpass);

   /* init image views and framebuffers */
   vkGetSwapchainImagesKHR(device, chain->handle, &chain->count, NULL);
   if(chain->count > MAX_SWAPCHAIN_IMAGES)
      chain->count = MAX_SWAPCHAIN_IMAGES;

   VkImage swapchainImages[chain->count];
   vkGetSwapchainImagesKHR(device, chain->handle, &chain->count, swapchainImages);


   int i;
   for (i = 0; i < chain->count; i++)
   {
      {
         VkImageViewCreateInfo info =
         {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_B8G8R8A8_SRGB,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.levelCount = 1,
            .subresourceRange.layerCount = 1
         };
         vkCreateImageView(device, &info, NULL, &chain->views[i]);
      }

      {
         VkFramebufferCreateInfo info =
         {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = chain->renderpass,
            .attachmentCount = 1, &chain->views[i],
            .width = init_info->width,
            .height = init_info->height,
            .layers = 1
         };
         vkCreateFramebuffer(device, &info, NULL, &chain->framebuffers[i]);
      }
   }

   chain->viewport.x = 0.0f;
   chain->viewport.y = 0.0f;
   chain->viewport.width = init_info->width;
   chain->viewport.height = init_info->height;
   chain->viewport.minDepth = -1.0f;
   chain->viewport.maxDepth =  1.0f;

   chain->scissor.offset.x = 0.0f;
   chain->scissor.offset.y = 0.0f;
   chain->scissor.extent.width = init_info->width;
   chain->scissor.extent.height = init_info->height;
}

void swapchain_free(VkDevice device, swapchain_t *chain)
{
   int i;
   for (i = 0; i < chain->count; i++)
   {
      vkDestroyImageView(device, chain->views[i], NULL);
      vkDestroyFramebuffer(device, chain->framebuffers[i], NULL);
      chain->views[i] = VK_NULL_HANDLE;
      chain->framebuffers[i] = VK_NULL_HANDLE;
   }
   vkDestroySwapchainKHR(device, chain->handle, NULL);
   vkDestroyRenderPass(device, chain->renderpass, NULL);

   chain->handle = VK_NULL_HANDLE;
   chain->renderpass = VK_NULL_HANDLE;
}
