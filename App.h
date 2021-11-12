#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <deque>
#include <functional>
#include <array>
#include <memory>
#include "VulkanUtils.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderPass.h"
#include "VulkanFramebuffer.h"
#include "VulkanDescriptorPool.h"
#include "VulkanPipelineCache.h"
#include "VulkanSemaphore.h"
#include "VulkanPipelineLayout.h"
#include "VulkanCommandPool.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanBuffer.h"
#include "VulkanComputePipeline.h"
#include "VulkanGraphicsPipeline.h"

#define PARTICLE_NUM 20000
#define PARTICLE_RADIUS 0.005f
#define WORK_GROUP_SIZE 128
#define WORK_GROUP_NUM ((PARTICLE_NUM + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE)

const std::vector<const char *> validationLayers = {
	"VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME};

class App
{
public:
	App(std::string title, int32_t width, int32_t height);
	~App();

	void Run();

private:
	void Init();
	void Update();
	void Simulate();
	void Draw();
	void SubmitAndPresent();

	void CreateWindow(std::string title, int32_t width, int32_t height);
	void LoadVulkanLib();

	void InitParticleData(std::array<glm::vec2, PARTICLE_NUM> initParticlePosition);

	void UpdateComputeDescriptorSets();

	bool m_IsRunning;

	std::string m_WindowTitle;
	int32_t m_WindowWidth, m_WindowHeight;
	SDL_Window *m_WindowHandle;

	std::unique_ptr<VulkanInstance> m_Instance;
	std::unique_ptr<VulkanDevice> m_Device;
	std::unique_ptr<VulkanSwapChain> m_SwapChain;
	std::unique_ptr<VulkanRenderPass> m_RenderPass;
	std::vector<std::unique_ptr<VulkanFramebuffer>> m_SwapChainFrameBuffers;
	std::unique_ptr<VulkanDescriptorPool> m_GlobalDescriptorPool;
	std::unique_ptr<VulkanPipelineCache> m_GlobalPipelineCache;

	std::unique_ptr<VulkanBuffer> m_PackedParticlesBuffer;

	std::unique_ptr<VulkanPipelineLayout>  m_GraphicsPipelineLayout;
	std::unique_ptr<VulkanGraphicsPipeline> m_GraphicsPipeline;

	std::unique_ptr<VulkanCommandPool> m_GraphicsCommandPool;
	std::vector<VkCommandBuffer> m_GraphicsCommandBufferHandles;

	std::unique_ptr<VulkanSemaphore> m_ImageAvailableSemaphore;
	std::unique_ptr<VulkanSemaphore> m_RenderFinishedSemaphore;

	std::unique_ptr<VulkanDescriptorSetLayout> m_ComputeDescriptorSetLayout;
	VkDescriptorSet m_ComputeDescriptorSetHandle;
	std::unique_ptr<VulkanPipelineLayout > m_ComputePipelineLayout;
	 std::array<std::unique_ptr<VulkanComputePipeline>,3> m_ComputePipelines;

	std::unique_ptr<VulkanCommandPool> m_ComputeCommandPool;
	VkCommandBuffer m_ComputeCommandBufferHandle;

	VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	uint32_t m_SwapChainImageIndex = 0;

	DeletionQueue m_DeletionQueue;

	const uint64_t m_PosSsboSize = sizeof(glm::vec2) * PARTICLE_NUM;
	const uint64_t m_VelocitySsboSize = sizeof(glm::vec2) * PARTICLE_NUM;
	const uint64_t m_ForceSsboSize = sizeof(glm::vec2) * PARTICLE_NUM;
	const uint64_t m_DensitySsboSize = sizeof(float) * PARTICLE_NUM;
	const uint64_t m_PressureSsboSize = sizeof(float) * PARTICLE_NUM;

	const uint64_t m_PackedBufferSize = m_PosSsboSize + m_VelocitySsboSize + m_ForceSsboSize + m_DensitySsboSize + m_PressureSsboSize;

	const uint64_t m_PosSsboOffset = 0;
	const uint64_t m_VelocitySsboOffset = m_PosSsboSize;
	const uint64_t m_ForceSsboOffset = m_VelocitySsboOffset + m_VelocitySsboSize;
	const uint64_t m_DensitySsboOffset = m_ForceSsboOffset + m_ForceSsboSize;
	const uint64_t m_PressureSsboOffset = m_DensitySsboOffset + m_DensitySsboSize;

	glm::ivec2 m_SpawnPos;
};
