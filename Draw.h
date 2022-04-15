#include "imgui.h"
#include <string>

static void DrawStrokeText(float Screen_x, float Screen_y, float red, float green, float blue, float Alpha, const char* str)
{
	ImFont a;
	std::string utf_8_1 = std::string(str);
	ImGui::GetOverlayDrawList()->AddText(ImVec2(Screen_x, Screen_y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_1.c_str());
	ImGui::GetOverlayDrawList()->AddText(ImVec2(Screen_x, Screen_y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_1.c_str());
	ImGui::GetOverlayDrawList()->AddText(ImVec2(Screen_x - 1, Screen_y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_1.c_str());
	ImGui::GetOverlayDrawList()->AddText(ImVec2(Screen_x + 1, Screen_y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_1.c_str());
	ImGui::GetOverlayDrawList()->AddText(ImVec2(Screen_x, Screen_y), ImGui::ColorConvertFloat4ToU32(ImVec4(red / 255.0, green / 255.0, blue / 255.0, Alpha / 255.0)), utf_8_1.c_str());
}

static void DrawNewText(float x, float y, float red, float green, float blue, float Alpha, const char* str)
{
	std::string utf_8_1 = std::string(str);
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(red / 255.0, green / 255.0, blue / 255.0, Alpha / 255.0)), utf_8_1.c_str());
}

static void DrawRect(float x, float y, int w, int h, float red, float green, float blue, float Alpha, int thickness)
{
	ImGui::GetOverlayDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(red / 255.0, green / 255.0, blue / 255.0, Alpha / 255.0)), 0, 0, thickness);
}

static void DrawLine(int x1, int y1, int x2, int y2, float red, float green, float blue, float Alpha, int thickness)
{
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::ColorConvertFloat4ToU32(ImVec4(red / 255.0, green / 255.0, blue / 255.0, Alpha / 255.0)), thickness);
}

static void DrawRect(int x, int y, int w, int h, float red, float green, float blue, float Alpha, int thickness)
{
	ImGui::GetOverlayDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(red / 255.0, green / 255.0, blue / 255.0, Alpha / 255.0)), 0, 0, thickness);
}