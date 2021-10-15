#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "VulkanUtils.h"

#define PARTICLE_NUM 20000
#define PARTICLE_RADIUS 0.005f
#define WORK_GROUP_SIZE 128
#define WORK_GROUP_NUM ((PARTICLE_NUM+WORK_GROUP_SIZE-1)/WORK_GROUP_SIZE)

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


struct WindowCreateInfo
{
	std::string title;
	int32_t width;
    int32_t height;
	bool resizeable = false;
};

class App
{
public:
	App(const WindowCreateInfo &info);
	~App();

	void Run();
private:
	void Init();
	void ProcessInput();
	void Update();
	void Draw();

	void CreateWindow(const struct WindowCreateInfo& info);
	void LoadVulkanLib();
	void CreateInstance();
	void CreateDebugUtilsMessenger();
	void CreateSurface();
	void SelectPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSwapChain();
	void CreateRenderPass();
	void CreateSwapChainFrameBuffers();
	void CreateDescriptorPool();
	void CreatePipelineCache();
	void CreatePackedParticleBuffer();
	void CreateGraphicsPipelineLayout();
	void CreateGraphicsPipeline();
	void CreateGraphicsCommandPool();
	void CreateGraphicsCommandBuffers();
	void CreateSemaphores();
	void CreateComputeDescriptorSetLayout();
	void UpdateComputeDescriptorSets();
	void CreateComputePipelineLayout();
	void CreateComputeComputePipelines();

	void DestroyComputePipelines();
	void DestroyComputePipelineLayout();
	void DestroyComputeDescriptorSetLayout();
	void DestroySemaphore();
	void DestroyGraphicsCommandBuffers();
	void DestroyGraphicsCommandPool();
	void DestroyGraphicsPipeline();
	void DestroyGraphicsPipelineLayout();	
	void DestroyPackedParticleBuffer();
	void DestroyPipelineCache();
	void DestroyDescriptorPool();
	void DestroySwapChainFrameBuffers();
	void DestroyRenderPass();
	void DestroySwapChain();
	void DestroyLogicalDevice();
	void DestroySurface();
	void DestroyDebugUtilMessenger();
	void DestroyInstance();
	void UnLoadVulkanLib();
	void DestroyWindow();

	bool m_IsRunning;

    WindowCreateInfo m_WindowCreateInfo;

	SDL_Window* m_WindowHandle;

	VkInstance m_InstanceHandle;

	VkDebugUtilsMessengerEXT m_DebugMessengerHandle;

	VkSurfaceKHR m_SurfaceHandle;

	VkPhysicalDevice m_PhysicalDeviceHandle;

	QueueFamilyIndices m_QueueIndices;
	VkDevice m_LogicalDeviceHandle;
	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;
	VkQueue m_ComputeQueue;

	SwapChainSupportDetails m_SwapChainSupportDetails;
	VkSwapchainKHR m_SwapChainHandle;
	std::vector<VkImage> m_SwapChainImages;
	std::vector<VkImageView> m_SwapChainImageViews;
	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtent;

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


	const uint64_t m_PosSsboSize = sizeof(glm::vec2) * PARTICLE_NUM;
	const uint64_t m_VelocitySsboSize = sizeof(glm::vec2) * PARTICLE_NUM;
	const uint64_t m_ForceSsboSize = sizeof(glm::vec2) * PARTICLE_NUM;
	const uint64_t m_DensitySsboSize = sizeof(float) * PARTICLE_NUM;
	const uint64_t m_PressureSsboSize = sizeof(float) * PARTICLE_NUM;

	const uint64_t m_PackedBufferSize = m_PosSsboSize + m_VelocitySsboSize + m_ForceSsboSize + m_DensitySsboSize + m_PressureSsboSize;

	const uint64_t m_PosSsboOffset = 0;
	const uint64_t m_VelocitySsboOffset = m_PosSsboSize;
	const uint64_t m_ForceSsboOffset = m_VelocitySsboOffset+ m_VelocitySsboSize;
	const uint64_t m_DensitySsboOffset = m_ForceSsboOffset+ m_ForceSsboSize;
	const uint64_t m_PressureSsboOffset = m_DensitySsboOffset+ m_DensitySsboSize;

	glm::ivec2 m_SpawnPos;
};
