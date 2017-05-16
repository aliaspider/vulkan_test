
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <X11/Xutil.h>

#include "vulkan.h"
#include "png_file.h"

typedef struct
{
   struct
   {
      float x;
      float y;
   } center;
   struct
   {
      float width;
      float height;
   } size;
   float angle;
} uniforms_t;

typedef struct
{
   struct
   {
      float x, y, z, w;
   } position;
   struct
   {
      float u, v;
   } texcoord;
   struct
   {
      float r, g, b, a;
   } color;
} vertex_t;

static const uint32_t vs_code [] =
#include "main.vert.inc"
   ;
static const uint32_t fs_code [] =
#include "main.frag.inc"
   ;

bool running = true;

#ifdef VK_USE_PLATFORM_XLIB_KHR
bool handle_input(Display* display, Window window, uniforms_t* uniforms)
{
   XEvent e;
   if (XCheckWindowEvent(display, window, ~0, &e) && (e.type == KeyPress))
   {
      if(e.xkey.state & Mod1Mask)
      {
         if(e.xkey.keycode == XKeysymToKeycode(display, XK_Left))
            uniforms->angle += M_PI / 60;
         if(e.xkey.keycode == XKeysymToKeycode(display, XK_Right))
            uniforms->angle -= M_PI / 60;
      }
      else if(e.xkey.state & ControlMask)
      {
         if(e.xkey.keycode == XKeysymToKeycode(display, XK_Left))
            uniforms->size.width += 0.1;
         if(e.xkey.keycode == XKeysymToKeycode(display, XK_Right))
            uniforms->size.width -= 0.1;
         if(e.xkey.keycode == XKeysymToKeycode(display, XK_Up))
            uniforms->size.height += 0.1;
         if(e.xkey.keycode == XKeysymToKeycode(display, XK_Down))
            uniforms->size.height -= 0.1;
      }
      else
      {
         if(e.xkey.keycode == XKeysymToKeycode(display, XK_q))
         {
            running = false;
            return false;
         }
         if(e.xkey.keycode == XKeysymToKeycode(display, XK_Left))
            uniforms->center.x -= 0.1;
         if(e.xkey.keycode == XKeysymToKeycode(display, XK_Right))
            uniforms->center.x += 0.1;
         if(e.xkey.keycode == XKeysymToKeycode(display, XK_Up))
            uniforms->center.y -= 0.1;
         if(e.xkey.keycode == XKeysymToKeycode(display, XK_Down))
            uniforms->center.y += 0.1;
      }
      return true;
   }
   return false;
}
#endif

