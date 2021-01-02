/*
Minetest
Copyright (C) 2016-2019 Duane Robertson <duane@duanerobertson.com>
Copyright (C) 2016-2019 paramat

Based on Valleys Mapgen by Gael de Sailly
(https://forum.minetest.net/viewtopic.php?f=9&t=11430)
and mapgen_v7, mapgen_flat by kwolekr and paramat.

Licensing changed by permission of Gael de Sailly.

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

#include "mapgen.h"

#define MGVALLEYS_ALT_CHILL        0x01
#define MGVALLEYS_HUMID_RIVERS     0x02
#define MGVALLEYS_VARY_RIVER_DEPTH 0x04
#define MGVALLEYS_ALT_DRY          0x08

class BiomeManager;
class BiomeGenOriginal;

extern FlagDesc flagdesc_mapgen_valleys[];


struct MapgenValleysParams : public MapgenParams {
	uint16_t altitude_chill = 90;
	uint16_t river_depth = 4;
	uint16_t river_size = 5;

	float cave_width = 0.09f;
	int16_t large_cave_depth = -33;
	uint16_t small_cave_num_min = 0;
	uint16_t small_cave_num_max = 0;
	uint16_t large_cave_num_min = 0;
	uint16_t large_cave_num_max = 2;
	float large_cave_flooded = 0.5f;
	int16_t cavern_limit = -256;
	int16_t cavern_taper = 192;
	float cavern_threshold = 0.6f;
	int16_t dungeon_ymin = -31000;
	int16_t dungeon_ymax = 63;

	NoiseParams np_filler_depth;
	NoiseParams np_inter_valley_fill;
	NoiseParams np_inter_valley_slope;
	NoiseParams np_rivers;
	NoiseParams np_terrain_height;
	NoiseParams np_valley_depth;
	NoiseParams np_valley_profile;

	NoiseParams np_cave1;
	NoiseParams np_cave2;
	NoiseParams np_cavern;
	NoiseParams np_dungeons;

	MapgenValleysParams();
	~MapgenValleysParams() = default;

	void readParams(const Settings *settings);
	void writeParams(Settings *settings) const;
	void setDefaultSettings(Settings *settings);
};


class MapgenValleys : public MapgenBasic {
public:

	MapgenValleys(MapgenValleysParams *params,
		EmergeParams *emerge);
	~MapgenValleys();

	virtual MapgenType getType() const { return MAPGEN_VALLEYS; }

	virtual void makeChunk(BlockMakeData *data);
	int getSpawnLevelAtPoint(v2s16 p);

private:
	BiomeGenOriginal *m_bgen;

	float altitude_chill;
	float river_depth_bed;
	float river_size_factor;

	Noise *noise_inter_valley_fill;
	Noise *noise_inter_valley_slope;
	Noise *noise_rivers;
	Noise *noise_terrain_height;
	Noise *noise_valley_depth;
	Noise *noise_valley_profile;

	virtual int generateTerrain();
};
