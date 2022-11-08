#pragma once
#include "driver.h"
#include "check.h"
#include "blow.h"
#include "ss.h"

#include "vectors.hpp"
#include "skcrypt.hpp"
#include "stdafx.h"
#include "xor.h"

#include <wininet.h>
#include <cstddef>
#include <thread>
#include <vector>
#include <string>

inline uintptr_t g_baseaddress{};

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
typedef          __int64 ll;
typedef unsigned __int64 ull;
typedef          char   int8;
typedef   signed char   sint8;
typedef unsigned char   uint8;
typedef          short  int16;
typedef   signed short  sint16;
typedef unsigned short  uint16;
typedef          int    int32;
typedef   signed int    sint32;
typedef unsigned int    uint32;
typedef ll              int64;
typedef ll              sint64;
typedef ull             uint64;
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

namespace VALORANT
{
	namespace settings
	{
		float aim_constant_offset = 0.f;

		bool aimbot_enable = true;
		bool aimbot_constant = true;
		bool aimbot_draw_fov = false;
		bool aimbot_recoil_control = true;
		bool aimbot_random_aim_bone = false;
		bool aimbot_draw_target_line = true;
		bool team_swap = false;
		bool aimbot_360_mode = false;
		float aimbot_fov = 100.f;
		float aimbot_smooth = 20.f;
		int aimbot_aimbone = 8;
		float aimbot_random_aim_bone_interval = 5;

		int aimbot_aimbone_selection = 0;
		int player_snapline_selection = 2;
		int player_box_selection = 2;

		static const char* aimbot_aimbone_items[]{ "Head", "Neck", "Pelvis" };
		static const char* player_snapline_items[]{ "Top", "Middle", "Bottom" };
		static const char* player_box_items[]{ "Filled", "3D Box", "Cornered Box" };

		bool player_box = true;
		bool player_skeleton = true;
		bool player_snapline = false;
		bool player_view_angle = false;
		bool player_distance = false;
		bool player_healthbar = true;
		bool player_agent = true;
		bool player_weapon = false;
		bool player_ammo = false;
		bool player_dormant_check = false;
		bool player_radar = false;
		bool chams = false;
		float fresnel_intensity = 1.f;
	}

	namespace offsets
	{

	}

	namespace guarded_pointers
	{

	}

	namespace pointer
	{
		uintptr_t local_pawn;
		uintptr_t local_pawn_old;
		uintptr_t player_controller;
		uintptr_t camera_manager;
	}

	namespace camera
	{
		fvector location;
		fvector rotation;
		float fov;
	}
	
	template<class T> T __ROL__(T value, int count)
	{
		const uint nbits = sizeof(T) * 8;

		if (count > 0)
		{
			count %= nbits;
			T high = value >> (nbits - count);
			if (T(-1) < 0)
				high &= ~((T(-1) << count));
			value <<= count;
			value |= high;
		}
		else
		{
			count = -count % nbits;
			T low = value << (nbits - count);
			value >>= count;
			value |= low;
		}
		return value;
	}

	inline uint8  __ROL1__(uint8  value, int count) { return __ROL__((uint8)value, count); }
	inline uint16 __ROL2__(uint16 value, int count) { return __ROL__((uint16)value, count); }
	inline uint32 __ROL4__(uint32 value, int count) { return __ROL__((uint32)value, count); }
	inline uint64 __ROL8__(uint64 value, int count) { return __ROL__((uint64)value, count); }
	inline uint8  __ROR1__(uint8  value, int count) { return __ROL__((uint8)value, -count); }
	inline uint16 __ROR2__(uint16 value, int count) { return __ROL__((uint16)value, -count); }
	inline uint32 __ROR4__(uint32 value, int count) { return __ROL__((uint32)value, -count); }
	inline uint64 __ROR8__(uint64 value, int count) { return __ROL__((uint64)value, -count); }

