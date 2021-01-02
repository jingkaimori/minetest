/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#include "voxel.h"
#include "map.h"
#include "gettime.h"
#include "nodedef.h"
#include "util/directiontables.h"
#include "util/timetaker.h"
#include <cstring>  // memcpy, memset

/*
	Debug stuff
*/
uint64_t addarea_time = 0;
uint64_t emerge_time = 0;
uint64_t emerge_load_time = 0;
uint64_t clearflag_time = 0;

VoxelManipulator::~VoxelManipulator()
{
	clear();
}

void VoxelManipulator::clear()
{
	// Reset area to volume=0
	m_area = VoxelArea();
	delete[] m_data;
	m_data = nullptr;
	delete[] m_flags;
	m_flags = nullptr;
}

void VoxelManipulator::print(std::ostream &o, const NodeDefManager *ndef,
	VoxelPrintMode mode)
{
	const v3s16 &em = m_area.getExtent();
	v3s16 of = m_area.MinEdge;
	o<<"size: "<<em.X<<"x"<<em.Y<<"x"<<em.Z
	 <<" offset: ("<<of.X<<","<<of.Y<<","<<of.Z<<")"<<std::endl;

	for(int32_t y=m_area.MaxEdge.Y; y>=m_area.MinEdge.Y; y--)
	{
		if(em.X >= 3 && em.Y >= 3)
		{
			if     (y==m_area.MinEdge.Y+2) o<<"^     ";
			else if(y==m_area.MinEdge.Y+1) o<<"|     ";
			else if(y==m_area.MinEdge.Y+0) o<<"y x-> ";
			else                           o<<"      ";
		}

		for(int32_t z=m_area.MinEdge.Z; z<=m_area.MaxEdge.Z; z++)
		{
			for(int32_t x=m_area.MinEdge.X; x<=m_area.MaxEdge.X; x++)
			{
				uint8_t f = m_flags[m_area.index(x,y,z)];
				char c;
				if(f & VOXELFLAG_NO_DATA)
					c = 'N';
				else
				{
					c = 'X';
					MapNode n = m_data[m_area.index(x,y,z)];
					content_t m = n.getContent();
					uint8_t pr = n.param2;
					if(mode == VOXELPRINT_MATERIAL)
					{
						if(m <= 9)
							c = m + '0';
					}
					else if(mode == VOXELPRINT_WATERPRESSURE)
					{
						if(ndef->get(m).isLiquid())
						{
							c = 'w';
							if(pr <= 9)
								c = pr + '0';
						}
						else if(m == CONTENT_AIR)
						{
							c = ' ';
						}
						else
						{
							c = '#';
						}
					}
					else if(mode == VOXELPRINT_LIGHT_DAY)
					{
						if(ndef->get(m).light_source != 0)
							c = 'S';
						else if(!ndef->get(m).light_propagates)
							c = 'X';
						else
						{
							uint8_t light = n.getLight(LIGHTBANK_DAY, ndef);
							if(light < 10)
								c = '0' + light;
							else
								c = 'a' + (light-10);
						}
					}
				}
				o<<c;
			}
			o<<' ';
		}
		o<<std::endl;
	}
}

void VoxelManipulator::addArea(const VoxelArea &area)
{
	// Cancel if requested area has zero volume
	if (area.hasEmptyExtent())
		return;

	// Cancel if m_area already contains the requested area
	if(m_area.contains(area))
		return;

	TimeTaker timer("addArea", &addarea_time);

	// Calculate new area
	VoxelArea new_area;
	// New area is the requested area if m_area has zero volume
	if(m_area.hasEmptyExtent())
	{
		new_area = area;
	}
	// Else add requested area to m_area
	else
	{
		new_area = m_area;
		new_area.addArea(area);
	}

	int32_t new_size = new_area.getVolume();

	/*dstream<<"adding area ";
	area.print(dstream);
	dstream<<", old area ";
	m_area.print(dstream);
	dstream<<", new area ";
	new_area.print(dstream);
	dstream<<", new_size="<<new_size;
	dstream<<std::endl;*/

	// Allocate new data and clear flags
	MapNode *new_data = new MapNode[new_size];
	assert(new_data);
	uint8_t *new_flags = new uint8_t[new_size];
	assert(new_flags);
	memset(new_flags, VOXELFLAG_NO_DATA, new_size);

	// Copy old data
	int32_t old_x_width = m_area.MaxEdge.X - m_area.MinEdge.X + 1;
	for(int32_t z=m_area.MinEdge.Z; z<=m_area.MaxEdge.Z; z++)
	for(int32_t y=m_area.MinEdge.Y; y<=m_area.MaxEdge.Y; y++)
	{
		unsigned int old_index = m_area.index(m_area.MinEdge.X,y,z);
		unsigned int new_index = new_area.index(m_area.MinEdge.X,y,z);

		memcpy(&new_data[new_index], &m_data[old_index],
				old_x_width * sizeof(MapNode));
		memcpy(&new_flags[new_index], &m_flags[old_index],
				old_x_width * sizeof(uint8_t));
	}

	// Replace area, data and flags

	m_area = new_area;

	MapNode *old_data = m_data;
	uint8_t *old_flags = m_flags;

	/*dstream<<"old_data="<<(int)old_data<<", new_data="<<(int)new_data
	<<", old_flags="<<(int)m_flags<<", new_flags="<<(int)new_flags<<std::endl;*/

	m_data = new_data;
	m_flags = new_flags;

	delete[] old_data;
	delete[] old_flags;

	//dstream<<"addArea done"<<std::endl;
}

