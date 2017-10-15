#include "stdafx.h"
#include "Level.h"
#include "Level_Bullet_Manager.h"
#include "game_sv_Single.h"
#include "xrmessages.h"
#include "game_cl_base.h"
#include "PHCommander.h"
#include "net_queue.h"
#include "MainMenu.h"
#include "space_restriction_manager.h"
#include "ai_space.h"
#include "script_engine.h"
#include "stalker_animation_data_storage.h"
#include "client_spawn_manager.h"
#include "seniority_hierarchy_holder.h"
#include "UIGameCustom.h"
#include "string_table.h"
#include "file_transfer.h"
#include "UI/UIGameTutorial.h"
#include "ui/UIPdaWnd.h"
#include "GamePersistent.h"

#include "../xrphysics/physicscommon.h"
ENGINE_API bool g_dedicated_server;

const int max_objects_size			= 2*1024;
const int max_objects_size_in_save	= 8*1024;

extern bool	g_b_ClearGameCaptions;

void CLevel::remove_objects()
{
	BOOL						b_stored = psDeviceFlags.test(rsDisableObjectsAsCrows);

	int loop = 5;
	while (loop)
	{
		//Server->SLS_Clear();
		for (int i = 0; i < 20; ++i)
		{
			snd_Events.clear();
			psNET_Flags.set(NETFLAG_MINIMIZEUPDATES, FALSE);
			// ugly hack for checks that update is twice on frame
			// we need it since we do updates for checking network messages
			++(Device.dwFrame);
			psDeviceFlags.set(rsDisableObjectsAsCrows, TRUE);
			ClientReceive();
			ProcessGameEvents();
			Objects.Update(false);
			Objects.dump_all_objects();
		}

		if (Objects.o_count())
		{
			--loop;
			Msg("Objects removal next loop. Active objects count=%d", Objects.o_count());
		}
		else break;
	}

	BulletManager().Clear		();
	ph_commander().clear		();
	ph_commander_scripts().clear();

	if(!g_dedicated_server)
		space_restriction_manager().clear	();

	psDeviceFlags.set			(rsDisableObjectsAsCrows, b_stored);
	g_b_ClearGameCaptions		= true;

	if (!g_dedicated_server)
		ai().script_engine().collect_all_garbage	();

	stalker_animation_data_storage().clear		();
	
	VERIFY										(Render);
	Render->models_Clear						(FALSE);
	
	Render->clear_static_wallmarks				();

#ifdef DEBUG
	if(!g_dedicated_server)
		if (!client_spawn_manager().registry().empty())
			client_spawn_manager().dump				();
#endif // DEBUG
	if(!g_dedicated_server)
	{
		VERIFY										(client_spawn_manager().registry().empty());
		client_spawn_manager().clear			();
	}

	g_pGamePersistent->destroy_particles		(false);
}

#ifdef DEBUG
	extern void	show_animation_stats	();
#endif // DEBUG

extern CUISequencer * g_tutorial;
extern CUISequencer * g_tutorial2;

void CLevel::net_Stop		()
{
	Msg							("- Disconnect");

	if(CurrentGameUI())
	{
		CurrentGameUI()->HideShownDialogs();
		CurrentGameUI()->PdaMenu().Reset();
	}

	if(g_tutorial && !g_tutorial->Persistent())
		g_tutorial->Stop();

	if(g_tutorial2 && !g_tutorial->Persistent())
		g_tutorial2->Stop();

	bReady						= false;
	m_bGameConfigStarted		= FALSE;

	if (m_file_transfer)
		xr_delete(m_file_transfer);

	if (IsDemoPlay() && m_current_spectator)	//destroying demo spectator ...
	{
		m_current_spectator->setDestroy	(TRUE);
		SetControlEntity(NULL); //m_current_spectator == CurrentControlEntity()
		m_current_spectator = NULL;
		
	}else 
	if(IsDemoSave() && !IsDemoInfoSaved())
		SaveDemoInfo();

	remove_objects				();
	
	//WARNING ! remove_objects() uses this flag, so position of this line must e here ..
	game_configured				= FALSE;
	
	IGame_Level::net_Stop		();
	IPureClient::Disconnect		();

	/*if (Server) 
	{
		Server->Disconnect		();
		xr_delete				(Server);
	}
	*/
	ai().script_engine().collect_all_garbage();

#ifdef DEBUG
	show_animation_stats		();
#endif // DEBUG
}

