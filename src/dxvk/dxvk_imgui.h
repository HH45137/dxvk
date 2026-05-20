#pragma once

#include "dxvk_device.h"
#include "dxvk_limits.h"

#include "imgui.h"

namespace dxvk {
    class DxvkImgui {
    public:
        static void init(const Rc<DxvkDevice> &device);

        static void initWin32(HWND hwnd);

        static void onSwapChainCreate(
            VkFormat format,
            VkColorSpaceKHR colorSpace,
            uint32_t imageCount
        );

        static void onSwapChainDestroy();

        static void newFrame();

        static void render(
            const Rc<DxvkCommandList> &ctx,
            const Rc<DxvkImageView> &dstView
        );

        static bool wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

        static void destroy();

        static bool m_initialized;

    private:
        static void initVulkanBackend();

        static void destroyVulkanBackend();

        static Rc<DxvkDevice> m_device;
        static VkFormat m_colorFormats[DxvkLimits::MaxNumRenderTargets];
        static uint32_t m_colorAttachmentCount;
        static uint32_t m_imageCount;
        static HWND m_hwnd;
        static bool m_useDynamicRendering;
        static ImGuiIO *m_io;
        static VkDescriptorPool m_descriptorPool;
    };
}
