#pragma once
#include "globals.hpp"
#include "imguipp.h"

using namespace VALORANT;

#define HELPMARKER(str) ImGui::SameLine(); ImGui::TextColored(ImColor(219, 17, 0, 255), "(?)"); if (ImGui::IsItemHovered()) ImGui::SetTooltip(str)

namespace hotkeys
{
	int aimkey = VK_SHIFT;
	int airstuckey;
	int instares;
}

static int keystatus = 0;
static int realkey = 0;

bool GetKey(int key)
{
	realkey = key;
	return true;
}
void ChangeKey(void* blank)
{
	keystatus = 1;
	while (true)
	{
		for (int i = 0; i < 0x87; i++)
		{
			if (GetKeyState(i) & 0x8000)
			{
				hotkeys::aimkey = i;
				keystatus = 0;
				return;
			}
		}
	}
}

static const char* keyNames[] =
{
	skCrypt("Press any key",
	"Left Mouse",
	"Right Mouse",
	"Cancel",
	"Middle Mouse",
	"Mouse 5",
	"Mouse 4",
	"",
	"Backspace",
	"Tab",
	"",
	"",
	"Clear",
	"Enter",
	"",
	"",
	"Shift",
	"Control",
	"Alt",
	"Pause",
	"Caps",
	"",
	"",
	"",
	"",
	"",
	"",
	"Escape",
	"",
	"",
	"",
	"",
	"Space",
	"Page Up",
	"Page Down",
	"End",
	"Home",
	"Left",
	"Up",
	"Right",
	"Down",
	"",
	"",
	"",
	"Print",
	"Insert",
	"Delete",
	"",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"",
	"",
	"",
	"",
	"",
	"Numpad 0",
	"Numpad 1",
	"Numpad 2",
	"Numpad 3",
	"Numpad 4",
	"Numpad 5",
	"Numpad 6",
	"Numpad 7",
	"Numpad 8",
	"Numpad 9",
	"Multiply",
	"Add",
	"",
	"Subtract",
	"Decimal",
	"Divide",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",)
};

static bool Items_ArrayGetter(void* data, int idx, const char** out_text)
{
	const char* const* items = (const char* const*)data;
	if (out_text)
		*out_text = items[idx];
	return true;
}

inline void HotkeyButton(int aimkey, void* changekey, int status)
{
	const char* preview_value = NULL;
	if (aimkey >= 0 && aimkey < IM_ARRAYSIZE(keyNames))
		Items_ArrayGetter(keyNames, aimkey, &preview_value);

	std::string aimkeys;
	if (preview_value == NULL)
		aimkeys = skCrypt("Select Key");
	else
		aimkeys = preview_value;

	if (status == 1)
	{
		aimkeys = skCrypt("Press the Key");
	}
	if (ImGui::Button(aimkeys.c_str(), ImVec2(120, 19)))
	{
		if (status == 0)
		{
			CreateThread(0, 0, (LPTHREAD_START_ROUTINE)changekey, nullptr, 0, nullptr);
		}
	}
}

bool bMenu = true;
static bool Toggledsdsd= true;
static int Tabs = 0;
void DrawMenu()
{
	if (GetAsyncKeyState(VK_INSERT) & 1) bMenu = !bMenu;

	if (settings::aimbot_draw_fov) ImGui::GetOverlayDrawList()->AddCircle(ImVec2(center_x, center_y), settings::aimbot_fov, ImGui::GetColorU32({ 1.f, 1.f, 1.f, 1.f }), 64, 1.f);

	if (bMenu)
	{
		ImGui::SetNextWindowSize(ImVec2(500, 300));
		ImGui::Begin(skCrypt("Your shitty p2c"), &Toggledsdsd, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);


		ImGui::Columns(2);
		ImGui::SetColumnOffset(1, 120);

		ImGui::Spacing();
		if (ImGui::Button("Aimbot", ImVec2(100, 40)))
			Tabs = 0;

		ImGui::Spacing();
		if (ImGui::Button("Visuals", ImVec2(100, 40)))
			Tabs = 1;

		ImGui::Spacing();
		if (ImGui::Button("Extras", ImVec2(100, 40)))
			Tabs = 2;

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::SameLine(); // CHEcks

		ImGui::NextColumn();
		{
			if (Tabs == 0)
			{
				HotkeyButton(hotkeys::aimkey, ChangeKey, keystatus); // AIMKEYYY

				ImGui::Checkbox("Aimbot", &settings::aimbot_enable);
				ImGui::Checkbox("Legit Smoothing", &settings::aimbot_constant);
				if (&settings::aimbot_constant)
				{
					ImGui::SliderFloat(" ", &settings::aimbot_smooth, 1.f, 30.f, skCrypt("%.f"));
				}
				ImGui::Checkbox("Draw FOV Circle", &settings::aimbot_draw_fov);
				if (settings::aimbot_draw_fov)
				{
					ImGui::SliderFloat("", &settings::aimbot_fov, 10.f, 1000.f, skCrypt("%.f"));
				}
				ImGui::Checkbox("Draw Aim Line", &settings::aimbot_draw_target_line);

				ImGui::Checkbox("Recoil Control", &settings::aimbot_recoil_control);

				ImGui::Checkbox("Enable 360 Aimbot", &settings::aimbot_360_mode);
				if (settings::aimbot_360_mode) settings::aimbot_fov = 100000000000000000000000000000000000000.f;
				ImGui::EndChild();

				/*ImGui::Checkbox("Target Aim Bone", &settings::aimbot_draw_target_line);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
				ImGui::SetCursorPosX(8);
				ImGui::PushItemWidth(121); ImGui::Combo(skCrypt("##aimbone"), &settings::aimbot_aimbone_selection, settings::aimbot_aimbone_items, sizeof(settings::aimbot_aimbone_items) / sizeof(*settings::aimbot_aimbone_items));
				ImGui::PopStyleVar();

				if (settings::aimbot_aimbone_selection == 0) settings::aimbot_aimbone = 8;
				if (settings::aimbot_aimbone_selection == 1) settings::aimbot_aimbone = 21;
				if (settings::aimbot_aimbone_selection == 2) settings::aimbot_aimbone = 6;*/
			}
			else if (Tabs == 1)
			{
				ImGui::Checkbox("Player Chams", &settings::chams);
				ImGui::Checkbox("Player 2D Box", &settings::player_box);
				ImGui::Checkbox("Player Distance", &settings::player_distance);
				ImGui::Checkbox("Player Snapline", &settings::player_snapline);
				ImGui::Checkbox("Player Skeleton", &settings::player_skeleton);
				ImGui::Checkbox("Player Healthbar", &settings::player_healthbar);

			}
			else if (Tabs == 2)
			{

			}
		}

		ImGui::End();
	}
}