void CLevel::ClientSend(bool bForce)
{
	static u32 cur_index = 0;

	if (bForce)
		cur_index = 0;

	u32 object_count = Objects.o_count();
	u32 position;
	for (u32 start = cur_index; start < (bForce ? object_count : cur_index + 20); start++)
	{
		CObject	*pO = Objects.o_get_by_iterator(cur_index);

		cur_index++;
		if (cur_index >= object_count)
			cur_index = 0;

		if (pO && !pO->getDestroy() && pO->net_Relevant())
		{
			NET_Packet P;
			P.w_begin(M_UPDATE);

			P.w_u16(u16(pO->ID()));
			P.w_chunk_open8(position);

			pO->net_Export(P);

			P.w_chunk_close8(position);
			if (max_objects_size >= (NET_PacketSizeLimit - P.w_tell()))
				continue;
			Send(P, net_flags(FALSE));
		}
	}
}

u32	CLevel::Objects_net_Save	(NET_Packet* _Packet, u32 start, u32 max_object_size)
{
	NET_Packet& Packet	= *_Packet;
	u32			position;
	for (; start<Objects.o_count(); start++)	{
		CObject		*_P = Objects.o_get_by_iterator(start);
		CGameObject *P = smart_cast<CGameObject*>(_P);
		if (P && !P->getDestroy() && P->net_SaveRelevant())	{
			Packet.w_u16			(u16(P->ID())	);
			Packet.w_chunk_open16	(position);
			P->net_Save				(Packet);
			Packet.w_chunk_close16	(position);
			if (max_object_size >= (NET_PacketSizeLimit - Packet.w_tell()))
				break;
		}
	}
	return	++start;
}

void CLevel::ClientSave()
{
	u32 position;
	for (u32 start = 0; start < Objects.o_count(); start++)
	{
		CObject	*pO = Objects.o_get_by_iterator(start);
		CGameObject *pGO = smart_cast<CGameObject*>(pO);

		if (pGO && !pGO->getDestroy() && pGO->net_SaveRelevant())
		{
			NET_Packet P;
			P.w_begin(M_SAVE_PACKET);

			P.w_u16(pGO->ID());
			P.w_chunk_open16(position);

			pGO->net_Save(P);
			P.w_chunk_close16(position);
			if (max_objects_size >= (NET_PacketSizeLimit - P.w_tell()))
				continue;
			Send(P, net_flags(FALSE));
		}
	}
}

void CLevel::Send(NET_Packet& P, u32 dwFlags, u32 dwTimeout)
{
	if (!IsDemoPlayStarted() && !IsDemoPlayFinished())
	{
		// optimize the case when server located in our memory

		ClientID	_clid;
		_clid.set(1);
		//Server->OnMessage(P, _clid);
	}
}

void CLevel::net_Update()
{
	if(game_configured)
	{
		// If we have enought bandwidth - replicate client data on to server
		Device.Statistic->netClient2.Begin	();
		ClientSend					();
		Device.Statistic->netClient2.End		();
	}
	// If server - perform server-update
	if (Server)
	{
		Device.Statistic->netServer.Begin();
		Server->Update					();
		Device.Statistic->netServer.End	();
	}
}

struct _NetworkProcessor : public pureFrame
{
	virtual void _BCL OnFrame()
	{
		if (g_pGameLevel && !Device.Paused() )	g_pGameLevel->net_Update();
	}
}	NET_processor;

pureFrame*	g_pNetProcessor	= &NET_processor;


bool CLevel::Connect2Server(const char* options)
{
	m_bConnectResultReceived	= true	;
	m_bConnectResult			= true	;

	if (!Connect(options))		
		return false;
	//---------------------------------------------------------------------------

	Msg("%c client : connection %s - <%s>", m_bConnectResult ? '*' : '!', m_bConnectResult ? "accepted" : "rejected", m_sConnectResult.c_str());

	net_Syncronised = TRUE;
	if (net_Disconnected)
	{
		OnConnectRejected();
		Disconnect();
		return false;
	}
	//---------------------------------------------------------------------------

	return true;
};

