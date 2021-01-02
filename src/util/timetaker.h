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

#pragma once

#include "irrlichttypes.h"
#include "gettime.h"

/*
	TimeTaker
*/

class TimeTaker
{
public:
	TimeTaker(const std::string &name, uint64_t *result=nullptr,
		TimePrecision prec=PRECISION_MILLI);

	~TimeTaker()
	{
		stop();
	}

	uint64_t stop(bool quiet=false);

	uint64_t getTimerTime();

private:
	std::string m_name;
	uint64_t m_time1;
	bool m_running = true;
	TimePrecision m_precision;
	uint64_t *m_result = nullptr;
};