	__forceinline __int64 decrypt_uworld(const uint32_t key, const uintptr_t* state) {

		unsigned __int64 v19; // r11
		unsigned __int64 v20; // r8
		unsigned __int64 v21; // r9
		unsigned int v22; // er10
		unsigned __int64 v23; // rcx
		unsigned __int64 v24; // rdx
		unsigned __int64 v25; // rcx
		int v26; // ebx
		unsigned int v27; // ecx
		__int64 v28; // rax
		unsigned __int64 v29; // r8
		unsigned __int64 v30; // r8
		unsigned __int64 v31; // rcx
		unsigned __int64 v32; // rdx
		unsigned __int64 v33; // rcx

		v19 = 2685821657736338717i64
			* ((unsigned int)key ^ (unsigned int)(key << 25) ^ (((unsigned int)key ^ ((unsigned __int64)(unsigned int)key >> 15)) >> 12))
			% 7;
		v20 = state[v19];
		v21 = (2685821657736338717i64
			* ((unsigned int)key ^ (unsigned int)(key << 25) ^ (((unsigned int)key ^ ((unsigned __int64)(unsigned int)key >> 15)) >> 12))) >> 32;
		v22 = (unsigned int)v19 % 7;
		if (!((unsigned int)v19 % 7))
		{
			v23 = (2 * (v20 - (unsigned int)(v21 - 1))) ^ ((2 * (v20 - (unsigned int)(v21 - 1))) ^ ((v20
				- (unsigned int)(v21 - 1)) >> 1)) & 0x5555555555555555i64;
			v24 = (4 * v23) ^ ((4 * v23) ^ (v23 >> 2)) & 0x3333333333333333i64;
			v25 = (16 * v24) ^ ((16 * v24) ^ (v24 >> 4)) & 0xF0F0F0F0F0F0F0Fi64;
			v20 = __ROL8__((v25 << 8) ^ ((v25 << 8) ^ (v25 >> 8)) & 0xFF00FF00FF00FFi64, 32);
		LABEL_26:
			v26 = 2 * v19;
			goto LABEL_27;
		}
		if (v22 != 1)
			goto LABEL_26;
		v26 = 2 * v19;
		v20 = __ROL8__(v20 - (unsigned int)(2 * v19 + v21), (unsigned __int8)(((int)v21 + (int)v19) % 0x3Fu) + 1);
	LABEL_27:
		v27 = v26 + v21;
		if (v22 == 2)
			v20 = ~(v20 - v27);
		switch (v22)
		{
		case 3u:
			v28 = 2 * ((2 * v20) ^ ((2 * v20) ^ (v20 >> 1)) & 0x5555555555555555i64);
			v20 = v28 ^ (v28 ^ (((2 * v20) ^ ((2 * v20) ^ (v20 >> 1)) & 0x5555555555555555i64) >> 1)) & 0x5555555555555555i64;
			break;
		case 4u:
			v29 = __ROR8__(v20, (unsigned __int8)(v27 % 0x3F) + 1);
			v20 = (2 * v29) ^ ((2 * v29) ^ (v29 >> 1)) & 0x5555555555555555i64;
			break;
		case 5u:
			v30 = __ROR8__(v20, (unsigned __int8)(v27 % 0x3F) + 1);
			v31 = (2 * v30) ^ ((2 * v30) ^ (v30 >> 1)) & 0x5555555555555555i64;
			v32 = (4 * v31) ^ ((4 * v31) ^ (v31 >> 2)) & 0x3333333333333333i64;
			v33 = (16 * v32) ^ ((16 * v32) ^ (v32 >> 4)) & 0xF0F0F0F0F0F0F0Fi64;
			v20 = __ROL8__((v33 << 8) ^ ((v33 << 8) ^ (v33 >> 8)) & 0xFF00FF00FF00FFi64, 32);
			break;
		case 6u:
			v20 = ~v20 - (unsigned int)(v21 + v19);
			break;
		}
		return v20 ^ (unsigned int)key;
	}
	inline auto decryptWorld(uintptr_t base_address) -> uintptr_t {
		auto key = read<uintptr_t>(base_address + offsets::uworld_key);
#pragma pack(push, 1)
		struct State
		{
			uint64_t Keys[7];
		};
#pragma pack(pop)
		auto state = read<State>(base_address + offsets::uworld_state);
		auto decrypt = decrypt_uworld(key, (uintptr_t*)&state);
		return read<uintptr_t>(decrypt);
	}
}