int main(int argc, char **argv)
{
   printf("Vulkan test:\n");


   context_t vk;
   context_init(&vk);

   swapchain_t chain;
   swapchain_init(&vk, 640, 480, VK_PRESENT_MODE_FIFO_KHR, &chain);

   png_file_t png;
   png_file_init("texture.png", &png);

   texture_t tex;
   texture_init(&vk, png.width, png.height, &tex);

   png_file_read(&png, tex.mem.u8 + tex.layout.offset, tex.layout.rowPitch);
   memory_flush(&vk, &tex.mem);
   png_file_free(&png);

   buffer_t vbo;
   {
      vertex_t vertices[] =
      {
         {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
         {{ 1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
         {{ 1.0f,  1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
         {{-1.0f,  1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}
      };
      vertex_buffer_init(&vk, sizeof(vertices), vertices, &vbo);
   }

   buffer_t  ubo;
   uniform_buffer_init(&vk, sizeof(uniforms_t), &ubo);
   uniforms_t *uniforms = ubo.mem.ptr;
   uniforms->center.x = 0.0f;
   uniforms->center.y = 0.0f;
   uniforms->size.width  = 1.0f;
   uniforms->size.height = 1.0f;
   uniforms->angle = 0.0f;

   descriptor_t desc;
   {
      VkVertexInputAttributeDescription attrib_desc[] =
      {
         {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex_t, position)},
         {1, 0, VK_FORMAT_R32G32_SFLOAT,       offsetof(vertex_t, texcoord)},
         {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex_t, color)}
      };
      descriptors_init(&vk, sizeof(vertex_t), countof(attrib_desc), attrib_desc, &ubo, &tex, &desc);
   }

   shaders_t shaders;
   shaders_init(&vk, sizeof(vs_code), vs_code, sizeof(fs_code), fs_code, &shaders);   

   pipeline_t pipe;
   pipeline_init(&vk, &desc, &chain, &shaders, &pipe);

   shaders_free(&vk, &shaders);


   VkCommandBuffer cmd;
   {
      VkCommandBufferAllocateInfo commandBufferAllocateInfo =
      {
         VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
         .commandPool = vk.cmd_pool,
         .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
         .commandBufferCount = 1
      };
      vkAllocateCommandBuffers(vk.device, &commandBufferAllocateInfo, &cmd);
   }

   VkFence queue_fence;
   VkFence chain_fence;
   {
      VkFenceCreateInfo fcinfo =
      {
         VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
         .flags = VK_FENCE_CREATE_SIGNALED_BIT
      };
      vkCreateFence(vk.device, &fcinfo, NULL, &chain_fence);
      vkCreateFence(vk.device, &fcinfo, NULL, &queue_fence);
   }

   int frames = 0;
   struct timespec start_time;
   clock_gettime(CLOCK_MONOTONIC, &start_time);

   while (running)
   {
      uint32_t image_index;
      vkWaitForFences(vk.device, 1, &chain_fence, VK_TRUE, -1);
      vkResetFences(vk.device, 1, &chain_fence);
      vkAcquireNextImageKHR(vk.device, chain.handle, UINT64_MAX, NULL, chain_fence, &image_index);

      vkWaitForFences(vk.device, 1, &queue_fence, VK_TRUE, -1);
      vkResetFences(vk.device, 1, &queue_fence);

      {
         VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
         commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
         vkBeginCommandBuffer(cmd, &commandBufferBeginInfo);
      }

      {
         VkClearValue clearValue = {{{0.0f, 0.1f, 1.0f, 0.0f}}};
         VkRenderPassBeginInfo renderPassBeginInfo =
         {
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = chain.renderpass,
            .framebuffer = chain.framebuffers[image_index],
            .renderArea = chain.scissor,
            .clearValueCount = 1,
            .pClearValues = &clearValue
         };
         vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
      }

      vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle);
      vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.layout, 0, 1, &desc.set, 0 , NULL);
//      vkCmdPushConstants(vk.cmd, vk.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uniforms_t), mapped_uniforms);

      VkDeviceSize vbo_offset = 0;
      vkCmdBindVertexBuffers(cmd, 0, 1, &vbo.handle, &vbo_offset);

      vkCmdDraw(cmd, 4, 1, 0, 0);

      vkCmdEndRenderPass(cmd);
      vkEndCommandBuffer(cmd);

      {
         VkSubmitInfo submitInfo =
         {
            VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmd
         };
         vkQueueSubmit(vk.queue, 1, &submitInfo, queue_fence);
      }

      {
         VkPresentInfoKHR presentInfo =
         {
            VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .swapchainCount = 1,
            .pSwapchains = &chain.handle,
            .pImageIndices = &image_index
         };
         vkQueuePresentKHR(vk.queue, &presentInfo);
      }

      struct timespec end_time;
      clock_gettime(CLOCK_MONOTONIC, &end_time);
      float diff = (end_time.tv_sec - start_time.tv_sec) +
                   (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0f;
      frames++;

      if(handle_input(chain.display, chain.window, uniforms))
         memory_flush(&vk, &ubo.mem);

      if (diff > 0.5f)
      {
         printf("\r fps: %f", frames / diff);
         frames = 0;
         start_time = end_time;
         fflush(stdout);
      }
   }

   vkWaitForFences(vk.device, 1, &queue_fence, VK_TRUE, -1);
   vkDestroyFence(vk.device, queue_fence, NULL);

   vkWaitForFences(vk.device, 1, &chain_fence, VK_TRUE, -1);
   vkDestroyFence(vk.device, chain_fence, NULL);

   pipeline_free(&vk, &pipe);
   descriptors_free(&vk, &desc);
   buffer_free(&vk, &ubo);
   buffer_free(&vk, &vbo);
   texture_free(&vk, &tex);
   swapchain_free(&vk, &chain);
   context_free(&vk);

   printf("\n");
   return 0;
}
