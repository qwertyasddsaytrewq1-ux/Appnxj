#pragma once
#include "inc/custom_theme.h"

void StyleColorsCustom(ImGuiStyle* _style) {
    ImGuiStyle& style = _style ? *_style : ImGui::GetStyle();
    style = ImGuiStyle();
    style.WindowRounding = 18.0f;
    style.ChildRounding = 14.0f;
    style.FrameRounding = 10.0f;
    style.PopupRounding = 14.0f;
    style.ScrollbarRounding = 10.0f;
    style.GrabRounding = 10.0f;
    style.TabRounding = 10.0f;
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(10, 7);
    style.ItemSpacing = ImVec2(8, 8);
    style.ScrollbarSize = 13.0f;
    style.Colors[ImGuiCol_Text]                 = ImVec4(0.22f,0.13f,0.05f,1.00f);
    style.Colors[ImGuiCol_TextDisabled]         = ImVec4(0.48f,0.35f,0.18f,1.00f);
    style.Colors[ImGuiCol_WindowBg]             = ImVec4(0.88f,0.78f,0.58f,1.00f);
    style.Colors[ImGuiCol_ChildBg]              = ImVec4(0.92f,0.84f,0.67f,1.00f);
    style.Colors[ImGuiCol_PopupBg]              = ImVec4(0.86f,0.73f,0.48f,1.00f);
    style.Colors[ImGuiCol_Border]               = ImVec4(0.55f,0.35f,0.10f,0.75f);
    style.Colors[ImGuiCol_BorderShadow]         = ImVec4(0.18f,0.10f,0.03f,0.25f);
    style.Colors[ImGuiCol_FrameBg]              = ImVec4(0.55f,0.38f,0.16f,1.00f);
    style.Colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.67f,0.48f,0.22f,1.00f);
    style.Colors[ImGuiCol_FrameBgActive]        = ImVec4(0.75f,0.54f,0.25f,1.00f);
    style.Colors[ImGuiCol_TitleBg]              = ImVec4(0.47f,0.28f,0.07f,1.00f);
    style.Colors[ImGuiCol_TitleBgActive]        = ImVec4(0.55f,0.34f,0.10f,1.00f);
    style.Colors[ImGuiCol_MenuBarBg]            = ImVec4(0.60f,0.40f,0.13f,1.00f);
    style.Colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.50f,0.32f,0.10f,0.45f);
    style.Colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.75f,0.52f,0.18f,1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.88f,0.65f,0.25f,1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.65f,0.40f,0.10f,1.00f);
    style.Colors[ImGuiCol_CheckMark]            = ImVec4(1.00f,0.96f,0.82f,1.00f);
    style.Colors[ImGuiCol_SliderGrab]           = ImVec4(0.83f,0.55f,0.10f,1.00f);
    style.Colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.96f,0.70f,0.20f,1.00f);
    style.Colors[ImGuiCol_Button]               = ImVec4(0.57f,0.38f,0.12f,1.00f);
    style.Colors[ImGuiCol_ButtonHovered]        = ImVec4(0.72f,0.50f,0.18f,1.00f);
    style.Colors[ImGuiCol_ButtonActive]         = ImVec4(0.45f,0.27f,0.07f,1.00f);
    style.Colors[ImGuiCol_Header]               = ImVec4(0.70f,0.48f,0.15f,0.35f);
    style.Colors[ImGuiCol_HeaderHovered]        = ImVec4(0.78f,0.56f,0.22f,0.55f);
    style.Colors[ImGuiCol_HeaderActive]         = ImVec4(0.65f,0.41f,0.10f,0.70f);
    style.Colors[ImGuiCol_Tab]                  = ImVec4(0.52f,0.32f,0.08f,1.00f);
    style.Colors[ImGuiCol_TabHovered]           = ImVec4(0.78f,0.56f,0.22f,1.00f);
    style.Colors[ImGuiCol_TabActive]            = ImVec4(0.70f,0.45f,0.11f,1.00f);
    style.Colors[ImGuiCol_TabUnfocused]         = ImVec4(0.45f,0.27f,0.07f,1.00f);
    style.Colors[ImGuiCol_TabUnfocusedActive]   = ImVec4(0.60f,0.37f,0.09f,1.00f);
    style.Colors[ImGuiCol_Separator]            = ImVec4(0.55f,0.35f,0.10f,0.60f);
    style.Colors[ImGuiCol_SeparatorHovered]     = ImVec4(0.82f,0.58f,0.20f,0.85f);
    style.Colors[ImGuiCol_SeparatorActive]      = ImVec4(0.90f,0.66f,0.25f,1.00f);
    style.Colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.18f,0.10f,0.03f,0.38f);
}
