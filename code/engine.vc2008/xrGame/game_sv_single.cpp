#include "stdafx.h"
#include "game_sv_single.h"
#include "alife_simulator.h"
#include "alife_object_registry.h"
#include "alife_graph_registry.h"
#include "alife_time_manager.h"
#include "object_broker.h"
#include "gamepersistent.h"
#include "../xrEngine/x_ray.h"
#include "../xrEngine/dedicated_server_only.h"
#include "../xrEngine/no_single.h"

game_sv_Single::game_sv_Single():CSE_Abstract("single")
{
	m_qwEStartGameTime = 0;
	m_qwEStartProcessorTime = 0;
	m_fETimeFactor = 0;
	m_alife_simulator = nullptr;
	m_qwStartGameTime = 0;
	m_qwStartProcessorTime = 0;
	m_fTimeFactor = 0;
};

void game_sv_Single::SLS_Save(IWriter& fs)
{
	// Generate spawn+update
	NET_Packet		P;
	u32				position;
	xrS_entities::iterator	I = entities.begin(), E = entities.end();
	for (u32 C = 0; I != E; ++I, ++C)
	{
		CSE_Abstract*	E = I->second;

		fs.open_chunk(C);

		// Spawn
		E->Spawn_Write(P, TRUE);
		fs.w_u16(u16(P.B.count));
		fs.w(P.B.data, P.B.count);

		// Update
		P.w_begin(M_UPDATE);
		P.w_u16(E->ID);
		P.w_chunk_open8(position);
		E->UPDATE_Write(P);
		P.w_chunk_close8(position);

		fs.w_u16(u16(P.B.count));
		fs.w(P.B.data, P.B.count);

		fs.close_chunk();
	}
}


#define USE_DESIGNER_KEY

#ifdef USE_DESIGNER_KEY
#	include "xrServer_Objects_ALife_Monsters.h"
#endif

void game_sv_Single::SLS_Default()
{
#ifdef USE_DESIGNER_KEY
	bool					_designer = !!strstr(Core.Params, "-designer");
	CSE_ALifeCreatureActor	*_actor = 0;
#endif

	string_path				fn_spawn;
	if (FS.exist(fn_spawn, "$level$", "level.spawn")) {
		IReader*			SP = FS.r_open(fn_spawn);
		NET_Packet			P;
		u32					S_id;
		for (IReader *S = SP->open_chunk_iterator(S_id); S; S = SP->open_chunk_iterator(S_id, S)) {
			P.B.count = S->length();
			S->r(P.B.data, P.B.count);

			u16				ID;
			P.r_begin(ID);
			R_ASSERT(M_SPAWN == ID);
			ClientID clientID; clientID.set(0);

#ifdef USE_DESIGNER_KEY
			CSE_Abstract			*entity =
#endif
				Process_spawn(P, clientID);
#ifdef USE_DESIGNER_KEY
			if (_designer) {
				CSE_ALifeCreatureActor	*actor = smart_cast<CSE_ALifeCreatureActor*>(entity);
				if (actor)
					_actor = actor;
			}
#endif
		}
		FS.r_close(SP);
	}

#ifdef USE_DESIGNER_KEY
	if (!_designer)
		return;

	if (_actor)
		return;

	_actor = smart_cast<CSE_ALifeCreatureActor*>(F_entity_Create("actor"));
	_actor->o_Position = Fvector().set(0.f, 0.f, 0.f);
	_actor->set_name_replace("designer");
	_actor->s_flags.flags |= M_SPAWN_OBJECT_ASPLAYER;
	NET_Packet				packet;
	packet.w_begin(M_SPAWN);
	_actor->Spawn_Write(packet, TRUE);

	u16						id;
	packet.r_begin(id);
	R_ASSERT(id == M_SPAWN);
	ClientID				clientID;
	clientID.set(0);
	Process_spawn(packet, clientID);
#endif
}

game_sv_Single::~game_sv_Single()
{
	delete_data(m_alife_simulator);
}
#include "script_process.h"
#include "script_engine_space.h"
#include "script_engine.h"
#include "ai_space.h"
#include "../xrEngine/XR_IOConsole.h"
#include "../xrEngine/xr_ioc_cmd.h"
#include "LevelGameDef.h"

