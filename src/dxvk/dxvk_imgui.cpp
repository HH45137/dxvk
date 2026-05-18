#include "dxvk_imgui.h"
#include "../util/log/log.h"

namespace dxvk {
    ImGuiIO *DxvkImgui::m_io = nullptr;

    void DxvkImgui::init(const Rc<DxvkDevice> &device, HWND hwnd, uint32_t width) {
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
