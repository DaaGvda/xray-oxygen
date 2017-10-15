#include "stdafx.h"
#include "level.h"
#include "DemoInfo.h"
#include "../xrCore/stream_reader.h"
#include "object_broker.h"
#include "game_cl_base.h"

u32 const demo_player_info::demo_info_max_size = DEMOSTRING_MAX_SIZE + 80;

demo_player_info::demo_player_info()
{
}

demo_player_info::~demo_player_info()
{
}

void demo_player_info::read_from_file(CStreamReader* file_to_read)
{
	file_to_read->r_stringZ	(m_name);
}

void demo_player_info::write_to_file(IWriter* file_to_write) const
{
	file_to_write->w_stringZ(m_name);
}

void demo_player_info::load_from_player(const char* player_state)
{
	m_name		= player_state ? player_state : Core.UserName;
}

u32 const demo_info::max_demo_info_size = 
	(demo_player_info::demo_info_max_size) +
	(DEMOSTRING_MAX_SIZE * 5) + sizeof(u32);

demo_info::demo_info()
{
}

demo_info::~demo_info()
{
	delete_data				(m_players);
}

void demo_info::read_from_file(CStreamReader* file_to_read)
{
	file_to_read->r_stringZ	(m_map_name);
	file_to_read->r_stringZ	(m_map_version);
}
void demo_info::write_to_file(IWriter* file_to_write) const
{
	file_to_write->w_stringZ	(m_map_name);
	file_to_write->w_stringZ	(m_map_version);
}
void demo_info::sort_players(sorting_less_comparator sorting_comparator)
{
	std::sort(m_players.begin(), m_players.end(), sorting_comparator);
}

void demo_info::load_from_game()
{
	m_map_name = Level().name();
	m_map_version = Level().version();

	m_game_type = "single";
	m_game_score = "";
	m_author_name = "unknown";

	delete_data(m_players);
	m_players.reserve(1);
}

demo_player_info const * demo_info::get_player(u32 player_index) const
{
	R_ASSERT(player_index < m_players.size());
	return m_players[player_index];
}


using namespace luabind;

#pragma optimize("s",on)
void demo_player_info::script_register(lua_State *L)
{
	module(L)
	[
		class_<demo_player_info>("demo_player_info")
			.def("get_name",		&demo_player_info::get_name)
	];
}

void demo_info::script_register(lua_State *L)
{
	module(L)
	[
		class_<demo_info>("demo_info")
			.def("get_map_name",		&demo_info::get_map_name)
			.def("get_map_version",		&demo_info::get_map_version)
	];
}