void game_sv_Single::SetEnvironmentGameTimeFactor(u64 GameTime, const float fTimeFactor)
{
	m_qwEStartGameTime = GameTime;
	m_qwEStartProcessorTime = Level().timeServer_Async();
	m_fETimeFactor = fTimeFactor;
}

void game_sv_Single::SetGameTimeFactor(u64 GameTime, const float fTimeFactor)
{
	m_qwStartGameTime = GameTime;
	m_qwStartProcessorTime = Level().timeServer_Async();
	m_fTimeFactor = fTimeFactor;
}

void game_sv_Single::Create(shared_str& options)
{
	string_path	fn_game;
	if (FS.exist(fn_game, "$level$", "level.game"))
	{
		IReader *F = FS.r_open(fn_game);
		IReader *O = 0;

		if (O = F->open_chunk(RPOINT_CHUNK))
		{
			O->close();
		}

		FS.r_close(F);
	}

	// loading scripts
	ai().script_engine().remove_script_process(ScriptEngine::eScriptProcessorGame);
	string_path					S;
	FS.update_path(S, "$game_config$", "script.ltx");
	CInifile					*l_tpIniFile = xr_new<CInifile>(S);
	R_ASSERT(l_tpIniFile);

	const shared_str type_name = "single";
	if (l_tpIniFile->section_exist(type_name))
	{
		if (l_tpIniFile->r_string(type_name, "script"))
			ai().script_engine().add_script_process(ScriptEngine::eScriptProcessorGame, xr_new<CScriptProcess>("game", l_tpIniFile->r_string(type_name, "script")));
		else
			ai().script_engine().add_script_process(ScriptEngine::eScriptProcessorGame, xr_new<CScriptProcess>("game", ""));
	}
	xr_delete(l_tpIniFile);
	//---------------------------------------------------------------------
	if (strstr(*options, "/alife"))
		m_alife_simulator = xr_new<CALifeSimulator>(this, &options);
}

CSE_Abstract* game_sv_Single::ID_to_entity(u16 ID)
{
	if (0xffff == ID)				return 0;
	xrS_entities::iterator	I = entities.find(ID);
	return (entities.end() != I) ? I->second : 0;
}

void	game_sv_Single::OnCreate(u16 id_who)
{
	if (!ai().get_alife())
		return;

	CSE_Abstract			*e_who = ID_to_entity(id_who);
	VERIFY(e_who);
	if (!e_who->m_bALifeControl)
		return;

	CSE_ALifeObject			*alife_object = smart_cast<CSE_ALifeObject*>(e_who);
	if (!alife_object)
		return;

	alife_object->m_bOnline = true;

	if (alife_object->ID_Parent != 0xffff)
	{
		CSE_ALifeDynamicObject			*parent = ai().alife().objects().object(alife_object->ID_Parent, true);
		if (parent) {
			CSE_ALifeTraderAbstract		*trader = smart_cast<CSE_ALifeTraderAbstract*>(parent);
			if (trader)
				alife().create(alife_object);
			else {
				CSE_ALifeInventoryBox* const	box = smart_cast<CSE_ALifeInventoryBox*>(parent);
				if (box)
					alife().create(alife_object);
				else
					alife_object->m_bALifeControl = false;
			}
		}
		else
			alife_object->m_bALifeControl = false;
	}
	else
		alife().create(alife_object);
}

BOOL game_sv_Single::OnTouch(u16 eid_who, u16 eid_what, BOOL bForced)
{
	CSE_Abstract*		e_who = ID_to_entity(eid_who);		VERIFY(e_who);
	CSE_Abstract*		e_what = ID_to_entity(eid_what);	VERIFY(e_what);

	if (ai().get_alife()) {
		CSE_ALifeInventoryItem	*l_tpALifeInventoryItem = smart_cast<CSE_ALifeInventoryItem*>	(e_what);
		CSE_ALifeDynamicObject	*l_tpDynamicObject = smart_cast<CSE_ALifeDynamicObject*>	(e_who);

		if (
			l_tpALifeInventoryItem &&
			l_tpDynamicObject &&
			ai().alife().graph().level().object(l_tpALifeInventoryItem->base()->ID, true) &&
			ai().alife().objects().object(e_who->ID, true) &&
			ai().alife().objects().object(e_what->ID, true)
			)
			alife().graph().attach(*e_who, l_tpALifeInventoryItem, l_tpDynamicObject->m_tGraphID, false, false);
	}
	return TRUE;
}

