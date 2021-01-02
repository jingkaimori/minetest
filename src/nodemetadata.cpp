/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#include "nodemetadata.h"
#include "exceptions.h"
#include "gamedef.h"
#include "inventory.h"
#include "log.h"
#include "util/serialize.h"
#include "util/basic_macros.h"
#include "constants.h" // MAP_BLOCKSIZE
#include <sstream>

/*
	NodeMetadata
*/

NodeMetadata::NodeMetadata(IItemDefManager *item_def_mgr):
	m_inventory(new Inventory(item_def_mgr))
{}

NodeMetadata::~NodeMetadata()
{
	delete m_inventory;
}

void NodeMetadata::serialize(std::ostream &os, uint8_t version, bool disk) const
{
	int num_vars = disk ? m_stringvars.size() : countNonPrivate();
	writeU32(os, num_vars);
	for (const auto &sv : m_stringvars) {
		bool priv = isPrivate(sv.first);
		if (!disk && priv)
			continue;

		os << serializeString16(sv.first);
		os << serializeString32(sv.second);
		if (version >= 2)
			writeU8(os, (priv) ? 1 : 0);
	}

	m_inventory->serialize(os);
}

void NodeMetadata::deSerialize(std::istream &is, uint8_t version)
{
	clear();
	int num_vars = readU32(is);
	for(int i=0; i<num_vars; i++){
		std::string name = deSerializeString16(is);
		std::string var = deSerializeString32(is);
		m_stringvars[name] = var;
		if (version >= 2) {
			if (readU8(is) == 1)
				markPrivate(name, true);
		}
	}

	m_inventory->deSerialize(is);
}

void NodeMetadata::clear()
{
	Metadata::clear();
	m_privatevars.clear();
	m_inventory->clear();
}

bool NodeMetadata::empty() const
{
	return Metadata::empty() && m_inventory->getLists().empty();
}


void NodeMetadata::markPrivate(const std::string &name, bool set)
{
	if (set)
		m_privatevars.insert(name);
	else
		m_privatevars.erase(name);
}

int NodeMetadata::countNonPrivate() const
{
	// m_privatevars can contain names not actually present
	// DON'T: return m_stringvars.size() - m_privatevars.size();
	int n = 0;
	for (const auto &sv : m_stringvars) {
		if (!isPrivate(sv.first))
			n++;
	}
	return n;
}

/*
	NodeMetadataList
*/

void NodeMetadataList::serialize(std::ostream &os, uint8_t blockver, bool disk,
	bool absolute_pos) const
{
	/*
		Version 0 is a placeholder for "nothing to see here; go away."
	*/

	uint16_t count = countNonEmpty();
	if (count == 0) {
		writeU8(os, 0); // version
		return;
	}

	uint8_t version = (blockver > 27) ? 2 : 1;
	writeU8(os, version);
	writeU16(os, count);

	for (NodeMetadataMap::const_iterator
			i = m_data.begin();
			i != m_data.end(); ++i) {
		v3s16 p = i->first;
		NodeMetadata *data = i->second;
		if (data->empty())
			continue;

		if (absolute_pos) {
			writeS16(os, p.X);
			writeS16(os, p.Y);
			writeS16(os, p.Z);
		} else {
			// Serialize positions within a mapblock
			uint16_t p16 = (p.Z * MAP_BLOCKSIZE + p.Y) * MAP_BLOCKSIZE + p.X;
			writeU16(os, p16);
		}
		data->serialize(os, version, disk);
	}
}

void NodeMetadataList::deSerialize(std::istream &is,
	IItemDefManager *item_def_mgr, bool absolute_pos)
{
	clear();

	uint8_t version = readU8(is);

	if (version == 0) {
		// Nothing
		return;
	}

	if (version > 2) {
		std::string err_str = std::string(FUNCTION_NAME)
			+ ": version " + itos(version) + " not supported";
		infostream << err_str << std::endl;
		throw SerializationError(err_str);
	}

	uint16_t count = readU16(is);

	for (uint16_t i = 0; i < count; i++) {
		v3s16 p;
		if (absolute_pos) {
			p.X = readS16(is);
			p.Y = readS16(is);
			p.Z = readS16(is);
		} else {
			uint16_t p16 = readU16(is);
			p.X = p16 & (MAP_BLOCKSIZE - 1);
			p16 /= MAP_BLOCKSIZE;
			p.Y = p16 & (MAP_BLOCKSIZE - 1);
			p16 /= MAP_BLOCKSIZE;
			p.Z = p16;
		}
		if (m_data.find(p) != m_data.end()) {
			warningstream << "NodeMetadataList::deSerialize(): "
					<< "already set data at position " << PP(p)
					<< ": Ignoring." << std::endl;
			continue;
		}

		NodeMetadata *data = new NodeMetadata(item_def_mgr);
		data->deSerialize(is, version);
		m_data[p] = data;
	}
}

NodeMetadataList::~NodeMetadataList()
{
	clear();
}

std::vector<v3s16> NodeMetadataList::getAllKeys()
{
	std::vector<v3s16> keys;

	NodeMetadataMap::const_iterator it;
	for (it = m_data.begin(); it != m_data.end(); ++it)
		keys.push_back(it->first);

	return keys;
}

NodeMetadata *NodeMetadataList::get(v3s16 p)
{
	NodeMetadataMap::const_iterator n = m_data.find(p);
	if (n == m_data.end())
		return NULL;
	return n->second;
}

void NodeMetadataList::remove(v3s16 p)
{
	NodeMetadata *olddata = get(p);
	if (olddata) {
		if (m_is_metadata_owner)
			delete olddata;
		m_data.erase(p);
	}
}

void NodeMetadataList::set(v3s16 p, NodeMetadata *d)
{
	remove(p);
	m_data.insert(std::make_pair(p, d));
}

void NodeMetadataList::clear()
{
	if (m_is_metadata_owner) {
		NodeMetadataMap::const_iterator it;
		for (it = m_data.begin(); it != m_data.end(); ++it)
			delete it->second;
	}
	m_data.clear();
}

int NodeMetadataList::countNonEmpty() const
{
	int n = 0;
	NodeMetadataMap::const_iterator it;
	for (it = m_data.begin(); it != m_data.end(); ++it) {
		if (!it->second->empty())
			n++;
	}
	return n;
}
