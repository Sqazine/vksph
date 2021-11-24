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
#include "Utils.h"
#include "Instance.h"
#include "Device.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "DescriptorPool.h"
#include "PipelineCache.h"
#include "Semaphore.h"
#include "PipelineLayout.h"
#include "CommandPool.h"
#include "DescriptorSetLayout.h"
#include "Buffer.h"
#include "ComputePipeline.h"
#include "GraphicsPipeline.h"
#include "Fence.h"
#include "GraphicsCOntext.h"
#define PARTICLE_NUM 20000
#define PARTICLE_RADIUS 0.005f
#define WORK_GROUP_SIZE 128
#define WORK_GROUP_NUM ((PARTICLE_NUM + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE)

#define MAX_FRAMES_IN_FLIGHT 2


class App
{
public:
	App(const WindowCreateInfo& windowInfo);
	~App();

	void Run();

private:
	void Init();
	void Update();
	void Simulate();
	void Draw();
	void SubmitAndPresent();

	void InitParticleData(std::array<glm::vec2, PARTICLE_NUM> initParticlePosition);

	void UpdateComputeDescriptorSets();

	bool m_IsRunning;
	WindowCreateInfo m_WinInfo;

	std::unique_ptr<VK::SwapChain> m_SwapChain;
	std::unique_ptr<VK::RenderPass> m_RenderPass;
	std::vector<std::unique_ptr<VK::Framebuffer>> m_SwapChainFrameBuffers;
	std::unique_ptr<VK::DescriptorPool> m_GlobalDescriptorPool;
	std::unique_ptr<VK::PipelineCache> m_GlobalPipelineCache;

	std::unique_ptr<VK::Buffer> m_PackedParticlesBuffer;

	std::unique_ptr<VK::PipelineLayout>  m_GraphicsPipelineLayout;
	std::unique_ptr<VK::GraphicsPipeline> m_GraphicsPipeline;

	std::unique_ptr<VK::CommandPool> m_GraphicsCommandPool;
	std::vector<VkCommandBuffer> m_GraphicsCommandBufferHandles;

	std::vector<std::unique_ptr<VK::Semaphore>> m_ImageAvailableSemaphores;
	std::vector<std::unique_ptr<VK::Semaphore>> m_RenderFinishedSemaphores;
	std::vector<std::unique_ptr<VK::Fence>> m_InFlightFences;
	std::vector<VK::Fence*> m_ImagesInFlight;
	size_t currentFrame=0;

	std::unique_ptr<VK::DescriptorSetLayout> m_ComputeDescriptorSetLayout;
	VkDescriptorSet m_ComputeDescriptorSetHandle;
	std::unique_ptr<VK::PipelineLayout > m_ComputePipelineLayout;
	 std::array<std::unique_ptr<VK::ComputePipeline>,3> m_ComputePipelines;

	std::unique_ptr<VK::CommandPool> m_ComputeCommandPool;
	VkCommandBuffer m_ComputeCommandBufferHandle;

	VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

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