void CLevel::OnConnectResult(NET_Packet*	P)
{
	// multiple results can be sent during connection they should be "AND-ed"
	m_bConnectResultReceived	= true;
	u8	result					= P->r_u8();
	u8  res1					= P->r_u8();
	string512 ResultStr;	
	P->r_stringZ_s				(ResultStr);
	ClientID tmp_client_id;
	P->r_clientID				(tmp_client_id);
	SetClientID					(tmp_client_id);
	
	if (!result)
		m_bConnectResult = false;
	
	m_sConnectResult			= ResultStr;
	if (IsDemoSave() && result)
	{
		P->r_u8(); //server client or not
		shared_str server_options;
		P->r_stringZ(server_options);
		StartSaveDemo(server_options);
	}
};

void CLevel::ClearAllObjects()
{
	u32 CLObjNum = Level().Objects.o_count();

	bool ParentFound = true;
	
	while (ParentFound)
	{	
		ParentFound = false;
		for (u32 i=0; i<CLObjNum; i++)
		{
			CObject* pObj = Level().Objects.o_get_by_iterator(i);
			if (!pObj->H_Parent()) continue;
			//-----------------------------------------------------------
			NET_Packet			GEN;
			GEN.w_begin			(M_EVENT);
			//---------------------------------------------		
			GEN.w_u32			(Level().timeServer());
			GEN.w_u16			(GE_OWNERSHIP_REJECT);
			GEN.w_u16			(pObj->H_Parent()->ID());
			GEN.w_u16			(u16(pObj->ID()));
			game_events->insert	(GEN);
			if (g_bDebugEvents)	ProcessGameEvents();
			//-------------------------------------------------------------
			ParentFound = true;
			//-------------------------------------------------------------
		};
		ProcessGameEvents();
	};

	CLObjNum = Level().Objects.o_count();

	for (u32 i=0; i<CLObjNum; i++)
	{
		CObject* pObj = Level().Objects.o_get_by_iterator(i);
		if (pObj->H_Parent() != nullptr)
			FATAL("pObj->H_Parent()==NULL");

		//-----------------------------------------------------------
		NET_Packet			GEN;
		GEN.w_begin			(M_EVENT);
		//---------------------------------------------		
		GEN.w_u32			(Level().timeServer());
		GEN.w_u16			(GE_DESTROY);
		GEN.w_u16			(u16(pObj->ID()));
		game_events->insert	(GEN);
		if (g_bDebugEvents)	ProcessGameEvents();
		//-------------------------------------------------------------
		ParentFound = true;
		//-------------------------------------------------------------
	};
	ProcessGameEvents();
};

void CLevel::OnInvalidHost()
{
	IPureClient::OnInvalidHost();
	if (MainMenu()->GetErrorDialogType() == CMainMenu::ErrNoError)
		MainMenu()->SetErrorDialog(CMainMenu::ErrInvalidHost);
};

void CLevel::OnSessionFull()
{
	IPureClient::OnSessionFull();
	if (MainMenu()->GetErrorDialogType() == CMainMenu::ErrNoError)
		MainMenu()->SetErrorDialog(CMainMenu::ErrSessionFull);
}

void CLevel::OnConnectRejected()
{
	IPureClient::OnConnectRejected();
};

void CLevel::net_OnChangeSelfName(NET_Packet* P)
{
	if (!P) return;
	string64 NewName			;
	P->r_stringZ(NewName)		;
    shared_str clientOption = GamePersistent().GetClientOption();
	if (!strstr(*clientOption, "/name="))
	{
		string1024 tmpstr;
		xr_strcpy(tmpstr, *clientOption);
		xr_strcat(tmpstr, "/name=");
		xr_strcat(tmpstr, NewName);
        GamePersistent().SetClientOption(tmpstr);
	}
	else
	{
		string1024 tmpstr;
		xr_strcpy(tmpstr, *clientOption);
		*(strstr(tmpstr, "name=")+5) = 0;
		xr_strcat(tmpstr, NewName);
		const char* ptmp = strstr(strstr(*clientOption, "name="), "/");
		if (ptmp)
			xr_strcat(tmpstr, ptmp);
        GamePersistent().SetClientOption(tmpstr);
	}
}
