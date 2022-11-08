#pragma once
#include "globals.hpp"
#include "menu.hpp"
#include "renderer.hpp"

using namespace VALORANT;

RGBA espcolor;
ImColor ESPColor;

uintptr_t g_base_address;



struct Enemy
{
	uintptr_t for_actor;
	uintptr_t for_mesh;

	uintptr_t actor;
	uintptr_t mesh;
	uintptr_t bone_array;
	uintptr_t root_component;
	uintptr_t damage_handler;

	INT32 bone_count;
	INT32 ammo_count;

	std::string weapon_name;
	std::string agent_name;
	std::string player_name;

	float distance;
	float health;
	float shield;

	bool is_valid;
	bool is_damage_handler_guarded;
	bool is_mesh_guarded;
};

std::vector<Enemy> player_pawns;

bool operator==(const Enemy& a, const Enemy& b) {
	if (a.actor == b.actor) return true;
	return false;
}

boolean in_rect(double centerX, double centerY, double radius, double x, double y) {
	return x >= centerX - radius && x <= centerX + radius &&
		y >= centerY - radius && y <= centerY + radius;
}

auto getuworld(uintptr_t pointer) -> uintptr_t
{
	uintptr_t uworld_addr = read<uintptr_t>(pointer + offsets::uworld_pointer);

	unsigned long long uworld_offset;

	if (uworld_addr > 0x10000000000)
	{
		uworld_offset = uworld_addr - 0x10000000000;
	}
	else {
		uworld_offset = uworld_addr - 0x8000000000;
	}

	return pointer + uworld_offset;
}

DWORD_PTR GetProcessBaseAddress(DWORD processID)
{
	DWORD_PTR   baseAddress = 0;
	HANDLE      processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
	HMODULE* moduleArray;
	LPBYTE      moduleArrayBytes;
	DWORD       bytesRequired;

	if (processHandle)
	{
		if (EnumProcessModules(processHandle, NULL, 0, &bytesRequired))
		{
			if (bytesRequired)
			{
				moduleArrayBytes = (LPBYTE)LocalAlloc(LPTR, bytesRequired);

				if (moduleArrayBytes)
				{
					unsigned int moduleCount;

					moduleCount = bytesRequired / sizeof(HMODULE);
					moduleArray = (HMODULE*)moduleArrayBytes;

					if (EnumProcessModules(processHandle, moduleArray, bytesRequired, &bytesRequired))
					{
						baseAddress = (DWORD_PTR)moduleArray[0];
					}

					LocalFree(moduleArrayBytes);
				}
			}
		}

		CloseHandle(processHandle);
	}

	return baseAddress;
}

void calculate_random_for_constant_aimbot()
{
	while (true)
	{
		settings::aim_constant_offset = 15.f;
		Sleep(200);

		settings::aim_constant_offset = 2.f;
		Sleep(200);

		settings::aim_constant_offset = 8.f;
		Sleep(200);

		settings::aim_constant_offset = 20.f;
		Sleep(200);

		settings::aim_constant_offset = 4.f;
		Sleep(200);

		settings::aim_constant_offset = 12.f;
		Sleep(200);
	}

}

void normalize(fvector& in)
{
	if (in.x > 89.f) in.x -= 360.f;
	else if (in.x < -89.f) in.x += 360.f;

	while (in.y > 180)in.y -= 360;
	while (in.y < -180)in.y += 360;
	in.z = 0;
}

fvector smooth_aim(fvector target, fvector delta_rotation, float smooth)
{
	fvector diff = target - delta_rotation;
	normalize(diff);
	return delta_rotation + diff / smooth;
}

float degree_to_radian(float degree)
{
	return degree * (M_PI / 180);
}

void angle_rotation(const fvector& angles, fvector* forward)
{
	float	sp, sy, cp, cy;

	sy = sin(degree_to_radian(angles.y));
	cy = cos(degree_to_radian(angles.y));

	sp = sin(degree_to_radian(angles.x));
	cp = cos(degree_to_radian(angles.x));

	forward->x = cp * cy;
	forward->y = cp * sy;
	forward->z = -sp;
}

fvector bone_matrix(int index, Enemy _player)
{
	size_t size = sizeof(ftransform);
	ftransform first_bone, comp_to_world;

	first_bone = read<ftransform>(_player.bone_array + (size * index));
	if (_player.is_mesh_guarded)
	{
		comp_to_world = read<ftransform>(virtualaddy + _player.mesh + 0x250);
	}
	else
	{
		comp_to_world = read<ftransform>(_player.mesh + 0x250);
	}

	D3DMATRIX matrix = MatrixMultiplication(first_bone.ToMatrixWithScale(), comp_to_world.ToMatrixWithScale());
	return fvector(matrix._41, matrix._42, matrix._43);
}

