#include "stdafx.h"
#include "level.h"
#include "../xrEngine/x_ray.h"
#include "../xrEngine/igame_persistent.h"

#include "ai_space.h"
#include "game_cl_base.h"
#include "NET_Queue.h"
#include "file_transfer.h"
#include "hudmanager.h"

#include "../xrphysics/iphworld.h"

#include "phcommander.h"
#include "physics_game.h"
#include "GamePersistent.h"

extern	pureFrame*				g_pNetProcessor;

BOOL CLevel::net_Start_client	( LPCSTR options )
{
	return FALSE;
}
#include "string_table.h"
bool	CLevel::net_start_client1				()
{
	pApp->LoadBegin	();
	// name_of_server
	string64					name_of_server = "";
    shared_str clientOption = GamePersistent().GetClientOption();
	if (strchr(*clientOption, '/'))
		strncpy_s(name_of_server,*clientOption, strchr(*clientOption, '/')-*clientOption);

	if (strchr(name_of_server,'/'))	*strchr(name_of_server,'/') = 0;

	// Startup client
	g_pGamePersistent->LoadTitle();
	return true;
}

#include "game_sv_Single.h"

bool	CLevel::net_start_client2()
{
//	Server->create_direct_client();
	//offline account creation
	m_bConnectResultReceived = false;
	while (!m_bConnectResultReceived)
	{
		ClientReceive();
	//	Server->Update();
	}

    shared_str clientOption = GamePersistent().GetClientOption();
	connected_to_server = Connect2Server(*clientOption);

	return true;
}

bool	CLevel::net_start_client3				()
{
	if(connected_to_server)
	{
		//shared_str const & server_options = Server->GetConnectOptions();

		LPCSTR					level_name = name().c_str();
		//LPCSTR					level_ver = Server->level_version(server_options).c_str();

		// Determine internal level-ID
		int						level_id = pApp->Level_ID(level_name, map_data.m_map_version.c_str(), true);
		
		map_data.m_name					= level_name;
		//map_data.m_map_version			= level_ver;
		map_data.m_map_loaded			= true;
		
		deny_m_spawn			= FALSE;
		// Load level
		R_ASSERT2				(Load(level_id),"Loading failed.");
		map_data.m_level_geom_crc32 = 0;
	}
	return true;
}

bool	CLevel::net_start_client4				()
{
	if (connected_to_server) {
		// Begin spawn
		g_pGamePersistent->LoadTitle();

		// Send physics to single or multithreaded mode

		create_physics_world(!!psDeviceFlags.test(mtPhysics), &ObjectSpace, &Objects, &Device);

		R_ASSERT(physics_world());

		m_ph_commander_physics_worldstep = xr_new<CPHCommander>();
		physics_world()->set_update_callback(m_ph_commander_physics_worldstep);

		physics_world()->set_default_contact_shotmark(ContactShotMark);
		physics_world()->set_default_character_contact_shotmark(CharacterContactShotMark);

		VERIFY(physics_world());
		physics_world()->set_step_time_callback((PhysicsStepTimeCallback*)&PhisStepsCallback);


		// Send network to single or multithreaded mode
		// *note: release version always has "mt_*" enabled
		Device.seqFrameMT.Remove(g_pNetProcessor);
		Device.seqFrame.Remove(g_pNetProcessor);
		if (psDeviceFlags.test(mtNetwork))	Device.seqFrameMT.Add(g_pNetProcessor, REG_PRIORITY_HIGH + 2);
		else								Device.seqFrame.Add(g_pNetProcessor, REG_PRIORITY_LOW - 2);
	}
	return true;
}

bool CLevel::net_start_client5()
{
	if(connected_to_server)
	{
		// HUD
		// Textures
		if	(!g_dedicated_server)
		{
			g_pGamePersistent->LoadTitle		();
			Device.m_pRender->DeferredLoad		(FALSE);
			Device.m_pRender->ResourcesDeferredUpload();
			LL_CheckTextures					();
		}
		sended_request_connection_data	= FALSE;
		deny_m_spawn					= TRUE;
	}
	return true;
}

bool CLevel::net_start_client6()
{
	if (connected_to_server) 
	{
		// Sync
		if (!synchronize_map_data())
			return false;

		if (!game_configured)
		{
			pApp->LoadEnd(); 
			return true;
		}
		if (!g_dedicated_server)
		{
			g_hud->Load						();
			g_hud->OnConnected				();
		}

		if (game)
			game->OnConnected();

		g_pGamePersistent->LoadTitle		();
		Device.PreCache						(60, true, true);
		net_start_result_total				= TRUE;
	}
	else net_start_result_total				= FALSE;

	pApp->LoadEnd							(); 
	return true;
}
