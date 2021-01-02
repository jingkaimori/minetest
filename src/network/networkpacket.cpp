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

#include "networkpacket.h"
#include <sstream>
#include "networkexceptions.h"
#include "util/serialize.h"
#include "networkprotocol.h"

NetworkPacket::NetworkPacket(uint16_t command, uint32_t datasize, session_t peer_id):
m_datasize(datasize), m_command(command), m_peer_id(peer_id)
{
	m_data.resize(m_datasize);
}

NetworkPacket::NetworkPacket(uint16_t command, uint32_t datasize):
m_datasize(datasize), m_command(command)
{
	m_data.resize(m_datasize);
}

NetworkPacket::~NetworkPacket()
{
	m_data.clear();
}

void NetworkPacket::checkReadOffset(uint32_t from_offset, uint32_t field_size)
{
	if (from_offset + field_size > m_datasize) {
		std::stringstream ss;
		ss << "Reading outside packet (offset: " <<
				from_offset << ", packet size: " << getSize() << ")";
		throw PacketError(ss.str());
	}
}

void NetworkPacket::putRawPacket(uint8_t *data, uint32_t datasize, session_t peer_id)
{
	// If a m_command is already set, we are rewriting on same packet
	// This is not permitted
	assert(m_command == 0);

	m_datasize = datasize - 2;
	m_peer_id = peer_id;

	m_data.resize(m_datasize);

	// split command and datas
	m_command = readU16(&data[0]);
	memcpy(m_data.data(), &data[2], m_datasize);
}

void NetworkPacket::clear()
{
	m_data.clear();
	m_datasize = 0;
	m_read_offset = 0;
	m_command = 0;
	m_peer_id = 0;
}

const char* NetworkPacket::getString(uint32_t from_offset)
{
	checkReadOffset(from_offset, 0);

	return (char*)&m_data[from_offset];
}

void NetworkPacket::putRawString(const char* src, uint32_t len)
{
	if (m_read_offset + len > m_datasize) {
		m_datasize = m_read_offset + len;
		m_data.resize(m_datasize);
	}

	if (len == 0)
		return;

	memcpy(&m_data[m_read_offset], src, len);
	m_read_offset += len;
}

NetworkPacket& NetworkPacket::operator>>(std::string& dst)
{
	checkReadOffset(m_read_offset, 2);
	uint16_t strLen = readU16(&m_data[m_read_offset]);
	m_read_offset += 2;

	dst.clear();

	if (strLen == 0) {
		return *this;
	}

	checkReadOffset(m_read_offset, strLen);

	dst.reserve(strLen);
	dst.append((char*)&m_data[m_read_offset], strLen);

	m_read_offset += strLen;
	return *this;
}

NetworkPacket& NetworkPacket::operator<<(const std::string &src)
{
	if (src.size() > STRING_MAX_LEN) {
		throw PacketError("String too long");
	}

	uint16_t msgsize = src.size();

	*this << msgsize;

	putRawString(src.c_str(), (uint32_t)msgsize);

	return *this;
}

void NetworkPacket::putLongString(const std::string &src)
{
	if (src.size() > LONG_STRING_MAX_LEN) {
		throw PacketError("String too long");
	}

	uint32_t msgsize = src.size();

	*this << msgsize;

	putRawString(src.c_str(), msgsize);
}

NetworkPacket& NetworkPacket::operator>>(std::wstring& dst)
{
	checkReadOffset(m_read_offset, 2);
	uint16_t strLen = readU16(&m_data[m_read_offset]);
	m_read_offset += 2;

	dst.clear();

	if (strLen == 0) {
		return *this;
	}

	checkReadOffset(m_read_offset, strLen * 2);

	dst.reserve(strLen);
	for(uint16_t i=0; i<strLen; i++) {
		wchar_t c16 = readU16(&m_data[m_read_offset]);
		dst.append(&c16, 1);
		m_read_offset += sizeof(uint16_t);
	}

	return *this;
}

