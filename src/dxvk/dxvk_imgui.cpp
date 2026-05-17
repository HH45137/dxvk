#include "dxvk_imgui.h"

namespace dxvk {
    ImGuiIO *DxvkImgui::io = nullptr;

    void DxvkImgui::init() {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        io = &ImGui::GetIO();
        (void *) io;
        io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
    }

    void DxvkImgui::destroy() {
    }
}