void game_sv_Single::OnDetach(u16 eid_who, u16 eid_what)
{
	if (ai().get_alife())
	{
		CSE_Abstract*		e_who = ID_to_entity(eid_who);		VERIFY(e_who);
		CSE_Abstract*		e_what = ID_to_entity(eid_what);	VERIFY(e_what);

		CSE_ALifeInventoryItem *l_tpALifeInventoryItem = smart_cast<CSE_ALifeInventoryItem*>(e_what);
		if (!l_tpALifeInventoryItem)
			return;

		CSE_ALifeDynamicObject *l_tpDynamicObject = smart_cast<CSE_ALifeDynamicObject*>(e_who);
		if (!l_tpDynamicObject)
			return;

		if (
			ai().alife().objects().object(e_who->ID, true) &&
			!ai().alife().graph().level().object(l_tpALifeInventoryItem->base()->ID, true) &&
			ai().alife().objects().object(e_what->ID, true)
			)
			alife().graph().detach(*e_who, l_tpALifeInventoryItem, l_tpDynamicObject->m_tGraphID, false, false);
		else {
			if (!ai().alife().objects().object(e_what->ID, true)) {
				u16				id = l_tpALifeInventoryItem->base()->ID_Parent;
				l_tpALifeInventoryItem->base()->ID_Parent = 0xffff;

				CSE_ALifeDynamicObject *dynamic_object = smart_cast<CSE_ALifeDynamicObject*>(e_what);
				VERIFY(dynamic_object);
				dynamic_object->m_tNodeID = l_tpDynamicObject->m_tNodeID;
				dynamic_object->m_tGraphID = l_tpDynamicObject->m_tGraphID;
				dynamic_object->m_bALifeControl = true;
				dynamic_object->m_bOnline = true;
				alife().create(dynamic_object);
				l_tpALifeInventoryItem->base()->ID_Parent = id;
			}
		}
	}
}

#include "level.h"
void game_sv_Single::Update()
{
		if (Level().game)
		{
			auto script_process = ai().script_engine().script_process(ScriptEngine::eScriptProcessorGame);
			if (script_process)
				script_process->update();
		}
}

u64 game_sv_Single::GetStartGameTime	()
{
	return(ai().alife().time_manager().start_game_time());
}

u64 game_sv_Single::GetGameTime		()
{
	return(ai().alife().time_manager().game_time());
}

float game_sv_Single::GetGameTimeFactor		()
{
	return(ai().alife().time_manager().time_factor());
}

void game_sv_Single::SetGameTimeFactor		(const float fTimeFactor)
{
	return(alife().time_manager().set_time_factor(fTimeFactor));
}

ALife::_TIME_ID game_sv_Single::GetEnvironmentGameTime		()
{
	return(alife().time_manager().game_time());
}

float game_sv_Single::GetEnvironmentGameTimeFactor()
{
	return (m_fTimeFactor);
}

void game_sv_Single::SetEnvironmentGameTimeFactor		(const float fTimeFactor)
{
	m_qwStartGameTime = GetGameTime();
	m_qwStartProcessorTime = Level().timeServer_Async();
	m_fTimeFactor = fTimeFactor;
}

bool game_sv_Single::change_level					(NET_Packet &net_packet, ClientID sender)
{
	if (ai().get_alife())
		return					(alife().change_level(net_packet));
	else
		return					(true);
}

void game_sv_Single::save_game						(NET_Packet &net_packet, ClientID sender)
{
	if (!ai().get_alife())
		return;
	
	alife().save				(net_packet);
}

bool game_sv_Single::load_game					(NET_Packet &net_packet, ClientID sender)
{
	if (!ai().get_alife())
		return true;
	shared_str						game_name;
	net_packet.r_stringZ		(game_name);
	return						(alife().load_game(*game_name,true));
}