NetworkPacket& NetworkPacket::operator<<(const std::wstring &src)
{
	if (src.size() > WIDE_STRING_MAX_LEN) {
		throw PacketError("String too long");
	}

	uint16_t msgsize = src.size();

	*this << msgsize;

	// Write string
	for (uint16_t i=0; i<msgsize; i++) {
		*this << (uint16_t) src[i];
	}

	return *this;
}

std::string NetworkPacket::readLongString()
{
	checkReadOffset(m_read_offset, 4);
	uint32_t strLen = readU32(&m_data[m_read_offset]);
	m_read_offset += 4;

	if (strLen == 0) {
		return "";
	}

	if (strLen > LONG_STRING_MAX_LEN) {
		throw PacketError("String too long");
	}

	checkReadOffset(m_read_offset, strLen);

	std::string dst;

	dst.reserve(strLen);
	dst.append((char*)&m_data[m_read_offset], strLen);

	m_read_offset += strLen;

	return dst;
}

NetworkPacket& NetworkPacket::operator>>(char& dst)
{
	checkReadOffset(m_read_offset, 1);

	dst = readU8(&m_data[m_read_offset]);

	m_read_offset += 1;
	return *this;
}

NetworkPacket& NetworkPacket::operator<<(char src)
{
	checkDataSize(1);

	writeU8(&m_data[m_read_offset], src);

	m_read_offset += 1;
	return *this;
}

NetworkPacket& NetworkPacket::operator<<(uint8_t src)
{
	checkDataSize(1);

	writeU8(&m_data[m_read_offset], src);

	m_read_offset += 1;
	return *this;
}

NetworkPacket& NetworkPacket::operator<<(bool src)
{
	checkDataSize(1);

	writeU8(&m_data[m_read_offset], src);

	m_read_offset += 1;
	return *this;
}

NetworkPacket& NetworkPacket::operator<<(uint16_t src)
{
	checkDataSize(2);

	writeU16(&m_data[m_read_offset], src);

	m_read_offset += 2;
	return *this;
}

NetworkPacket& NetworkPacket::operator<<(uint32_t src)
{
	checkDataSize(4);

	writeU32(&m_data[m_read_offset], src);

	m_read_offset += 4;
	return *this;
}

NetworkPacket& NetworkPacket::operator<<(uint64_t src)
{
	checkDataSize(8);

	writeU64(&m_data[m_read_offset], src);

	m_read_offset += 8;
	return *this;
}

NetworkPacket& NetworkPacket::operator<<(float src)
{
	checkDataSize(4);

	writeF32(&m_data[m_read_offset], src);

	m_read_offset += 4;
	return *this;
}

NetworkPacket& NetworkPacket::operator>>(bool& dst)
{
	checkReadOffset(m_read_offset, 1);

	dst = readU8(&m_data[m_read_offset]);

	m_read_offset += 1;
	return *this;
}

NetworkPacket& NetworkPacket::operator>>(uint8_t& dst)
{
	checkReadOffset(m_read_offset, 1);

	dst = readU8(&m_data[m_read_offset]);

	m_read_offset += 1;
	return *this;
}

uint8_t NetworkPacket::getU8(uint32_t offset)
{
	checkReadOffset(offset, 1);

	return readU8(&m_data[offset]);
}

u8* NetworkPacket::getU8Ptr(uint32_t from_offset)
{
	if (m_datasize == 0) {
		return NULL;
	}

	checkReadOffset(from_offset, 1);

	return (uint8_t*)&m_data[from_offset];
}

NetworkPacket& NetworkPacket::operator>>(uint16_t& dst)
{
	checkReadOffset(m_read_offset, 2);

	dst = readU16(&m_data[m_read_offset]);

	m_read_offset += 2;
	return *this;
}

u16 NetworkPacket::getU16(uint32_t from_offset)
{
	checkReadOffset(from_offset, 2);

	return readU16(&m_data[from_offset]);
}

NetworkPacket& NetworkPacket::operator>>(uint32_t& dst)
{
	checkReadOffset(m_read_offset, 4);

	dst = readU32(&m_data[m_read_offset]);

	m_read_offset += 4;
	return *this;
}

