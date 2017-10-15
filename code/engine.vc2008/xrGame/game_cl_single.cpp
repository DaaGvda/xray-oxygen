#include "stdafx.h"
#include "game_cl_single.h"
#include "UIGameSP.h"
#include "actor.h"
#include "clsid_game.h"
#include "Level.h"

using namespace luabind;

ESingleGameDifficulty g_SingleGameDifficulty = egdStalker;

xr_token	difficulty_type_token[] = {
	{ "gd_novice",	 egdNovice	},
	{ "gd_stalker",	 egdStalker	},
	{ "gd_veteran",	 egdVeteran	},
	{ "gd_master",	 egdMaster	},
	{ 0,			 0			}};

void game_cl_Single::SendPickUpEvent(u16 ID_who, u16 ID_what)
{
	CObject* O = Level().Objects.net_Find(ID_what);
	Level().m_feel_deny.feel_touch_deny(O, 1000);

	NET_Packet		P;
	u_EventGen(P, GE_OWNERSHIP_TAKE, ID_who);
	P.w_u16(ID_what);
	u_EventSend(P);
};

void game_cl_Single::SetEnvironmentGameTimeFactor(u64 GameTime, const float fTimeFactor)
{
	m_qwEStartGameTime = GameTime;
	m_qwEStartProcessorTime = Level().timeServer_Async();
	m_fETimeFactor = fTimeFactor;
}

void game_cl_Single::SetGameTimeFactor(u64 GameTime, const float fTimeFactor)
{
	m_qwStartGameTime = GameTime;
	m_qwStartProcessorTime = Level().timeServer_Async();
	m_fTimeFactor = fTimeFactor;
}

game_cl_Single::game_cl_Single()
{
}

CUIGameCustom* game_cl_Single::createGameUI()
{
	CLASS_ID clsid			= CLSID_GAME_UI_SINGLE;
	CUIGameSP*	pUIGame		= smart_cast<CUIGameSP*> ( NEW_INSTANCE ( clsid ) );
	R_ASSERT				(pUIGame);
	pUIGame->Load			();
	pUIGame->SetClGame		(this);
	pUIGame->Init			(0);
	pUIGame->Init			(1);
	pUIGame->Init			(2);
	return					pUIGame;
}

void game_cl_Single::OnDifficultyChanged()
{
	Actor()->OnDifficultyChanged();
}

#include "ai_space.h"
#include "alife_simulator.h"
#include "alife_time_manager.h"
u64 game_cl_Single::GetGameTime()
{
	return(ai().alife().time_manager().game_time());
}

u64 game_cl_Single::GetStartGameTime()
{
	return(ai().alife().time_manager().start_game_time());
}

float game_cl_Single::GetGameTimeFactor()
{
	return(ai().alife().time_manager().time_factor());
}

void game_cl_Single::SetGameTimeFactor(const float fTimeFactor)
{
	const u64 GameTime = m_qwStartGameTime + m_fTimeFactor*float(Level().timeServer_Async() - m_qwStartProcessorTime);
	SetGameTimeFactor(GameTime, fTimeFactor);
}

u64 game_cl_Single::GetEnvironmentGameTime	()
{
		return	(ai().alife().time_manager().game_time());
}

float game_cl_Single::GetEnvironmentGameTimeFactor		()
{
		return	(ai().alife().time_manager().time_factor());
}

void game_cl_Single::SetEnvironmentGameTimeFactor(const float fTimeFactor)
{
	const u64 GameTime = m_qwStartGameTime + m_fTimeFactor*float(Level().timeServer_Async() - m_qwStartProcessorTime);
	SetGameTimeFactor(GameTime, fTimeFactor);
}

#pragma optimize("s",on)
void CScriptGameDifficulty::script_register(lua_State *L)
{
	module(L)
		[
			class_<enum_exporter<ESingleGameDifficulty> >("game_difficulty")
			.enum_("game_difficulty")
			[
				value("novice",				int(egdNovice			)),
				value("stalker",			int(egdStalker			)),
				value("veteran",			int(egdVeteran			)),
				value("master",				int(egdMaster			))
			]
		];
}