#pragma once

#include "imgui.h"
#include "imgui_impl_win32.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "dxvk_device.h"
#include "imgui_impl_vulkan.h"
#include "imgui/imgui.h"

namespace dxvk {
    class DxvkImgui {
    public:
        static void init(
            const Rc<DxvkDevice> &device,
            HWND hwnd,
            uint32_t width
        );

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

    private:
        static bool m_initialized;
        static bool m_useDynamicRendering;
        static ImGuiIO *m_io;
        static VkDescriptorPool m_descriptorPool;
    };
}
