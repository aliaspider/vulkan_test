
#include "vulkan.h"

void swapchain_init(const context_t* vk, int width, int height, VkPresentModeKHR present_mode, swapchain_t* chain)
{
   /* init display and window */
#ifdef VK_USE_PLATFORM_XLIB_KHR
   XInitThreads();
   chain->display = XOpenDisplay(NULL);
   chain->window  = XCreateSimpleWindow(chain->display, DefaultRootWindow(chain->display), 0, 0, width, height, 0, 0, 0);
   XStoreName(chain->display, chain->window, "Vulkan Test");
   XSelectInput(chain->display, chain->window, ExposureMask | KeyPressMask);
   XMapWindow(chain->display, chain->window);

   /* init surface */
   VkXlibSurfaceCreateInfoKHR xlibSurfaceCreateInfo =
   {
      VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
      .dpy = chain->display,
      .window = chain->window
   };
   vkCreateXlibSurfaceKHR(vk->instance, &xlibSurfaceCreateInfo, NULL, &chain->surface);
#else
#error platform not supported
#endif

   /* init swapchain */
   VkSurfaceCapabilitiesKHR surfaceCapabilities;
   vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk->gpu, chain->surface, &surfaceCapabilities);

   VkBool32 physicalDeviceSurfaceSupport;
   vkGetPhysicalDeviceSurfaceSupportKHR(vk->gpu, vk->queue_family_index, chain->surface, &physicalDeviceSurfaceSupport);

   uint32_t surfaceFormatcount;
   vkGetPhysicalDeviceSurfaceFormatsKHR(vk->gpu, chain->surface, &surfaceFormatcount, NULL);
   VkSurfaceFormatKHR surfaceFormats[surfaceFormatcount];
   vkGetPhysicalDeviceSurfaceFormatsKHR(vk->gpu, chain->surface, &surfaceFormatcount, surfaceFormats);

   uint32_t presentModeCount;
   vkGetPhysicalDeviceSurfacePresentModesKHR(vk->gpu, chain->surface, &presentModeCount, NULL);
   VkPresentModeKHR presentModes[presentModeCount];
   vkGetPhysicalDeviceSurfacePresentModesKHR(vk->gpu, chain->surface, &presentModeCount, presentModes);

   VkSwapchainCreateInfoKHR swapchainCreateInfo =
   {
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = chain->surface,
      .minImageCount = 2,
      .imageFormat = VK_FORMAT_B8G8R8A8_SRGB,
      .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
      .imageExtent.width = width,
      .imageExtent.height = height,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = present_mode,
      .clipped = VK_TRUE
   };
   vkCreateSwapchainKHR(vk->device, &swapchainCreateInfo, NULL, &chain->handle);

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

   vkCreateRenderPass(vk->device, &renderPassCreateInfo, NULL, &chain->renderpass);

   /* init image views and framebuffers */
   vkGetSwapchainImagesKHR(vk->device, chain->handle, &chain->count, NULL);
   if(chain->count > MAX_SWAPCHAIN_IMAGES)
      chain->count = MAX_SWAPCHAIN_IMAGES;

   VkImage swapchainImages[chain->count];
   vkGetSwapchainImagesKHR(vk->device, chain->handle, &chain->count, swapchainImages);


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
         vkCreateImageView(vk->device, &info, NULL, &chain->views[i]);
      }

      {
         VkFramebufferCreateInfo info =
         {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = chain->renderpass,
            .attachmentCount = 1, &chain->views[i],
            .width = width,
            .height = height,
            .layers = 1
         };
         vkCreateFramebuffer(vk->device, &info, NULL, &chain->framebuffers[i]);
      }
   }

   chain->viewport.x = 0.0f;
   chain->viewport.y = 0.0f;
   chain->viewport.width = width;
   chain->viewport.height = height;
   chain->viewport.minDepth = -1.0f;
   chain->viewport.maxDepth =  1.0f;

   chain->scissor.offset.x = 0.0f;
   chain->scissor.offset.y = 0.0f;
   chain->scissor.extent.width = width;
   chain->scissor.extent.height = height;
}

void swapchain_free(const context_t *vk, swapchain_t *chain)
{
   int i;
   for (i = 0; i < chain->count; i++)
   {
      vkDestroyImageView(vk->device, chain->views[i], NULL);
      vkDestroyFramebuffer(vk->device, chain->framebuffers[i], NULL);
      chain->views[i] = VK_NULL_HANDLE;
      chain->framebuffers[i] = VK_NULL_HANDLE;
   }
   vkDestroySwapchainKHR(vk->device, chain->handle, NULL);
   vkDestroySurfaceKHR(vk->instance, chain->surface, NULL);
   vkDestroyRenderPass(vk->device, chain->renderpass, NULL);

   XDestroyWindow(chain->display, chain->window);
   XCloseDisplay(chain->display);

   chain->handle = VK_NULL_HANDLE;
   chain->surface = VK_NULL_HANDLE;
   chain->renderpass = VK_NULL_HANDLE;
   chain->display = NULL;
}
