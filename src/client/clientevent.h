/*
Minetest
Copyright (C) 2017 nerzhul, Loic Blot <loic.blot@unix-experience.fr>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#pragma once

#include <string>
#include "irrlichttypes_bloated.h"

struct ParticleParameters;
struct ParticleSpawnerParameters;
struct SkyboxParams;
struct SunParams;
struct MoonParams;
struct StarParams;

enum ClientEventType : u8
{
	CE_NONE,
	CE_PLAYER_DAMAGE,
	CE_PLAYER_FORCE_MOVE,
	CE_DEATHSCREEN,
	CE_SHOW_FORMSPEC,
	CE_SHOW_LOCAL_FORMSPEC,
	CE_SPAWN_PARTICLE,
	CE_ADD_PARTICLESPAWNER,
	CE_DELETE_PARTICLESPAWNER,
	CE_HUDADD,
	CE_HUDRM,
	CE_HUDCHANGE,
	CE_SET_SKY,
	CE_SET_SUN,
	CE_SET_MOON,
	CE_SET_STARS,
	CE_OVERRIDE_DAY_NIGHT_RATIO,
	CE_CLOUD_PARAMS,
	CLIENTEVENT_MAX,
};

struct ClientEvent
{
	ClientEventType type;
	union
	{
		// struct{
		//} none;
		struct
		{
			uint16_t amount;
		} player_damage;
		struct
		{
			float pitch;
			float yaw;
		} player_force_move;
		struct
		{
			bool set_camera_point_target;
			float camera_point_target_x;
			float camera_point_target_y;
			float camera_point_target_z;
		} deathscreen;
		struct
		{
			std::string *formspec;
			std::string *formname;
		} show_formspec;
		// struct{
		//} textures_updated;
		ParticleParameters *spawn_particle;
		struct
		{
			ParticleSpawnerParameters *p;
			uint16_t attached_id;
			uint64_t id;
		} add_particlespawner;
		struct
		{
			uint32_t id;
		} delete_particlespawner;
		struct
		{
			uint32_t server_id;
			uint8_t type;
			v2f *pos;
			std::string *name;
			v2f *scale;
			std::string *text;
			uint32_t number;
			uint32_t item;
			uint32_t dir;
			v2f *align;
			v2f *offset;
			v3f *world_pos;
			v2s32 *size;
			int16_t z_index;
			std::string *text2;
		} hudadd;
		struct
		{
			uint32_t id;
		} hudrm;
		struct
		{
			uint32_t id;
			HudElementStat stat;
			v2f *v2fdata;
			std::string *sdata;
			uint32_t data;
			v3f *v3fdata;
			v2s32 *v2s32data;
		} hudchange;
		SkyboxParams *set_sky;
		struct
		{
			bool do_override;
			float ratio_f;
		} override_day_night_ratio;
		struct
		{
			float density;
			uint32_t color_bright;
			uint32_t color_ambient;
			float height;
			float thickness;
			float speed_x;
			float speed_y;
		} cloud_params;
		SunParams *sun_params;
		MoonParams *moon_params;
		StarParams *star_params;
	};
};
