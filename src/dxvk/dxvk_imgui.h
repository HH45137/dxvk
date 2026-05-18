#pragma once

#include "dxvk_device.h"

#include "imgui.h"

namespace dxvk {
    class DxvkImgui {
    public:
        static void init(const Rc<DxvkDevice> &device, HWND hwnd);

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
        static HWND m_hwnd;
        static bool m_useDynamicRendering;
        static ImGuiIO *m_io;
        static VkDescriptorPool m_descriptorPool;
    };
}
