#include "dxvk_imgui.h"

#include "../util/log/log.h"

#include "imgui_impl_win32.h"
#include "imgui_impl_vulkan.h"

namespace dxvk {
    static void checkVkResult(VkResult err) {
        if (err == VK_SUCCESS)
            return;
        Logger::info("[vulkan] Error: VkResult =" + std::to_string(err));
    }

    bool DxvkImgui::m_initialized = false;
    bool DxvkImgui::m_useDynamicRendering = false;
    ImGuiIO *DxvkImgui::m_io = nullptr;
    VkDescriptorPool DxvkImgui::m_descriptorPool = nullptr;
    HWND DxvkImgui::m_hwnd = nullptr;

    void DxvkImgui::init(const Rc<DxvkDevice> &device, HWND hwnd) {
        if (m_hwnd == nullptr) {
            Logger::err("Please set the hwnd first!");
            return;
        }
        m_hwnd = hwnd;

        ImGui_ImplVulkan_LoadFunctions(
            DxvkVulkanApiVersion,
            [](const char *function_name, void *) -> PFN_vkVoidFunction {
                static vk::LibraryLoader loader;
                return loader.sym(function_name);
            },
            nullptr
        );

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
            initInfo.UseDynamicRendering = true;
            initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR
            };
            initInfo.CheckVkResultFn = checkVkResult;
            ImGui_ImplVulkan_Init(&initInfo);
        }

        m_initialized = true;
        Logger::info("Initialized ImGui");
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
        Logger::info("Destroyed ImGui");
    }
}
