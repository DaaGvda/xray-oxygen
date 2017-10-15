#include "stdafx.h"
#include "game_cl_base.h"
#include "level.h"
#include "UIGameCustom.h"
#include "script_engine.h"

game_cl_GameState::game_cl_GameState()
{
	shedule.t_min				= 5;
	shedule.t_max				= 20;
	m_game_ui_custom			= nullptr;
	shedule_register			();
	m_bServerControlHits		= true;
}

game_cl_GameState::~game_cl_GameState()
{
	shedule_unregister();
}

void game_cl_GameState::shedule_Update(u32 dt)
{
	ISheduled::shedule_Update(dt);

	if (!m_game_ui_custom)
	{
		if (CurrentGameUI())
			m_game_ui_custom = CurrentGameUI();
	}
};

void game_cl_GameState::sv_GameEventGen(NET_Packet& P)
{
	P.w_begin	(M_EVENT);
	P.w_u32		(Level().timeServer());
	P.w_u16		( u16(GE_GAME_EVENT&0xffff) );
	P.w_u16		(0);//dest==0
}

void	game_cl_GameState::sv_EventSend(NET_Packet& P)
{
	Level().Send(P,net_flags(TRUE,TRUE));
}

void game_cl_GameState::u_EventGen(NET_Packet& P, u16 type, u16 dest)
{
	P.w_begin	(M_EVENT);
	P.w_u32		(Level().timeServer());
	P.w_u16		(type);
	P.w_u16		(dest);
}

void game_cl_GameState::u_EventSend(NET_Packet& P)
{
	Level().Send(P,net_flags(TRUE,TRUE));
}

void game_cl_GameState::OnConnected()
{
	m_game_ui_custom = CurrentGameUI();
}