void VoxelManipulator::copyFrom(MapNode *src, const VoxelArea& src_area,
		v3s16 from_pos, v3s16 to_pos, const v3s16 &size)
{
	/* The reason for this optimised code is that we're a member function
	 * and the data type/layout of m_data is know to us: it's stored as
	 * [z*h*w + y*h + x]. Therefore we can take the calls to m_area index
	 * (which performs the preceding mapping/indexing of m_data) out of the
	 * inner loop and calculate the next index as we're iterating to gain
	 * performance.
	 *
	 * src_step and dest_step is the amount required to be added to our index
	 * every time y increments. Because the destination area may be larger
	 * than the source area we need one additional variable (otherwise we could
	 * just continue adding dest_step as is done for the source data): dest_mod.
	 * dest_mod is the difference in size between a "row" in the source data
	 * and a "row" in the destination data (I am using the term row loosely
	 * and for illustrative purposes). E.g.
	 *
	 * src       <-------------------->|'''''' dest mod ''''''''
	 * dest      <--------------------------------------------->
	 *
	 * dest_mod (it's essentially a modulus) is added to the destination index
	 * after every full iteration of the y span.
	 *
	 * This method falls under the category "linear array and incrementing
	 * index".
	 */

	int32_t src_step = src_area.getExtent().X;
	int32_t dest_step = m_area.getExtent().X;
	int32_t dest_mod = m_area.index(to_pos.X, to_pos.Y, to_pos.Z + 1)
			- m_area.index(to_pos.X, to_pos.Y, to_pos.Z)
			- dest_step * size.Y;

	int32_t i_src = src_area.index(from_pos.X, from_pos.Y, from_pos.Z);
	int32_t i_local = m_area.index(to_pos.X, to_pos.Y, to_pos.Z);

	for (int16_t z = 0; z < size.Z; z++) {
		for (int16_t y = 0; y < size.Y; y++) {
			memcpy(&m_data[i_local], &src[i_src], size.X * sizeof(*m_data));
			memset(&m_flags[i_local], 0, size.X);
			i_src += src_step;
			i_local += dest_step;
		}
		i_local += dest_mod;
	}
}

void VoxelManipulator::copyTo(MapNode *dst, const VoxelArea& dst_area,
		v3s16 dst_pos, v3s16 from_pos, const v3s16 &size)
{
	for(int16_t z=0; z<size.Z; z++)
	for(int16_t y=0; y<size.Y; y++)
	{
		int32_t i_dst = dst_area.index(dst_pos.X, dst_pos.Y+y, dst_pos.Z+z);
		int32_t i_local = m_area.index(from_pos.X, from_pos.Y+y, from_pos.Z+z);
		for (int16_t x = 0; x < size.X; x++) {
			if (m_data[i_local].getContent() != CONTENT_IGNORE)
				dst[i_dst] = m_data[i_local];
			i_dst++;
			i_local++;
		}
	}
}

/*
	Algorithms
	-----------------------------------------------------
*/

void VoxelManipulator::clearFlag(uint8_t flags)
{
	// 0-1ms on moderate area
	TimeTaker timer("clearFlag", &clearflag_time);

	//v3s16 s = m_area.getExtent();

	/*dstream<<"clearFlag clearing area of size "
			<<""<<s.X<<"x"<<s.Y<<"x"<<s.Z<<""
			<<std::endl;*/

	//int32_t count = 0;

	/*for(int32_t z=m_area.MinEdge.Z; z<=m_area.MaxEdge.Z; z++)
	for(int32_t y=m_area.MinEdge.Y; y<=m_area.MaxEdge.Y; y++)
	for(int32_t x=m_area.MinEdge.X; x<=m_area.MaxEdge.X; x++)
	{
		uint8_t f = m_flags[m_area.index(x,y,z)];
		m_flags[m_area.index(x,y,z)] &= ~flags;
		if(m_flags[m_area.index(x,y,z)] != f)
			count++;
	}*/

	int32_t volume = m_area.getVolume();
	for(int32_t i=0; i<volume; i++)
	{
		m_flags[i] &= ~flags;
	}

	/*int32_t volume = m_area.getVolume();
	for(int32_t i=0; i<volume; i++)
	{
		uint8_t f = m_flags[i];
		m_flags[i] &= ~flags;
		if(m_flags[i] != f)
			count++;
	}

	dstream<<"clearFlag changed "<<count<<" flags out of "
			<<volume<<" nodes"<<std::endl;*/
}

const MapNode VoxelManipulator::ContentIgnoreNode = MapNode(CONTENT_IGNORE);

//END
