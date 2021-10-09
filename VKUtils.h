#pragma once
#include <vulkan/vulkan.h>
#include <optional>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#define VK_CHECK(x)                                                     \
    do                                                                  \
    {                                                                   \
        VkResult err = x;                                               \
        if (err)                                                        \
        {                                                               \
            std::cout << "Detected Vulkan error: " << err << std::endl; \
            abort();                                                    \
        }                                                               \
    } while (0);
struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> computeFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
	}

	bool IsSameFamily()
	{
		return graphicsFamily.value() == presentFamily.value() &&
			graphicsFamily.value() == computeFamily.value() &&
			presentFamily.value() == computeFamily.value();
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

std::vector<VkExtensionProperties> GetPhysicalDeviceExtensionProps(VkPhysicalDevice device);
VkPhysicalDeviceProperties GetPhysicalDeviceProps(VkPhysicalDevice device);
VkPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProps(VkPhysicalDevice device);
VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures(VkPhysicalDevice device);

bool CheckValidationLayerSupport(std::vector<const char*> validationLayerNames, std::vector<VkLayerProperties> instanceLayerProps);
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

VkSurfaceFormatKHR ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
VkExtent2D ChooseSwapChainExtent(SDL_Window* window, const VkSurfaceCapabilitiesKHR& capabilities);

std::vector<VkImage> GetSwapChainImages(VkDevice device, VkSwapchainKHR swapChain);
std::vector<VkImageView> CreateSwapChainImageViews(VkDevice device, const std::vector<VkImage>& images, VkFormat format);