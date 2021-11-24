#include "RenderPass.h"
#include "SwapChain.h"
#include "Utils.h"
#include <iostream>
#include "GraphicsContext.h"

namespace VK
{

   RenderPass::RenderPass( const SwapChain *swapChain)
       
   {
      VkAttachmentDescription colorAttachment{};
      colorAttachment.format = swapChain->GetVKSwapChainImageFormat();
      colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
      colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

      VkAttachmentReference colorAttachmentRef = {};
      colorAttachmentRef.attachment = 0;
      colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

      VkSubpassDescription subpass{};
      subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
      subpass.colorAttachmentCount = 1;
      subpass.pColorAttachments = &colorAttachmentRef;

      VkRenderPassCreateInfo renderPassInfo{};
      renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      renderPassInfo.attachmentCount = 1;
      renderPassInfo.pAttachments = &colorAttachment;
      renderPassInfo.subpassCount = 1;
      renderPassInfo.pSubpasses = &subpass;

      VK_CHECK(vkCreateRenderPass(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), &renderPassInfo, nullptr, &m_RenderPassHandle));
   }
   RenderPass::~RenderPass()
   {
      vkDestroyRenderPass(GraphicsContext::GetDevice()->GetLogicalDeviceHandle(), m_RenderPassHandle, nullptr);
   }

   const VkRenderPass &RenderPass::GetVKRenderPassHandle() const
   {
      return m_RenderPassHandle;
   }
}