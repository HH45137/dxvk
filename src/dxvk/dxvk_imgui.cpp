#include "dxvk_imgui.h"

#include "../util/log/log.h"
#include "../vulkan/vulkan_loader.h"

#include "imgui_impl_win32.h"
#include "imgui_impl_vulkan.h"

namespace dxvk {
    static void ImguiLoging(const std::string &str) {
        Logger::log(LogLevel::Debug, "[imgui] " + str);
    }

    static void checkVkResult(VkResult err) {
        if (err == VK_SUCCESS)
            return;
        ImguiLoging("Error: VkResult =" + std::to_string(err));
    }

    bool DxvkImgui::m_initialized = false;
    Rc<DxvkDevice> DxvkImgui::m_device = nullptr;
    VkFormat DxvkImgui::m_colorFormats[DxvkLimits::MaxNumRenderTargets] = {};
    uint32_t DxvkImgui::m_colorAttachmentCount = 0;
    uint32_t DxvkImgui::m_imageCount = 0;
    bool DxvkImgui::m_useDynamicRendering = false;
    ImGuiIO *DxvkImgui::m_io = nullptr;
    VkDescriptorPool DxvkImgui::m_descriptorPool = VK_NULL_HANDLE;
    HWND DxvkImgui::m_hwnd = nullptr;

    void DxvkImgui::init(const Rc<DxvkDevice> &device) {
        m_device = device;

        {
            ImguiLoging("Start loading ImGui library");
            auto loader = [](const char *function_name, void *user_data) -> PFN_vkVoidFunction {
                VkInstance instance = static_cast<VkInstance>(user_data);
                static vk::LibraryLoader loader;
                return loader.sym(instance, function_name);
            };
            if (!ImGui_ImplVulkan_LoadFunctions(DxvkVulkanApiVersion, loader, device->instance()->handle())) {
                ImguiLoging("Failed to load required functions!");
                return;
            }
            ImguiLoging("Load ImGui library success");
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

        // Vulkan backend will be initialized in onSwapChainCreate
        // when we have the actual swapchain format information

        ImguiLoging("[Vulkan] Initialized ImGui");
    }

    void DxvkImgui::initWin32(HWND hwnd) {
        if (hwnd == nullptr) {
            ImguiLoging("[Win32] Please set the hwnd first!");
            return;
        }
        m_hwnd = hwnd;

        // The win32 backend
        ImGui_ImplWin32_Init(m_hwnd);
        ImguiLoging("[Win32] Initialized ImGui");
    }

    void DxvkImgui::initVulkanBackend() {
        if (!m_device)
            return;

        VkPipelineRenderingCreateInfo pipelineCI = {VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR};
        pipelineCI.colorAttachmentCount = m_colorAttachmentCount;
        pipelineCI.pColorAttachmentFormats = m_colorFormats;

        auto queues = m_device->queues();

        ImguiLoging("Initializing Vulkan backend with ImageCount=" + std::to_string(m_imageCount)
                    + ", colorFormat=" + std::to_string(m_colorFormats[0]));

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.Instance = m_device->instance()->handle();
        initInfo.PhysicalDevice = m_device->adapter()->handle();
        initInfo.Device = m_device->vkd()->device();
        initInfo.QueueFamily = queues.graphics.queueFamily;
        initInfo.Queue = queues.graphics.queueHandle;
        initInfo.DescriptorPool = m_descriptorPool;
        initInfo.ImageCount = m_imageCount;
        initInfo.MinImageCount = m_imageCount;
        initInfo.UseDynamicRendering = true;
        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineCI;
        initInfo.CheckVkResultFn = checkVkResult;
        bool initOk = ImGui_ImplVulkan_Init(&initInfo);
        ImguiLoging("ImGui_ImplVulkan_Init returned: " + std::to_string(initOk));

        m_initialized = true;
        ImguiLoging("Vulkan backend initialized successfully");
    }

    void DxvkImgui::destroyVulkanBackend() {
        if (m_initialized) {
            ImGui_ImplVulkan_Shutdown();
            m_initialized = false;
            ImguiLoging("Vulkan backend destroyed");
        }
    }

    void DxvkImgui::onSwapChainCreate(VkFormat format, VkColorSpaceKHR colorSpace, uint32_t imageCount) {
        // If Vulkan backend was already initialized (swapchain recreation), destroy it first
        destroyVulkanBackend();

        // Store the color format locally to avoid dangling pointers
        m_colorFormats[0] = format;
        m_colorAttachmentCount = 1;
        m_imageCount = imageCount;

        ImguiLoging(
            "SwapChain created: format=" + std::to_string(format) +
            ", imageCount=" + std::to_string(imageCount)
        );

        // Now initialize the Vulkan backend with the correct format
        initVulkanBackend();
    }

    void DxvkImgui::onSwapChainDestroy() {
        destroyVulkanBackend();
    }

    void DxvkImgui::newFrame() {
        ImguiLoging("Begin ImGui newFrame");

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        static bool isShowDemoWindow = true;
        ImGui::ShowDemoWindow(&isShowDemoWindow);

        ImGui::Begin("DXVK Debug");
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

        ImGui::End();

        ImGui::Render();

        ImguiLoging("End ImGui newFrame");
    }

    void DxvkImgui::render(const Rc<DxvkCommandList> &ctx, const Rc<DxvkImageView> &dstView) {
        ImDrawData *drawData = ImGui::GetDrawData();
        if (!drawData || drawData->DisplaySize.x <= 0 || drawData->DisplaySize.y <= 0) {
            Logger::log(LogLevel::Debug, "Render skipped: no draw data");
            return;
        }

        ImGui_ImplVulkan_RenderDrawData(drawData, ctx->getCmdBuffer());
    }

    bool DxvkImgui::wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        return true;
    }

    void DxvkImgui::destroy() {
        ImguiLoging("Destroyed ImGui");
    }
}
