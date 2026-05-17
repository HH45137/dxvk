#pragma once

#include "imgui.h"
#include "imgui_impl_win32.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "imgui_impl_vulkan.h"
#include "imgui/imgui.h"

namespace dxvk {
    class DxvkImgui {
    public:
        static void init();

        static void destroy();

    private:
        static ImGuiIO *io;
    };
}
