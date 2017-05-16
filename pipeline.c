
#include "vulkan.h"

void pipeline_init(const context_t* vk, const shaders_t *shaders,
                   int vertex_size, int attrib_count, const VkVertexInputAttributeDescription* attrib_desc,
                   const swapchain_t* chain, const descriptor_t* desc, pipeline_t* pipe)
{
   {
#if 0
      VkPushConstantRange ranges[] =
      {
         {
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            .size = sizeof(uniforms_t)
         }
      };
#endif
      const VkPipelineLayoutCreateInfo info =
      {
         VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
         .setLayoutCount = 1, &desc->set_layout,
//         .pushConstantRangeCount = countof(ranges), ranges
      };

      vkCreatePipelineLayout(vk->device, &info, NULL, &pipe->layout);
   }

   {
      const VkPipelineShaderStageCreateInfo shader_info[] =
      {
         {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .pName = "main",
            .module = shaders->vs
         },
         {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pName = "main",
            .module = shaders->fs
         }
      };

      const VkVertexInputBindingDescription vertex_description =
      {
         0, vertex_size, VK_VERTEX_INPUT_RATE_VERTEX
      };

      const VkPipelineVertexInputStateCreateInfo vertex_input_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
         .vertexBindingDescriptionCount = 1, &vertex_description,
         .vertexAttributeDescriptionCount = attrib_count, attrib_desc
      };

      const VkPipelineInputAssemblyStateCreateInfo input_assembly_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
         .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
         .primitiveRestartEnable = VK_FALSE
      };

      const VkPipelineViewportStateCreateInfo viewport_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
         .viewportCount = 1, &chain->viewport,
         .scissorCount = 1, &chain->scissor
      };

      const VkPipelineRasterizationStateCreateInfo rasterization_info =
      {
         VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
         .lineWidth = 1.0f
      };

      const VkPipelineMultisampleStateCreateInfo multisample_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
         .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      };

      const VkPipelineColorBlendAttachmentState attachement_state =
      {
         .blendEnable = VK_FALSE,
         .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
      };

      const VkPipelineColorBlendStateCreateInfo colorblend_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
         .attachmentCount = 1, &attachement_state
      };

      const VkGraphicsPipelineCreateInfo info =
      {
         VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
         .flags = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT,
         .stageCount = countof(shader_info), shader_info,
         .pVertexInputState = &vertex_input_state,
         .pInputAssemblyState = &input_assembly_state,
         .pViewportState = &viewport_state,
         .pRasterizationState = &rasterization_info,
         .pMultisampleState = &multisample_state,
         .pColorBlendState = &colorblend_state,
         .layout = pipe->layout,
         .renderPass = chain->renderpass,
         .subpass = 0
      };
      vkCreateGraphicsPipelines(vk->device, VK_NULL_HANDLE, 1, &info, NULL, &pipe->handle);
   }
}

void pipeline_free(const context_t *vk, pipeline_t *pipe)
{
   vkDestroyPipelineLayout(vk->device, pipe->layout, NULL);
   vkDestroyPipeline(vk->device, pipe->handle, NULL);
   pipe->layout = VK_NULL_HANDLE;
   pipe->handle = VK_NULL_HANDLE;
}