void game_sv_Single::switch_distance			(NET_Packet &net_packet, ClientID sender)
{
	if (!ai().get_alife())
		return;

	alife().set_switch_distance	(net_packet.r_float());
}

void game_sv_Single::teleport_object			(NET_Packet &net_packet, u16 id)
{
	if (!ai().get_alife())
		return;
	
	GameGraph::_GRAPH_ID		game_vertex_id;
	u32						level_vertex_id;
	Fvector					position;

	net_packet.r			(&game_vertex_id,sizeof(game_vertex_id));
	net_packet.r			(&level_vertex_id,sizeof(level_vertex_id));
	net_packet.r_vec3		(position);

	alife().teleport_object (id,game_vertex_id,level_vertex_id,position);
}

void game_sv_Single::add_restriction			(NET_Packet &packet, u16 id)
{
	if (!ai().get_alife())
		return;

	ALife::_OBJECT_ID		restriction_id;
	packet.r				(&restriction_id,sizeof(restriction_id));

	RestrictionSpace::ERestrictorTypes	restriction_type;
	packet.r				(&restriction_type,sizeof(restriction_type));
	
	alife().add_restriction (id,restriction_id,restriction_type);
}

void game_sv_Single::remove_restriction			(NET_Packet &packet, u16 id)
{
	if (!ai().get_alife())
		return;

	ALife::_OBJECT_ID		restriction_id;
	packet.r				(&restriction_id,sizeof(restriction_id));

	RestrictionSpace::ERestrictorTypes	restriction_type;
	packet.r				(&restriction_type,sizeof(restriction_type));
	
	alife().remove_restriction (id,restriction_id,restriction_type);
}

void game_sv_Single::remove_all_restrictions	(NET_Packet &packet, u16 id)
{
	if (!ai().get_alife())
		return;

	RestrictionSpace::ERestrictorTypes	restriction_type;
	packet.r				(&restriction_type,sizeof(restriction_type));

	alife().remove_all_restrictions (id,restriction_type);
}

void game_sv_Single::sls_default				()
{
	alife().update_switch	();
}

shared_str game_sv_Single::level_name			(const shared_str &server_options) const
{
	if (!ai().get_alife())
		return "";
	return					(alife().level_name());
}

void game_sv_Single::on_death					(CSE_Abstract *e_dest, CSE_Abstract *e_src)
{
	//inherited::on_death/////////////////////////////////////////////////////////////////
	CSE_ALifeCreatureAbstract	*creature = smart_cast<CSE_ALifeCreatureAbstract*>(e_dest);
	if (creature)
	{
		VERIFY(creature->get_killer_id() == ALife::_OBJECT_ID(-1));
		creature->set_killer_id(e_src->ID);
	}
	delete creature;
	//////////////////////////////////////////////////////////////////////////////////////
	if (ai().get_alife())
	{
		alife().on_death(e_dest, e_src);
	}
}

void game_sv_Single::restart_simulator			(LPCSTR saved_game_name)
{
	shared_str				options = GamePersistent().GetServerOption();
	delete_data				(m_alife_simulator);

	xr_strcpy					(g_pGamePersistent->m_game_params.m_game_or_spawn,saved_game_name);
	xr_strcpy					(g_pGamePersistent->m_game_params.m_new_or_load,"load");

	pApp->ls_header[0] = '\0';
	pApp->ls_tip_number[0] = '\0';
	pApp->ls_tip[0] = '\0';
	pApp->LoadBegin			();
	m_alife_simulator		= xr_new<CALifeSimulator>(this,&options);
	g_pGamePersistent->LoadTitle		();
	Device.PreCache			(60, true, true);
	pApp->LoadEnd			();
}

