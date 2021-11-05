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

#define PARTICLE_NUM 20000
#define PARTICLE_RADIUS 0.005f
#define WORK_GROUP_SIZE 128
#define WORK_GROUP_NUM ((PARTICLE_NUM + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE)

const std::vector<const char *> validationLayers = {
	"VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct DeletionQueue
{
	std::deque<std::function<void()>> deletors;

	void Add(std::function<void()> &&function)
	{
		deletors.emplace_back(function);
	}

	void Flush()
	{
		for (auto it = deletors.rbegin(); it != deletors.rend(); ++it)
			(*it)();

		deletors.clear();
	}
};

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
	void CreateRenderPass();
	void CreateSwapChainFrameBuffers();
	void CreateDescriptorPool();
	void CreatePipelineCache();
	void CreateGraphicsPipelineLayout();
	void CreateGraphicsPipeline();
	void CreateGraphicsCommandPool();
	void CreateGraphicsCommandBuffers();
	void CreateSemaphores();
	void CreateComputeDescriptorSetLayout();
	void CreateComputePipelineLayout();
	void CreateComputePipelines();
	void CreateComputeCommandPool();
	void CreateComputeCommandBuffer();

	void InitParticleData(std::array<glm::vec2, PARTICLE_NUM> initParticlePosition);
	void CreatePackedParticleBuffer();
	void UpdateComputeDescriptorSets();

	bool m_IsRunning;

	std::string m_WindowTitle;
	int32_t m_WindowWidth, m_WindowHeight;
	SDL_Window *m_WindowHandle;

	std::unique_ptr<VulkanInstance> m_Instance;
	std::unique_ptr<VulkanDevice> m_Device;
	std::unique_ptr<VulkanSwapChain> m_SwapChain;

	VkRenderPass m_RenderPassHandle;

	std::vector<VkFramebuffer> m_SwapChainFrameBufferHandles;

	VkDescriptorPool m_GlobalDescriptorPoolHandle;
	VkPipelineCache m_GlobalPipelineCacheHandle;

	VkBuffer m_PackedParticlesBufferHandle;
	VkDeviceMemory m_PackedParticleBufferMemoryHandle;

	VkPipelineLayout m_GraphicsPipelineLayoutHandle;
	VkPipeline m_GraphicePipelineHandle;

	VkCommandPool m_GraphicsCommandPoolHandle;
	std::vector<VkCommandBuffer> m_GraphicsCommandBufferHandles;

	VkSemaphore m_ImageAvailableSemaphoreHandle;
	VkSemaphore m_RenderFinishedSemaphoreHandle;

	VkDescriptorSetLayout m_ComputeDescriptorSetLayoutHandle;
	VkDescriptorSet m_ComputeDescriptorSetHandle;
	VkPipelineLayout m_ComputePipelineLayoutHandle;
	VkPipeline m_ComputePipelineHandles[3];

	VkCommandPool m_ComputeCommandPoolHandle;
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
