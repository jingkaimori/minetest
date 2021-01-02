/*
Minetest
Copyright (C) 2015 nerzhul, Loic Blot <loic.blot@unix-experience.fr>

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

#include "util/pointer.h"
#include "util/numeric.h"
#include "networkprotocol.h"
#include <SColor.h>

class NetworkPacket
{

public:
	NetworkPacket(uint16_t command, uint32_t datasize, session_t peer_id);
	NetworkPacket(uint16_t command, uint32_t datasize);
	NetworkPacket() = default;

	~NetworkPacket();

	void putRawPacket(uint8_t *data, uint32_t datasize, session_t peer_id);
	void clear();

	// Getters
	uint32_t getSize() const { return m_datasize; }
	session_t getPeerId() const { return m_peer_id; }
	uint16_t getCommand() { return m_command; }
	const uint32_t getRemainingBytes() const { return m_datasize - m_read_offset; }
	const char *getRemainingString() { return getString(m_read_offset); }

	// Returns a c-string without copying.
	// A better name for this would be getRawString()
	const char *getString(uint32_t from_offset);
	// major difference to putCString(): doesn't write len into the buffer
	void putRawString(const char *src, uint32_t len);
	void putRawString(const std::string &src)
	{
		putRawString(src.c_str(), src.size());
	}

	NetworkPacket &operator>>(std::string &dst);
	NetworkPacket &operator<<(const std::string &src);

	void putLongString(const std::string &src);

	NetworkPacket &operator>>(std::wstring &dst);
	NetworkPacket &operator<<(const std::wstring &src);

	std::string readLongString();

	NetworkPacket &operator>>(char &dst);
	NetworkPacket &operator<<(char src);

	NetworkPacket &operator>>(bool &dst);
	NetworkPacket &operator<<(bool src);

	uint8_t getU8(uint32_t offset);

	NetworkPacket &operator>>(uint8_t &dst);
	NetworkPacket &operator<<(uint8_t src);

	uint8_t *getU8Ptr(uint32_t offset);

	uint16_t getU16(uint32_t from_offset);
	NetworkPacket &operator>>(uint16_t &dst);
	NetworkPacket &operator<<(uint16_t src);

	NetworkPacket &operator>>(uint32_t &dst);
	NetworkPacket &operator<<(uint32_t src);

	NetworkPacket &operator>>(uint64_t &dst);
	NetworkPacket &operator<<(uint64_t src);

	NetworkPacket &operator>>(float &dst);
	NetworkPacket &operator<<(float src);

	NetworkPacket &operator>>(v2f &dst);
	NetworkPacket &operator<<(v2f src);

	NetworkPacket &operator>>(v3f &dst);
	NetworkPacket &operator<<(v3f src);

	NetworkPacket &operator>>(int16_t &dst);
	NetworkPacket &operator<<(int16_t src);

	NetworkPacket &operator>>(int32_t &dst);
	NetworkPacket &operator<<(int32_t src);

	NetworkPacket &operator>>(v2s32 &dst);
	NetworkPacket &operator<<(v2s32 src);

	NetworkPacket &operator>>(v3s16 &dst);
	NetworkPacket &operator<<(v3s16 src);

	NetworkPacket &operator>>(v3s32 &dst);
	NetworkPacket &operator<<(v3s32 src);

	NetworkPacket &operator>>(video::SColor &dst);
	NetworkPacket &operator<<(video::SColor src);

	// Temp, we remove SharedBuffer when migration finished
	SharedBuffer<u8> oldForgePacket();

private:
	void checkReadOffset(uint32_t from_offset, uint32_t field_size);

	inline void checkDataSize(uint32_t field_size)
	{
		if (m_read_offset + field_size > m_datasize) {
			m_datasize = m_read_offset + field_size;
			m_data.resize(m_datasize);
		}
	}

	std::vector<u8> m_data;
	uint32_t m_datasize = 0;
	uint32_t m_read_offset = 0;
	uint16_t m_command = 0;
	session_t m_peer_id = 0;
};