#include "sv_idgen.hpp"
CSE_Abstract* game_sv_Single::Process_spawn(NET_Packet& P, ClientID sender, bool bSpawnWithClientsMainEntityAsParent, CSE_Abstract* tpExistedEntity)
{
	// create server entity
	CSE_Abstract*	E = tpExistedEntity;
	if (!E) 
	{
		// read spawn information
		string64 s_name;
		P.r_stringZ(s_name);
		// create entity
		E = F_entity_Create(s_name); R_ASSERT3(E, "Can't create entity.", s_name);
		E->Spawn_Read(P);
		F_entity_Destroy(E);
		return 0;
	}
	else {
		VERIFY(E->m_bALifeControl);
	}

	CSE_Abstract			*e_parent = 0;
	if (E->ID_Parent != 0xffff) {
		e_parent = ID_to_entity(E->ID_Parent);
		if (!e_parent) {
			R_ASSERT(!tpExistedEntity);
			F_entity_Destroy(E);
			return			NULL;
		}
	}


	// check for respawn-capability and create phantom as needed
	if (E->RespawnTime && (0xffff == E->ID_Phantom))
	{
		// Create phantom
		CSE_Abstract* Phantom = F_entity_Create(*E->s_name); R_ASSERT(Phantom);
		Phantom->Spawn_Read(P);
		Phantom->ID = m_tID_Generator.tfGetID(0xffff);
		Phantom->ID_Phantom = Phantom->ID;	// Self-linked to avoid phantom-breeding
		entities.insert(std::make_pair(Phantom->ID, Phantom));

		Phantom->s_flags.set(M_SPAWN_OBJECT_PHANTOM, TRUE);

		// Spawn entity
		E->ID = m_tID_Generator.tfGetID(E->ID);
		E->ID_Phantom = Phantom->ID;
		entities.insert(std::make_pair(E->ID, E));
	}
	else {
		if (E->s_flags.is(M_SPAWN_OBJECT_PHANTOM))
		{
			// Clone from Phantom
			E->ID = m_tID_Generator.tfGetID(0xffff);
			E->s_flags.set(M_SPAWN_OBJECT_PHANTOM, FALSE);
			entities.insert(std::make_pair(E->ID, E));
		}
		else {
			E->ID = m_tID_Generator.tfGetID(E->ID);
			entities.insert(std::make_pair(E->ID, E));
		}
	}
	E->s_RP = 0xFE;	// Use supplied

					// Parent-Connect
	if (!tpExistedEntity) {
		OnCreate(E->ID);

		if (0xffff != E->ID_Parent) {
			R_ASSERT(e_parent);

			OnTouch(E->ID_Parent, E->ID);

			e_parent->children.push_back(E->ID);
		}
	}

	// create packet and broadcast packet to everybody
	NET_Packet Packet;

	E->Spawn_Write(Packet, FALSE);
	if (E->s_flags.is(M_SPAWN_UPDATE))
		E->UPDATE_Write(Packet);
	ClientID clientID; clientID.set(0);
	return E;
}


void game_sv_Single::Perform_destroy(CSE_Abstract* object, u32 mode)
{
	R_ASSERT(object);
	R_ASSERT(object->ID_Parent == 0xffff);

	while (!object->children.empty())
	{
		CSE_Abstract *child = ID_to_entity(object->children.back());
		R_ASSERT2(child, make_string("child registered but not found [%d]", object->children.back()));

		/////////////////////// Perform_reject //////////////
		//Perform_reject(child, object, 2 * NET_Latency);
		R_ASSERT(child->ID_Parent == object->ID);

		NET_Packet P;
		u32 time = Device.dwTimeGlobal - NET_Latency;

		P.w_begin(M_EVENT);
		P.w_u32(time);
		P.w_u16(GE_OWNERSHIP_REJECT);
		P.w_u16(object->ID);
		P.w_u16(child->ID);
		P.w_u8(1);
		////Process_event_reject/////////////////////////////////////////
		CSE_Abstract* e_entity = ID_to_entity(child->ID);
		e_entity->ID_Parent = 0xffff;
		OnDetach(object->ID, child->ID);
		////////////////////////////////////////////////////////////////
		Perform_destroy(child, mode);
	}

	u16						object_id = object->ID;
	F_entity_Destroy(object);

	NET_Packet				P;
	P.w_begin(M_EVENT);
	P.w_u32(Device.dwTimeGlobal - 2 * NET_Latency);
	P.w_u16(GE_DESTROY);
	P.w_u16(object_id);
	//	SendBroadcast			(BroadcastCID,P,mode);
}