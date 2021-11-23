#pragma once
#include <vulkan/vulkan.h>
#include <optional>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <string_view>
#include <array>
#include <deque>
#include <functional>

namespace VK
{

#define VK_CHECK(x)                                               \
	do                                                            \
	{                                                             \
		VkResult err = x;                                         \
		if (err)                                                  \
		{                                                         \
			std::cout << "Detected  error: " << err << std::endl; \
			abort();                                              \
		}                                                         \
	} while (0);

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

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> computeFamily;

		bool IsComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
		}

		bool IsSameFamily()
		{
			return graphicsFamily.value() == presentFamily.value() &&
				   graphicsFamily.value() == computeFamily.value() &&
				   presentFamily.value() == computeFamily.value();
		}

		std::array<uint32_t, 3> FamilyIndexArray()
		{
			return {graphicsFamily.value(), presentFamily.value(), computeFamily.value()};
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	std::vector<VkLayerProperties> GetInstanceLayerProps();
	std::vector<VkExtensionProperties> GetInstanceExtensionProps();
	bool CheckValidationLayerSupport(std::vector<const char *> validationLayerNames, std::vector<VkLayerProperties> instanceLayerProps);
	bool CheckExtensionSupport(std::vector<const char *> extensionNames, std::vector<VkExtensionProperties> extensionProps);

	std::vector<VkExtensionProperties> GetPhysicalDeviceExtensionProps(VkPhysicalDevice device);
	VkPhysicalDeviceProperties GetPhysicalDeviceProps(VkPhysicalDevice device);
	VkPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProps(VkPhysicalDevice device);
	VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures(VkPhysicalDevice device);

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

	VkSurfaceFormatKHR ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
	VkPresentModeKHR ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
	VkExtent2D ChooseSwapChainExtent(SDL_Window *window, const VkSurfaceCapabilitiesKHR &capabilities);

	VkShaderModule CreateShaderModuleFromSpirvFile(VkDevice device, std::string_view filePath);

}