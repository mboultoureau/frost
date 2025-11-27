#include "Editor/UI/EditorTheme.h"

#include <imgui.h>

namespace Editor
{
    void EditorTheme::Apply()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Geometry and Borders
        style.WindowRounding = 4.0f;
        style.ChildRounding = 4.0f;
        style.FrameRounding = 3.0f;
        style.GrabRounding = 3.0f;
        style.PopupRounding = 3.0f;
        style.ScrollbarRounding = 9.0f;
        style.TabRounding = 4.0f;

        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.TabBorderSize = 0.0f;

        // Spacing
        style.WindowPadding = ImVec2(8.0f, 8.0f);
        style.FramePadding = ImVec2(6.0f, 4.0f);
        style.ItemSpacing = ImVec2(8.0f, 4.0f);
        style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
        style.ScrollbarSize = 12.0f;
        style.GrabMinSize = 10.0f;

        // Colors
        ImVec4 color_bg_main = ImVec4(0.117f, 0.117f, 0.117f, 1.00f);  // #1E1E1E (Base)
        ImVec4 color_bg_panel = ImVec4(0.176f, 0.176f, 0.176f, 1.00f); // #2D2D2D (Panel)
        ImVec4 color_bg_input = ImVec4(0.090f, 0.090f, 0.090f, 1.00f); // #171717 (Input Fields)

        // Buttons
        ImVec4 color_button = ImVec4(0.220f, 0.220f, 0.220f, 1.00f);     // #383838
        ImVec4 color_button_hov = ImVec4(0.260f, 0.260f, 0.260f, 1.00f); // #424242
        ImVec4 color_button_act = ImVec4(0.300f, 0.300f, 0.300f, 1.00f); // #4D4D4D

        // Accent Colors
        ImVec4 color_accent = ImVec4(0.278f, 0.549f, 0.749f, 1.00f); // #478CBF
        ImVec4 color_accent_hov = ImVec4(0.350f, 0.600f, 0.800f, 1.00f);
        ImVec4 color_accent_act = ImVec4(0.400f, 0.650f, 0.850f, 1.00f);

        // Borders and Text
        ImVec4 color_border = ImVec4(0.000f, 0.000f, 0.000f, 0.30f);
        ImVec4 color_text = ImVec4(0.900f, 0.900f, 0.900f, 1.00f);
        ImVec4 color_text_dis = ImVec4(0.500f, 0.500f, 0.500f, 1.00f);

        colors[ImGuiCol_Text] = color_text;
        colors[ImGuiCol_TextDisabled] = color_text_dis;

        // Windows and Panels
        colors[ImGuiCol_WindowBg] = color_bg_main; // #1E1E1E
        colors[ImGuiCol_ChildBg] = color_bg_panel; // #2D2D2D
        colors[ImGuiCol_PopupBg] = ImVec4(0.140f, 0.140f, 0.140f, 1.00f);
        colors[ImGuiCol_Border] = color_border;
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

        // Inputs and Fields
        colors[ImGuiCol_FrameBg] = color_bg_input;
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.130f, 0.130f, 0.130f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.150f, 0.150f, 0.150f, 1.00f);

        // Titles
        colors[ImGuiCol_TitleBg] = color_bg_main;
        colors[ImGuiCol_TitleBgActive] = color_bg_panel;
        colors[ImGuiCol_TitleBgCollapsed] = color_bg_main;

        // MenuBar
        colors[ImGuiCol_MenuBarBg] = color_bg_input;

        // Scrollbar
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.40f, 0.40f, 0.60f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);

        // Checkbox and Sliders
        colors[ImGuiCol_CheckMark] = color_accent;
        colors[ImGuiCol_SliderGrab] = color_accent;
        colors[ImGuiCol_SliderGrabActive] = color_accent_act;

        // Buttons
        colors[ImGuiCol_Button] = color_button;
        colors[ImGuiCol_ButtonHovered] = color_button_hov;
        colors[ImGuiCol_ButtonActive] = color_button_act;

        // Headers (Selectables, Collapsing Headers)
        colors[ImGuiCol_Header] = color_button;
        colors[ImGuiCol_HeaderHovered] = color_button_hov;
        colors[ImGuiCol_HeaderActive] = color_accent;

        // Separators
        colors[ImGuiCol_Separator] = color_border;
        colors[ImGuiCol_SeparatorHovered] = color_accent;
        colors[ImGuiCol_SeparatorActive] = color_accent_act;

        // Resize Grip
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4(0.150f, 0.150f, 0.150f, 1.00f);
        colors[ImGuiCol_TabHovered] = color_button_hov;
        colors[ImGuiCol_TabActive] = color_bg_panel;
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.150f, 0.150f, 0.150f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = color_bg_panel;

        // Plotting / Graphs
        colors[ImGuiCol_PlotLines] = color_accent;
        colors[ImGuiCol_PlotLinesHovered] = color_accent_hov;
        colors[ImGuiCol_PlotHistogram] = color_accent;
        colors[ImGuiCol_PlotHistogramHovered] = color_accent_hov;

        // Tables
        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 0.50f);
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 0.50f);
        colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.04f);

        // Drag-drop
        colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);

        // Navigation
        colors[ImGuiCol_NavHighlight] = color_accent;
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    }
} // namespace Editor
