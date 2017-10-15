#include "stdafx.h"
#include "DemoPlay_Control.h"
#include "Level.h"
#include "game_cl_base.h"

#pragma todo("FX to FX: Remove demoplay!")
demoplay_control::user_callback_t demoplay_control::no_user_callback;

demoplay_control::demoplay_control()
{
	m_onround_start			= fastdelegate::MakeDelegate(this, &demoplay_control::on_round_start_impl);
	m_on_kill				= fastdelegate::MakeDelegate(this, &demoplay_control::on_kill_impl);
	m_on_die				= fastdelegate::MakeDelegate(this, &demoplay_control::on_die_impl);
	m_on_artefactdelivering	= fastdelegate::MakeDelegate(this, &demoplay_control::on_artefactdelivering_impl);
	m_on_artefactcapturing	= fastdelegate::MakeDelegate(this, &demoplay_control::on_artefactcapturing_impl);
	m_on_artefactloosing	= fastdelegate::MakeDelegate(this, &demoplay_control::on_artefactloosing_impl);
	m_current_mode			= not_active;
}

demoplay_control::~demoplay_control()
{
}

void demoplay_control::pause_on(EAction const action, shared_str const & param)
{
	if (m_current_mode != not_active)
	{
		Msg("! ERROR: already active.");
		return;
	}
	if (Device.Paused())
	{
		Device.Pause(FALSE, TRUE, TRUE, "playing demo until");
	}
	m_current_mode	= waiting_for_actions;
	activate_filer	(action, param);
}

void demoplay_control::cancel_pause_on()
{
	if (m_current_mode != waiting_for_actions)
	{
		Msg("! ERROR: pause on is not active");
		return;
	}
	deactivate_filter();
	m_current_mode = not_active;
}

const float demoplay_control::rewind_speed = 8.f;
bool demoplay_control::rewind_until(EAction const action, shared_str const & param, user_callback_t ucb)
{
	if (m_current_mode != not_active)
	{
		Msg("! ERROR: already active.");
		return false;
	}
	if (Device.Paused())
	{
		Device.Pause(FALSE, TRUE, TRUE, "playing demo until");
	}
	m_prev_speed	= Level().GetDemoPlaySpeed();
	m_current_mode	= rewinding;
	activate_filer	(action, param);
	m_user_callback = ucb;
	Level().SetDemoPlaySpeed(rewind_speed);
	return true;
}
void demoplay_control::stop_rewind()
{
	if (m_current_mode != rewinding)
	{
		return;
	}
	deactivate_filter();
	Level().SetDemoPlaySpeed(m_prev_speed);
	m_current_mode = not_active;
}

void demoplay_control::activate_filer(EAction const action, shared_str const & param)
{
}
void demoplay_control::deactivate_filter()
{
}
void demoplay_control::process_action()
{
	if (m_current_mode == rewinding)
	{
		Level().SetDemoPlaySpeed(m_prev_speed);
	}
	Device.Pause(TRUE, TRUE, TRUE, "game action captured");
	deactivate_filter();
	if (m_user_callback)
		m_user_callback();
	m_current_mode = not_active;
}

void __stdcall	demoplay_control::on_round_start_impl(u32 message, u32 subtype, NET_Packet & packet)
{
	process_action();
}

void __stdcall	demoplay_control::on_kill_impl(u32 message, u32 subtype, NET_Packet & packet)
{
	u16 msg_type;
	packet.r_begin(msg_type);	
	R_ASSERT(msg_type == M_GAMEMESSAGE);
	u32 game_msg_type;
	packet.r_u32(game_msg_type);
	R_ASSERT(game_msg_type == GAME_EVENT_PLAYER_KILLED);
	
	if (!m_action_param_str.size())
	{
		process_action();
		return;
	}
	packet.r_u8();	//kill type
	packet.r_u16();	//killed_id
	u16 killer_id	= packet.r_u16();
}
void __stdcall	demoplay_control::on_die_impl(u32 message, u32 subtype, NET_Packet & packet)
{
	u16 msg_type;
	packet.r_begin(msg_type);	
	R_ASSERT(msg_type == M_GAMEMESSAGE);
	u32 game_msg_type;
	packet.r_u32(game_msg_type);
	R_ASSERT(game_msg_type == GAME_EVENT_PLAYER_KILLED);
	
	if (!m_action_param_str.size())
	{
		process_action();
		return;
	}
	packet.r_u8(); //kill type
	u16 killed_id	= packet.r_u16();
}
void __stdcall	demoplay_control::on_artefactdelivering_impl(u32 message, u32 subtype, NET_Packet & packet)
{
}

void __stdcall	demoplay_control::on_artefactcapturing_impl(u32 message, u32 subtype, NET_Packet & packet)
{

}
void __stdcall	demoplay_control::on_artefactloosing_impl(u32 message, u32 subtype, NET_Packet & packet)
{

}