NetworkPacket& NetworkPacket::operator>>(uint64_t& dst)
{
	checkReadOffset(m_read_offset, 8);

	dst = readU64(&m_data[m_read_offset]);

	m_read_offset += 8;
	return *this;
}

NetworkPacket& NetworkPacket::operator>>(float& dst)
{
	checkReadOffset(m_read_offset, 4);

	dst = readF32(&m_data[m_read_offset]);

	m_read_offset += 4;
	return *this;
}

NetworkPacket& NetworkPacket::operator>>(v2f& dst)
{
	checkReadOffset(m_read_offset, 8);

	dst = readV2F32(&m_data[m_read_offset]);

	m_read_offset += 8;
	return *this;
}

NetworkPacket& NetworkPacket::operator>>(v3f& dst)
{
	checkReadOffset(m_read_offset, 12);

	dst = readV3F32(&m_data[m_read_offset]);

	m_read_offset += 12;
	return *this;
}

NetworkPacket& NetworkPacket::operator>>(s16& dst)
{
	checkReadOffset(m_read_offset, 2);

	dst = readS16(&m_data[m_read_offset]);

	m_read_offset += 2;
	return *this;
}

NetworkPacket& NetworkPacket::operator<<(int16_t src)
{
	*this << (uint16_t) src;
	return *this;
}

NetworkPacket& NetworkPacket::operator>>(s32& dst)
{
	checkReadOffset(m_read_offset, 4);

	dst = readS32(&m_data[m_read_offset]);

	m_read_offset += 4;
	return *this;
}

NetworkPacket& NetworkPacket::operator<<(int32_t src)
{
	*this << (uint32_t) src;
	return *this;
}

NetworkPacket& NetworkPacket::operator>>(v3s16& dst)
{
	checkReadOffset(m_read_offset, 6);

	dst = readV3S16(&m_data[m_read_offset]);

	m_read_offset += 6;
	return *this;
}

NetworkPacket& NetworkPacket::operator>>(v2s32& dst)
{
	checkReadOffset(m_read_offset, 8);

	dst = readV2S32(&m_data[m_read_offset]);

	m_read_offset += 8;
	return *this;
}

NetworkPacket& NetworkPacket::operator>>(v3s32& dst)
{
	checkReadOffset(m_read_offset, 12);

	dst = readV3S32(&m_data[m_read_offset]);

	m_read_offset += 12;
	return *this;
}

NetworkPacket& NetworkPacket::operator<<(v2f src)
{
	*this << (float) src.X;
	*this << (float) src.Y;
	return *this;
}

NetworkPacket& NetworkPacket::operator<<(v3f src)
{
	*this << (float) src.X;
	*this << (float) src.Y;
	*this << (float) src.Z;
	return *this;
}

NetworkPacket& NetworkPacket::operator<<(v3s16 src)
{
	*this << (s16) src.X;
	*this << (s16) src.Y;
	*this << (s16) src.Z;
	return *this;
}

NetworkPacket& NetworkPacket::operator<<(v2s32 src)
{
	*this << (s32) src.X;
	*this << (s32) src.Y;
	return *this;
}

NetworkPacket& NetworkPacket::operator<<(v3s32 src)
{
	*this << (s32) src.X;
	*this << (s32) src.Y;
	*this << (s32) src.Z;
	return *this;
}

NetworkPacket& NetworkPacket::operator>>(video::SColor& dst)
{
	checkReadOffset(m_read_offset, 4);

	dst = readARGB8(&m_data[m_read_offset]);

	m_read_offset += 4;
	return *this;
}

NetworkPacket& NetworkPacket::operator<<(video::SColor src)
{
	checkDataSize(4);

	writeU32(&m_data[m_read_offset], src.color);

	m_read_offset += 4;
	return *this;
}

SharedBuffer<uint8_t> NetworkPacket::oldForgePacket()
{
	SharedBuffer<uint8_t> sb(m_datasize + 2);
	writeU16(&sb[0], m_command);

	uint8_t* datas = getU8Ptr(0);

	if (datas != NULL)
		memcpy(&sb[2], datas, m_datasize);
	return sb;
}