fvector W2S(fvector world_location)
{
	fvector ScreenLocation;
	D3DMATRIX tempMatrix = matrix(camera::rotation);
	fvector vAxisX, vAxisY, vAxisZ;

	vAxisX = fvector(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	vAxisY = fvector(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	vAxisZ = fvector(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

	fvector vDelta = world_location - camera::location;
	fvector vTransformed = fvector(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	if (vTransformed.z < .1f)
		vTransformed.z = .1f;

	float ScreenCenterX = center_x;
	float ScreenCenterY = center_y;

	ScreenLocation.x = ScreenCenterX + vTransformed.x * (ScreenCenterX / tanf(camera::fov * (float)m_pi / 360.f)) / vTransformed.z;
	ScreenLocation.y = ScreenCenterY - vTransformed.y * (ScreenCenterX / tanf(camera::fov * (float)m_pi / 360.f)) / vTransformed.z;
	return ScreenLocation;
}

void draw_3d_box(fvector base, fvector top_reach, float wide, ImVec4 col, float thickness)
{
	//calculate bottom rect
	fvector bottom_rect_1 = fvector(base.x + wide, base.y + wide, base.z);
	fvector bottom_rect_2 = fvector(base.x + wide, base.y - wide, base.z);
	fvector bottom_rect_3 = fvector(base.x - wide, base.y + wide, base.z);
	fvector bottom_rect_4 = fvector(base.x - wide, base.y - wide, base.z);

	//calculate top rect
	fvector top_rect_1 = fvector(top_reach.x + wide, top_reach.y + wide, top_reach.z);
	fvector top_rect_2 = fvector(top_reach.x + wide, top_reach.y - wide, top_reach.z);
	fvector top_rect_3 = fvector(top_reach.x - wide, top_reach.y + wide, top_reach.z);
	fvector top_rect_4 = fvector(top_reach.x - wide, top_reach.y - wide, top_reach.z);

	//W2S bottom rect
	bottom_rect_1 = W2S(bottom_rect_1);
	bottom_rect_2 = W2S(bottom_rect_2);
	bottom_rect_3 = W2S(bottom_rect_3);
	bottom_rect_4 = W2S(bottom_rect_4);

	//W2S top rect
	top_rect_1 = W2S(top_rect_1);
	top_rect_2 = W2S(top_rect_2);
	top_rect_3 = W2S(top_rect_3);
	top_rect_4 = W2S(top_rect_4);

	//render bottom rect
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom_rect_1.x, bottom_rect_1.y), ImVec2(bottom_rect_2.x, bottom_rect_2.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom_rect_2.x, bottom_rect_2.y), ImVec2(bottom_rect_4.x, bottom_rect_4.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom_rect_4.x, bottom_rect_4.y), ImVec2(bottom_rect_3.x, bottom_rect_3.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom_rect_3.x, bottom_rect_3.y), ImVec2(bottom_rect_1.x, bottom_rect_1.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);

	//render top rect
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(top_rect_1.x, top_rect_1.y), ImVec2(top_rect_2.x, top_rect_2.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(top_rect_2.x, top_rect_2.y), ImVec2(top_rect_4.x, top_rect_4.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(top_rect_4.x, top_rect_4.y), ImVec2(top_rect_3.x, top_rect_3.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(top_rect_3.x, top_rect_3.y), ImVec2(top_rect_1.x, top_rect_1.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);

	//render connection lines
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom_rect_1.x, bottom_rect_1.y), ImVec2(top_rect_1.x, top_rect_1.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom_rect_2.x, bottom_rect_2.y), ImVec2(top_rect_2.x, top_rect_2.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom_rect_3.x, bottom_rect_3.y), ImVec2(top_rect_3.x, top_rect_3.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom_rect_4.x, bottom_rect_4.y), ImVec2(top_rect_4.x, top_rect_4.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
}

void DrawFilledRect3(int x, int y, int w, int h, RGBA* color)
{
	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), 0, 0);
}

void DrawCornerBox(int x, int y, int w, int h, int borderPx, RGBA* color)
{
	DrawFilledRect3(x + borderPx, y, w / 3, borderPx, color);
	DrawFilledRect3(x + w - w / 3 + borderPx, y, w / 3, borderPx, color);
	DrawFilledRect3(x, y, borderPx, h / 3, color);
	DrawFilledRect3(x, y + h - h / 3 + borderPx * 2, borderPx, h / 3, color);
	DrawFilledRect3(x + borderPx, y + h + borderPx, w / 3, borderPx, color);
	DrawFilledRect3(x + w - w / 3 + borderPx, y + h + borderPx, w / 3, borderPx, color);
	DrawFilledRect3(x + w + borderPx, y, borderPx, h / 3, color);
	DrawFilledRect3(x + w + borderPx, y + h - h / 3 + borderPx * 2, borderPx, h / 3, color);
}

void DrawCircle(ImVec2 Center, int radius, RGBA* color, float segments, float thickness)
{
	ImGui::GetOverlayDrawList()->AddCircle(Center, radius, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 153.0, color->B / 51.0, color->A / 255.0)), segments, thickness);
}

void draw_skeleton(Enemy _player, ImVec4 col, float thickness)
{
	fvector bone_head, bone_neck, bone_chest, bone_pelvis, bone_rshoulder, bone_relbow, bone_rhand, bone_rthigh, bone_rknee, bone_rfoot, bone_lshoulder, bone_lelbow, bone_lhand, bone_lthigh, bone_lknee, bone_lfoot;
	bone_head = bone_matrix(8, _player);
	bone_chest = bone_matrix(6, _player);
	bone_pelvis = bone_matrix(3, _player);

	if (_player.bone_count == 103) 
	{
		bone_neck = bone_matrix(9, _player);

		bone_lshoulder = bone_matrix(33, _player);
		bone_lelbow = bone_matrix(30, _player);
		bone_lhand = bone_matrix(32, _player);

		bone_rshoulder = bone_matrix(58, _player);
		bone_relbow = bone_matrix(55, _player);
		bone_rhand = bone_matrix(57, _player);

		bone_lthigh = bone_matrix(63, _player);
		bone_lknee = bone_matrix(65, _player);
		bone_lfoot = bone_matrix(69, _player);

		bone_rthigh = bone_matrix(77, _player);
		bone_rknee = bone_matrix(79, _player);
		bone_rfoot = bone_matrix(83, _player);
	}
	else if (_player.bone_count == 104) 
	{
		bone_neck = bone_matrix(21, _player);

		bone_lshoulder = bone_matrix(23, _player);
		bone_lelbow = bone_matrix(24, _player);
		bone_lhand = bone_matrix(25, _player);

		bone_rshoulder = bone_matrix(49, _player);
		bone_relbow = bone_matrix(50, _player);
		bone_rhand = bone_matrix(51, _player);

		bone_lthigh = bone_matrix(77, _player);
		bone_lknee = bone_matrix(78, _player);
		bone_lfoot = bone_matrix(80, _player);

		bone_rthigh = bone_matrix(84, _player);
		bone_rknee = bone_matrix(85, _player);
		bone_rfoot = bone_matrix(87, _player);
	}
	else if (_player.bone_count == 101) 
	{
		bone_neck = bone_matrix(21, _player);

		bone_lshoulder = bone_matrix(23, _player);
		bone_lelbow = bone_matrix(24, _player);
		bone_lhand = bone_matrix(25, _player);

		bone_rshoulder = bone_matrix(49, _player);
		bone_relbow = bone_matrix(50, _player);
		bone_rhand = bone_matrix(51, _player);

		bone_lthigh = bone_matrix(75, _player);
		bone_lknee = bone_matrix(76, _player);
		bone_lfoot = bone_matrix(78, _player);

		bone_rthigh = bone_matrix(82, _player);
		bone_rknee = bone_matrix(83, _player);
		bone_rfoot = bone_matrix(85, _player);
	}
	else
	{
		return;
	}

	bone_head = W2S(bone_head);
	bone_neck = W2S(bone_neck);
	bone_chest = W2S(bone_chest);
	bone_pelvis = W2S(bone_pelvis);
	bone_lshoulder = W2S(bone_lshoulder);
	bone_lelbow = W2S(bone_lelbow);
	bone_lhand = W2S(bone_lhand);
	bone_rshoulder = W2S(bone_rshoulder);
	bone_relbow = W2S(bone_relbow);
	bone_rhand = W2S(bone_rhand);
	bone_lthigh = W2S(bone_lthigh);
	bone_lknee = W2S(bone_lknee);
	bone_lfoot = W2S(bone_lfoot);
	bone_rthigh = W2S(bone_rthigh);
	bone_rknee = W2S(bone_rknee);
	bone_rfoot = W2S(bone_rfoot);

	ImDrawList* draw = ImGui::GetOverlayDrawList();

	//top stuff
	//draw->AddLine(ImVec2(bone_head.x, bone_head.y), ImVec2(bone_neck.x, bone_neck.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	draw->AddLine(ImVec2(bone_neck.x, bone_neck.y), ImVec2(bone_chest.x, bone_chest.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	draw->AddLine(ImVec2(bone_chest.x, bone_chest.y), ImVec2(bone_pelvis.x, bone_pelvis.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);

	//right arm
	draw->AddLine(ImVec2(bone_chest.x, bone_chest.y), ImVec2(bone_rshoulder.x, bone_rshoulder.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	draw->AddLine(ImVec2(bone_rshoulder.x, bone_rshoulder.y), ImVec2(bone_relbow.x, bone_relbow.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	draw->AddLine(ImVec2(bone_relbow.x, bone_relbow.y), ImVec2(bone_rhand.x, bone_rhand.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);

	//left arm
	draw->AddLine(ImVec2(bone_chest.x, bone_chest.y), ImVec2(bone_lshoulder.x, bone_lshoulder.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	draw->AddLine(ImVec2(bone_lshoulder.x, bone_lshoulder.y), ImVec2(bone_lelbow.x, bone_lelbow.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	draw->AddLine(ImVec2(bone_lelbow.x, bone_lelbow.y), ImVec2(bone_lhand.x, bone_lhand.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);

	//right foot
	draw->AddLine(ImVec2(bone_pelvis.x, bone_pelvis.y), ImVec2(bone_rthigh.x, bone_rthigh.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	draw->AddLine(ImVec2(bone_rthigh.x, bone_rthigh.y), ImVec2(bone_rknee.x, bone_rknee.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	draw->AddLine(ImVec2(bone_rknee.x, bone_rknee.y), ImVec2(bone_rfoot.x, bone_rfoot.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);

	//left foot
	draw->AddLine(ImVec2(bone_pelvis.x, bone_pelvis.y), ImVec2(bone_lthigh.x, bone_lthigh.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	draw->AddLine(ImVec2(bone_lthigh.x, bone_lthigh.y), ImVec2(bone_lknee.x, bone_lknee.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
	draw->AddLine(ImVec2(bone_lknee.x, bone_lknee.y), ImVec2(bone_lfoot.x, bone_lfoot.y), ImGui::GetColorU32({ col.x, col.y, col.z, col.w }), thickness);
}

void draw_health_bar(ImVec2 min, ImVec2 max, float health)
{
	float health_percentage = health;
	health_percentage *= 0.01f;

	float lenght_left_to_right = max.x - min.x;
	lenght_left_to_right *= health_percentage;

	float healthbar_size_y = 5.f;

	float g = health_percentage * 255.f;
	float r = 255.f - g;
	float b = 0.f;

	r /= 255.f;
	g /= 255.f;
	b /= 255.f;

	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(min.x, min.y - healthbar_size_y), ImVec2(max.x, max.y), ImGui::GetColorU32({ 0.1f, 0.1f, 0.1f, 1.f }), 0.f, 15);
	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(min.x, min.y - healthbar_size_y), ImVec2(min.x + lenght_left_to_right, max.y), ImGui::GetColorU32({ r, g, b, 1.f }), 0.f, 15);
	ImGui::GetOverlayDrawList()->AddRect(ImVec2(min.x, min.y - healthbar_size_y), ImVec2(max.x, max.y), ImGui::GetColorU32({ 0.f, 0.f, 0.f, 1.f }), 0.f, 15, 1.f);
}

std::string get_agent_name_by_id(int id)
{
	switch (id)
	{
	case 15342018:
		return "Default";
	case 14222427:
		return "Astra";
	case 14208983:
		return "Breach";
	case 15336891:
		return "Brimstone";
	case 15323988:
		return "Chamber";
	case 15329442:
		return "Cypher";
	case 15319792:
		return "Fade";
	case 15343659:
		return "Jett";
	case 14214159:
		return "Kay/O";
	case 14219028:
		return "Killjoy";
	case 14224708:
		return "Neon";
	case 15342356:
		return "Omen";
	case 1533054:
		return "Phoenix";
	case 15321953:
		return "Raze";
	case 15341565:
		return "Reyna";
	case 15340716:
		return "Sage";
	case 15330654:
		return "Sova";
	case 14220343:
		return "Viper";
	case 14226556:
		return "Yoru";
	default:
		return std::to_string(id);
	}
}

std::string get_player_name(int id)
{
	return "";
}

std::string get_weapon_name_by_id(int id)
{
	switch (id)
	{
	case 4587590:
		return "Knife";
	case 14228426:
		return "Classic";
	case 14344067:
		return "Shorty";
	case 14317610:
		return "Frenzy";
	case 14331757:
		return "Ghost";
	case 14337438:
		return "Sherif";
	case 14381275:
		return "Stinger";
	case 14372361:
		return "Spectre";
	case 14311125:
		return "Bucky";
	case 14306695:
		return "Judge";
	case 14290828:
		return "Bulldog";
	case 14359098:
		return "Guardian";
	case 931427888:
		return "Phantom";
	case 634223136:
		return "Vandal";
	case 14367103:
		return "Marshal";
	case 14347794:
		return "Operator";
	case 14277173:
		return "Ares";
	case 14273531:
		return "Odin";
	case 14230480:
		return "Jett Ult";
	case 14213014:
		return "Chamber Ult";
	case 14212558:
		return "Chamber Sherif";
	case 14225417:
		return "Neon Ult";
	case 2:
		return "Unknown";
	default:
		return std::to_string(id);
	}
}

void CheatCache()
{
	while (true)
	{
		uintptr_t world = read<uintptr_t>(virtualaddy + 0x60);
		world = check::validate_pointer(world);
		if (!world) continue;

		uintptr_t game_instance = read<uintptr_t>(virtualaddy + world + 0x1A0);
		if (!game_instance) continue;

		uintptr_t persistent_level = read<uintptr_t>(virtualaddy + world + 0x38);
		persistent_level = check::validate_pointer(persistent_level);
		if (!persistent_level) continue;

		uintptr_t local_players = read<uintptr_t>(game_instance + 0x40);
		if (!local_players) continue;

		uintptr_t local_player = read<uintptr_t>(local_players);
		if (!local_player) continue;

		pointer::player_controller = read<uintptr_t>(local_player + 0x38);
		if (check::is_guarded(pointer::player_controller))
		{
			guarded_pointers::guard_controller = virtualaddy;

			pointer::player_controller = check::validate_pointer(pointer::player_controller);
		}
		else guarded_pointers::guard_controller = 0;
		if (!pointer::player_controller) continue;

		uintptr_t local_pawn = read<uintptr_t>(guarded_pointers::guard_controller + pointer::player_controller + 0x460);
		if (check::is_guarded(local_pawn))
		{
			guarded_pointers::guard_local_pawn = virtualaddy;
			local_pawn = check::validate_pointer(local_pawn);
		}
		else guarded_pointers::guard_local_pawn = 0;

		pointer::local_pawn = local_pawn;
		if (!pointer::local_pawn || (pointer::local_pawn != pointer::local_pawn_old))
		{
			if (!player_pawns.empty())
				player_pawns.clear();
			pointer::local_pawn_old = pointer::local_pawn;
			continue;
		}

		pointer::camera_manager = read<uintptr_t>(pointer::player_controller + 0x478);
		if (!pointer::camera_manager)
		{
			pointer::camera_manager = read<uintptr_t>(virtualaddy + pointer::player_controller + 0x478);
			if (!pointer::camera_manager) continue;
		}

		if (check::is_guarded(pointer::camera_manager))
		{
			pointer::camera_manager = check::validate_pointer(pointer::camera_manager);
		} if (!pointer::camera_manager) continue;

		uintptr_t actor_array = read<uintptr_t>(virtualaddy + persistent_level + 0xA0);
		if (!actor_array) continue;

		INT32 actor_count = read<INT32>(virtualaddy + persistent_level + 0xb8);
		if (!actor_count) continue;

		for (int x = 0; x < actor_count; x++)
		{
			bool is_damage_handler_guarded = false;
			bool is_mesh_guarded = false;

			uintptr_t for_actor = 0;
			uintptr_t for_mesh = 0;

			uintptr_t actor = read<uintptr_t>(actor_array + (x * 8));
			if (!actor) continue;
			if (!check::is_valid(actor)) continue;

			if (check::is_guarded(actor))
			{
				for_actor = virtualaddy;
				actor = check::validate_pointer(actor);
				if (!check::is_valid(actor)) continue;
			}

			INT32 unique_id = read<INT32>(for_actor + actor + 0x38);
			if (unique_id != 0x11e0101) {
				continue;
			}

			uintptr_t mesh = read<uintptr_t>(for_actor + actor + 0x430);
			if (!mesh) continue;
			if (!check::is_valid(mesh)) continue;

			if (check::is_guarded(mesh))
			{
				is_mesh_guarded = true;
				for_mesh = virtualaddy;
				mesh = check::validate_pointer(mesh);
				if (!check::is_valid(mesh)) continue;
			}

			int team_color_diff = read<int>(for_actor + actor + 0x698); // team check
			if (team_color_diff != 2) continue;

			uintptr_t root_comp = read<uintptr_t>(for_actor + actor + 0x230);
			if (!root_comp) continue;

			uintptr_t damage_handler = read<uintptr_t>(for_actor + actor + 0x9A8);
			if (!damage_handler) continue;

			float health = 0.f;
			if (check::is_guarded(damage_handler))
			{
				is_damage_handler_guarded = true;
				damage_handler = check::validate_pointer(damage_handler);
				if (!check::is_valid(damage_handler)) continue;
				health = read<float>(virtualaddy + damage_handler + 0x1B0);
			}
			else
			{
				health = read<float>(damage_handler + 0x1B0);
			}

			if (health <= 0) continue;

			uintptr_t bone_array = read<uintptr_t>(for_mesh + mesh + 0x5C0);
			if (!bone_array) continue;

			INT32 bone_count = read<INT32>(for_mesh + mesh + 0x5C8);
			if (!bone_count) continue;

			Enemy Entities
			{
				for_actor, //guarded region ptr
				for_mesh, //guarded region ptr
				actor,
				mesh,
				bone_array,
				root_comp,
				damage_handler,
				bone_count,
				0, //player_ammo count
				"", //weapon name
				"", //agent name
				"", //Enemy name
				0.f, //distance
				health, //health
				0.f, //shleid
				true,
				is_damage_handler_guarded,
				is_mesh_guarded
			};

			if (!player_pawns.empty()) {
				auto found_player = std::find(player_pawns.begin(), player_pawns.end(), Entities);
				if (found_player == player_pawns.end())
				{
					player_pawns.push_back(Entities);
				}
			}
			else
			{
				player_pawns.push_back(Entities);
			}
		}
	}
}

void UpdateCamera()
{
	camera::location = read<fvector>(virtualaddy + pointer::camera_manager + 0x1260);
	camera::rotation = read<fvector>(virtualaddy + pointer::camera_manager + 0x126C);
	camera::fov = read<float>(virtualaddy + pointer::camera_manager + 0x1278);
}

bool bIsDormant(Enemy APawn) {
	bool dormant = read<bool>(APawn.actor + 0x100);
	if (!dormant) { return false; }
	return true;
};

auto bisVisible(Enemy APawn) -> bool {
	float LastRenderTime = read<float>(APawn.for_mesh + APawn.mesh + offsets::last_render_time);
	float LastSubmitTime = read<float>(APawn.for_mesh + APawn.mesh + offsets::last_submit_time);
	bool IsVisible = LastRenderTime + 0.06F >= LastSubmitTime;
	return IsVisible;
};

void Cheat()
{
	UpdateCamera();
	 
	char players_found[256];
	sprintf_s(players_found, "Player Count: %d", player_pawns.size());
	ImGui::GetOverlayDrawList()->AddText(ImVec2(0, 0), ImGui::GetColorU32({ 1.f, 0.f, 0.f, 1.f }), players_found);

	int closest_player = 1337;
	float closest_distance = FLT_MAX;

	for (int x = 0; x < player_pawns.size(); x++)
	{
		Enemy Entity = player_pawns[x];

		float health = 0;

		if (Entity.is_damage_handler_guarded)
			health = read<float>(virtualaddy + Entity.damage_handler + 0x1B0);
		else
			health = read<float>(Entity.damage_handler + 0x1B0);

		if (health <= 0.f || health > 999.f) player_pawns[x].is_valid = false;

		if (!Entity.is_valid) {
			auto erase_player = std::find(player_pawns.begin(), player_pawns.end(), Entity);
			player_pawns.erase(erase_player);
			continue;
		}

		fvector RootBone = bone_matrix(0, Entity);
		fvector RootBoneProjected = W2S(RootBone);

		fvector HeadBone = bone_matrix(8, Entity);
		fvector HeadBoneProjected = W2S(HeadBone);
		ImVec2 HeadBoneProjected2 = ImVec2(HeadBoneProjected.x, HeadBoneProjected.y);

		Vector3 head2 = Vector3(HeadBone.x, HeadBone.y, HeadBone.z);

		Vector3 CameraPosition = read<Vector3>(virtualaddy + pointer::camera_manager + offsets::camera_position);
		auto DistanceModifier = CameraPosition.Distance(head2) * 0.01F;
		auto Distance = CameraPosition.Distance(head2) * 0.001F;

		float BoxHeight = abs(RootBoneProjected.y - HeadBoneProjected.y);
		float BoxWidth = BoxHeight * 0.40;
		float Width = BoxWidth / 10;
		if (Width < 2) Width = 2;
		if (Width > 3) Width = 3;

		int bottom_text_offset = 2;

		auto Inventory = read<intptr_t>(Entity.actor + 0x948);
		intptr_t CurrentEquip = read<intptr_t>(Inventory + 0x238);
		uintptr_t MagazineAmmo = read<uintptr_t>(CurrentEquip + 0xFB0);
		int32_t AuthResourceAmount = read<int32_t>(MagazineAmmo + 0x100);
		int32_t MaxAmmo = read<int32_t>(MagazineAmmo + 0x120);
		std::string DisplayAmmoA = "Ammo: " + std::to_string(AuthResourceAmount) + " / " + std::to_string(MaxAmmo);

		bool IsVisible = bisVisible(Entity);

		if (IsVisible) { ESPColor = RGBCol.green; espcolor = Col.green; }
		else { ESPColor = RGBCol.red; espcolor = Col.red; }

		if (settings::player_dormant_check) if (!bIsDormant(Entity)) continue;

		if (settings::player_box)
		{
			switch (settings::player_box_selection)
			{
			case 0:
				DrawFilledRect3(RootBoneProjected.x - (BoxWidth / 2), HeadBoneProjected.y, BoxWidth, BoxHeight, &Col.glass);
				DrawCornerBox(RootBoneProjected.x - (BoxWidth / 2), HeadBoneProjected.y, BoxWidth, BoxHeight, 1, &espcolor); break;
			case 1: draw_3d_box(RootBone, fvector(HeadBone.x, HeadBone.y, HeadBone.z + 20), 43, ESPColor, 1.f); break;
			case 2: DrawCornerBox(RootBoneProjected.x - (BoxWidth / 2), HeadBoneProjected.y, BoxWidth, BoxHeight, 1, &espcolor); break;
			}
		}

		if (settings::player_distance)
		{
			char distance_text[256];
			ImVec2 text_size = ImGui::CalcTextSize(distance_text);
			sprintf_s(distance_text, skCrypt("[%.fm]"), DistanceModifier);

			DrawPlayerBar(HeadBoneProjected.x - (text_size.x / 2) + 10, HeadBoneProjected.y - 4, &Col.darkgray_, &Col.white_, distance_text);
		}

		if (settings::player_snapline)
		{
			switch (settings::player_snapline_selection)
			{
			case 0: ImGui::GetOverlayDrawList()->AddLine(ImVec2(center_x, center_y), ImVec2(RootBoneProjected.x, RootBoneProjected.y + bottom_text_offset), ImColor(0, 255, 255, 200), 1.f); break;
			case 1: ImGui::GetOverlayDrawList()->AddLine(ImVec2(center_x, GetSystemMetrics(2500)), ImVec2(RootBoneProjected.x, RootBoneProjected.y + bottom_text_offset), ImColor(0, 255, 255, 200), 1.f); break;
			case 2: ImGui::GetOverlayDrawList()->AddLine(ImVec2(center_x, center_y * 2), ImVec2(RootBoneProjected.x, RootBoneProjected.y + bottom_text_offset), ImColor(0, 255, 255, 200), 1.f); break;
			}
		}

		if (settings::player_skeleton)
		{
			DrawCircle(HeadBoneProjected2, 7 / Distance, &Col.red, 1000, 1);
			draw_skeleton(Entity, ImColor(255, 255, 255, 255), 1.f);
		}

		if (settings::player_healthbar)
		{
			float x1 = RootBoneProjected.x - (BoxWidth / 2);
			float x2 = RootBoneProjected.x + (BoxWidth / 2);

			draw_health_bar(ImVec2(x1, HeadBoneProjected.y - 3), ImVec2(x2, HeadBoneProjected.y - 3), health);
		}

		if (settings::player_view_angle)
		{
			
		}

		if (settings::player_ammo)
		{
		}

		if (settings::team_swap)
	    {
		}

		if (settings::player_agent)
		{
		}

		if (settings::player_weapon)
		{
			uintptr_t inventory = read<uintptr_t>(Entity.for_actor + Entity.actor + 0x948);
			uintptr_t current_equip = read<uintptr_t>(inventory + 0x218);
			int weapon_id = read<int>(current_equip + 0x18);
			std::string weapon_name = get_weapon_name_by_id(weapon_id);
			ImVec2 text_size = ImGui::CalcTextSize(weapon_name.c_str());

			ImGui::GetOverlayDrawList()->AddText(ImVec2(RootBoneProjected.x - (text_size.x / 2), RootBoneProjected.y + bottom_text_offset), ImGui::GetColorU32({ 1.f, 1.f, 1.f, 1.f }), weapon_name.c_str());
			bottom_text_offset += 14;
		}
		if (settings::chams)
		{

		}
		auto dx = HeadBoneProjected.x - center_x;
		auto dy = HeadBoneProjected.y - center_y;
		auto dist = sqrtf(dx * dx + dy * dy);
		if (IsVisible && (dist < closest_distance))
		{
			closest_distance = dist;
			closest_player = x;
		}
	}

	if (closest_player != 1337 && settings::aimbot_enable)
	{
		Enemy target = player_pawns[closest_player];

		fvector head = bone_matrix(settings::aimbot_aimbone, target);
		fvector head_screen = W2S(head);

		if (settings::aimbot_draw_target_line && in_rect(center_x, center_y, settings::aimbot_fov, head_screen.x, head_screen.y))
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(center_x, center_y), ImVec2(head_screen.x, head_screen.y), ImGui::GetColorU32({ 1.f, 0.f, 0.f, 1.f }), 1.f);

		if (settings::aimbot_recoil_control)
		{
			if (GetAsyncKeyState(hotkeys::aimkey) && in_rect(center_x, center_y, settings::aimbot_fov, head_screen.x, head_screen.y))
			{
				fvector control_rotation = read<fvector>(guarded_pointers::guard_controller + pointer::player_controller + 0x440);
				fvector camera_rotation = camera::rotation;
				fvector new_aim_rotation;
				fvector recoil;

				recoil.x = camera_rotation.x - control_rotation.x;
				recoil.y = camera_rotation.y - control_rotation.y;
				recoil.z = 0.f;

				fvector vector_pos = head - camera::location;
				float distance = (double)(sqrtf(vector_pos.x * vector_pos.x + vector_pos.y * vector_pos.y + vector_pos.z * vector_pos.z));
				float x, y, z;
				x = -((acosf(vector_pos.z / distance) * (float)(180.0f / 3.14159265358979323846264338327950288419716939937510)) - 90.f);
				y = atan2f(vector_pos.y, vector_pos.x) * (float)(180.0f / 3.14159265358979323846264338327950288419716939937510);
				z = 0;

				fvector target_rotation = fvector(x, y, z);
				new_aim_rotation.x = target_rotation.x - recoil.x - recoil.x;
				new_aim_rotation.y = target_rotation.y - recoil.y - recoil.y;
				new_aim_rotation.z = 0;

				float smooth = settings::aimbot_smooth;
				if (settings::aimbot_constant)
				{
					smooth += settings::aim_constant_offset;
				}

				if (!bIsDormant(target));

				fvector new_rotation = smooth_aim(new_aim_rotation, control_rotation, smooth);
				if (new_rotation.x < 0)
				{
					new_rotation.x += 360.f;
				}
				if (new_rotation.y < 0)
				{
					new_rotation.y += 360.f;
				}
				new_rotation.z = 0;

				write<fvector>(guarded_pointers::guard_controller + pointer::player_controller + 0x440, new_rotation);
			}
		}
		else
		{
			if (GetAsyncKeyState(hotkeys::aimkey) && in_rect(center_x, center_y, settings::aimbot_fov, head_screen.x, head_screen.y)) //TODO: FIX THE BOUNCE
			{
				fvector camera_rotation = camera::rotation;
				fvector new_aim_rotation;

				fvector vector_pos = head - camera::location;
				float distance = (double)(sqrtf(vector_pos.x * vector_pos.x + vector_pos.y * vector_pos.y + vector_pos.z * vector_pos.z));
				float x, y, z;
				x = -((acosf(vector_pos.z / distance) * (float)(180.0f / 3.14159265358979323846264338327950288419716939937510)) - 90.f);
				y = atan2f(vector_pos.y, vector_pos.x) * (float)(180.0f / 3.14159265358979323846264338327950288419716939937510);
				z = 0;

				fvector target_rotation = fvector(x, y, z);
				new_aim_rotation.x = target_rotation.x;
				new_aim_rotation.y = target_rotation.y;
				new_aim_rotation.z = 0;

				float smooth = settings::aimbot_smooth;
				if (settings::aimbot_constant)
				{
					smooth += settings::aim_constant_offset;
				}

				if (!bIsDormant(target));

				fvector new_rotation = smooth_aim(new_aim_rotation, camera_rotation, smooth);
				if (new_rotation.x < 0)
				{
					new_rotation.x += 360.f;
				}
				if (new_rotation.y < 0)
				{
					new_rotation.y += 360.f;
				}
				new_rotation.z = 0;

				write<fvector>(guarded_pointers::guard_controller + pointer::player_controller + 0x440, new_rotation);
			}
		}
	}
}

static HWND Window = NULL;
IDirect3D9Ex* p_Object = NULL;
static LPDIRECT3DDEVICE9 D3dDevice = NULL;
static LPDIRECT3DVERTEXBUFFER9 TriBuf = NULL;
HWND hwnd = NULL;
RECT GameRect = { NULL };
D3DPRESENT_PARAMETERS d3dpp;
const MARGINS Margin = { -1 };
MSG Message = { NULL };

void render()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	Cheat();
	DrawMenu();

	ImGui::EndFrame();
	D3dDevice->SetRenderState(D3DRS_ZENABLE, false);
	D3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	D3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
	D3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	if (D3dDevice->BeginScene() >= 0) {
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		D3dDevice->EndScene();
	}
	HRESULT result = D3dDevice->Present(NULL, NULL, NULL, NULL);

	if (result == D3DERR_DEVICELOST && D3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
		ImGui_ImplDX9_InvalidateDeviceObjects();
		D3dDevice->Reset(&d3dpp);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}
bool gay(uintptr_t pointer)
{
	constexpr uintptr_t filter = 0xFFFFFFF000000000;
	uintptr_t result = pointer & filter;
	return result == 0x8000000000 || result == 0x10000000000;
}
bool start_directx()
{
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
		return false;

	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth = Width;
	d3dpp.BackBufferHeight = Height;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.hDeviceWindow = Window;
	d3dpp.Windowed = TRUE;

	p_Object->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &D3dDevice);

	IMGUI_CHECKVERSION();

	ImGui::CreateContext();
	ImGuiStyle& s = ImGui::GetStyle();

	//s.Colors[ImGuiCol_Button] = ImColor(26, 26, 26, 255);
	//s.Colors[ImGuiCol_ButtonActive] = ImColor(26, 26, 26, 255);
	//s.Colors[ImGuiCol_ButtonHovered] = ImColor(26, 26, 26, 255);

	//s.Colors[ImGuiCol_FrameBg] = ImColor(26, 26, 26, 255);
	//s.Colors[ImGuiCol_FrameBgActive] = ImColor(26, 26, 26, 255);
	//s.Colors[ImGuiCol_FrameBgHovered] = ImColor(26, 26, 26, 255);

	//FONTS
	//fonts::standard_font = io.Fonts->AddFontDefault();
	//fonts::intro_font = io.Fonts->AddFontFromFileTTF("adfg.ttf", 300.f);
	//fonts::standard_font = io.Fonts->AddFontDefault();

	ImGui_ImplWin32_Init(Window);
	ImGui_ImplDX9_Init(D3dDevice);

	p_Object->Release();
	return true;
}

void wait_for_window()
{
	while (true)
	{
		HWND foreground_window = GetForegroundWindow();
		HWND target_window = FindWindowA(0, skCrypt("VALORANT  "));

		if (foreground_window == target_window)
			break;

		Sleep(200);
	}
}

void render_loop()
{
	static RECT old_rc;
	ZeroMemory(&Message, sizeof(MSG));

	while (Message.message != WM_QUIT)
	{
		if (PeekMessage(&Message, Window, 0, 0, 0x0001))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		HWND hwnd_active = GetForegroundWindow();

		if (hwnd_active == hwnd) {
			HWND hwndtest = GetWindow(hwnd_active, 3);
			SetWindowPos(Window, hwndtest, 2, 2, -3, -3, 0x0002 | 0x0001);
		}

		RECT rc;
		POINT xy;

		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		GetClientRect(hwnd, &rc);
		ClientToScreen(hwnd, &xy);
		rc.left = xy.x;
		rc.top = xy.y;

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = hwnd;
		io.DeltaTime = 1.0f / 60.0f;

		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;

		if (GetAsyncKeyState(VK_LBUTTON)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else
			io.MouseDown[0] = false;

		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom)
		{
			old_rc = rc;

			Width = rc.right;
			Height = rc.bottom;

			d3dpp.BackBufferWidth = Width;
			d3dpp.BackBufferHeight = Height;
			SetWindowPos(Window, (HWND)0, xy.x + 2, xy.y + 2, Width - 3, Height - 3, 0x0008);
			D3dDevice->Reset(&d3dpp);
		}

		render();
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	DestroyWindow(Window);
}

void stop_render()
{
	TriBuf->Release();
	D3dDevice->Release();
	p_Object->Release();

	DestroyWindow(Window);
	UnregisterClassW((_(L"Untitled - Notepad")), NULL);
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message)
	{
	case WM_DESTROY:
		stop_render();
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (D3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			d3dpp.BackBufferWidth = LOWORD(lParam);
			d3dpp.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = D3dDevice->Reset(&d3dpp);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

void create_window()
{
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.lpszClassName = (_(L"Untitled - Notepad"));
	wc.lpfnWndProc = WinProc;
	RegisterClassEx(&wc);

	if (hwnd)
	{
		GetClientRect(hwnd, &GameRect);
		POINT xy;
		ClientToScreen(hwnd, &xy);
		GameRect.left = xy.x;
		GameRect.top = xy.y;

		Width = GameRect.right;
		Height = GameRect.bottom;
	}
	else
		exit(2);

	//something is detected here, i will fix it later
	Window = CreateWindowExW(NULL, _(L"Untitled - Notepad"), _(L"Untitled - Notepad"), 0x80000000L | 0x10000000L, 2, 2, Width - 2, Height - 2, 0, 0, 0, 0);

	DwmExtendFrameIntoClientArea(Window, &Margin);
	SetWindowLong(Window, (-20), 0x00000020L | 0x00000080L | 0x00080000);

	ShowWindow(Window, SW_SHOW);
	UpdateWindow(Window);
}

void rndmBone()
{
	for (;;)
	{
		std::vector<int> boneidList{ 8, 21, 6 };

		int index = rand() % boneidList.size();
		int value = boneidList[index];

		if (settings::aimbot_random_aim_bone)
		{
			settings::aimbot_aimbone = value;
			std::cout << value << "\n";
		}

		Sleep(settings::aimbot_random_aim_bone_interval * 1000);
	}
}

void start_cheat()
{

	hwnd = FindWindowA(0, _("VALORANT  "));

	create_window();

	start_directx();

	wait_for_window();

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)CheatCache, NULL, NULL, NULL);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)rndmBone, NULL, NULL, NULL);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)calculate_random_for_constant_aimbot, NULL, NULL, NULL);

	render_loop();

	stop_render();
}