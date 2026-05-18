#include "dxvk_imgui.h"

#include "../util/log/log.h"
#include "../vulkan/vulkan_loader.h"

#include "imgui_impl_win32.h"
#include "imgui_impl_vulkan.h"

namespace dxvk {
    static void checkVkResult(VkResult err) {
        if (err == VK_SUCCESS)
            return;
        Logger::info("[imgui] [vulkan] Error: VkResult =" + std::to_string(err));
    }

    bool DxvkImgui::m_initialized = false;
    bool DxvkImgui::m_useDynamicRendering = false;
    ImGuiIO *DxvkImgui::m_io = nullptr;
    VkDescriptorPool DxvkImgui::m_descriptorPool = VK_NULL_HANDLE;
    HWND DxvkImgui::m_hwnd = nullptr;

    void DxvkImgui::init(const Rc<DxvkDevice> &device, HWND hwnd) {
        if (hwnd == nullptr) {
            Logger::err("[imgui] Please set the hwnd first!");
            return;
        }
        m_hwnd = hwnd;

        {
            Logger::info("[imgui] Start loading ImGui library");
            auto loader = [](const char *function_name, void *user_data) -> PFN_vkVoidFunction {
                VkInstance instance = static_cast<VkInstance>(user_data);
                static vk::LibraryLoader loader;
                return loader.sym(instance, function_name);
            };
            if (!ImGui_ImplVulkan_LoadFunctions(DxvkVulkanApiVersion, loader, device->instance()->handle())) {
                Logger::err("[imgui] Failed to load required functions!");
                return;
            }
            Logger::info("[imgui] Load ImGui library success");
        }

        // Create imgui context
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            m_io = &ImGui::GetIO();
            m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        }

        // Create imgui descriptor pool
        {
            auto vkd = device->vkd();

            VkDescriptorPoolSize poolSizes[] = {
                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, IMGUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE},
                {VK_DESCRIPTOR_TYPE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_SAMPLER_POOL_SIZE},
            };
            VkDescriptorPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            poolInfo.maxSets = 0;
            for (VkDescriptorPoolSize &poolSize: poolSizes)
                poolInfo.maxSets += poolSize.descriptorCount;
            poolInfo.poolSizeCount = (uint32_t) IM_COUNTOF(poolSizes);
            poolInfo.pPoolSizes = poolSizes;
            VkResult result = vkd->vkCreateDescriptorPool(
                vkd->device(),
                &poolInfo,
                nullptr,
                &m_descriptorPool
            );
            checkVkResult(result);
        }

        // Initial imgui backend
        {
            auto queues = device->queues();

            // The win32 backend
            ImGui_ImplWin32_Init(m_hwnd);

            // The vulkan backend
            ImGui_ImplVulkan_InitInfo initInfo{};
            initInfo.Instance = device->instance()->handle();
            initInfo.PhysicalDevice = device->adapter()->handle();
            initInfo.Device = device->vkd()->device();
            initInfo.QueueFamily = queues.graphics.queueFamily;
            initInfo.Queue = queues.graphics.queueHandle;
            initInfo.DescriptorPool = m_descriptorPool;
            initInfo.ImageCount = 2;
            initInfo.MinImageCount = 2;
            initInfo.UseDynamicRendering = true;
            initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR
            };
            initInfo.CheckVkResultFn = checkVkResult;
            ImGui_ImplVulkan_Init(&initInfo);
        }

        m_initialized = true;
        Logger::info("[imgui] Initialized ImGui");
    }

    void DxvkImgui::onSwapChainCreate(VkFormat format, VkColorSpaceKHR colorSpace, uint32_t imageCount) {
    }

    void DxvkImgui::onSwapChainDestroy() {
    }

    void DxvkImgui::newFrame() {
    }

    void DxvkImgui::render(const Rc<DxvkCommandList> &ctx, const Rc<DxvkImageView> &dstView) {
    }

    bool DxvkImgui::wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        return true;
    }

    void DxvkImgui::destroy() {
        Logger::info("[imgui] Destroyed ImGui");
    }
}
