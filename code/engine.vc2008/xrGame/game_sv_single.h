/////////////////////////////////////////////////////////////////
//////	Game Server Single Base class
//////	ForserX(nflaming) 13.10.2017
/////////////////////////////////////////////////////////////////

#pragma once
#include "xrServer_Object_Base.h"
class CALifeSimulator;

class game_sv_Single: public CSE_Abstract
{
protected:
	CALifeSimulator					*m_alife_simulator;
	typedef xr_hash_map<u16, CSE_Abstract*>	xrS_entities;
public:
	xrS_entities					entities;
	CSE_Abstract*					ID_to_entity(u16 ID);
public: /* IPure */
	virtual void					STATE_Write(NET_Packet &tNetPacket) {};
	virtual void					STATE_Read(NET_Packet &tNetPacket, u16 size) {};
	virtual void					UPDATE_Write(NET_Packet &tNetPacket) {};
	virtual void					UPDATE_Read(NET_Packet &tNetPacket) {};
public:
									game_sv_Single			();
									game_sv_Single			(const char*):CSE_Abstract("single"){ game_sv_Single();};

	virtual							~game_sv_Single			();

	// Main
	virtual		void				Create					(shared_str& options);
	virtual		void				Update();
	virtual		void				SLS_Save				(IWriter& fs);
	virtual		void				SLS_Default				();
	virtual		void				OnCreate				(u16 id_who);
	virtual		BOOL				OnTouch					(u16 eid_who, u16 eid_what, BOOL bForced = FALSE);
	virtual		void				OnDetach				(u16 eid_who, u16 eid_what);
		CSE_Abstract*				Process_spawn			(NET_Packet& P, ClientID sender, bool bSpawnWithClientsMainEntityAsParent=false, CSE_Abstract* tpExistedEntity=0);
	virtual		u64					GetStartGameTime		();
	virtual		u64					GetGameTime				();
	virtual		float				GetGameTimeFactor		();
	virtual		void				SetGameTimeFactor		(const float fTimeFactor);
	virtual		void				SetGameTimeFactor		(u64 GameTime, const float fTimeFactor);


	virtual		u64					GetEnvironmentGameTime	();
	virtual		float				GetEnvironmentGameTimeFactor		();
	virtual		void				SetEnvironmentGameTimeFactor		(const float fTimeFactor);
	virtual		void				SetEnvironmentGameTimeFactor		(u64 GameTime, const float fTimeFactor);

	virtual		bool				change_level			(NET_Packet &net_packet, ClientID sender);
	virtual		void				save_game				(NET_Packet &net_packet, ClientID sender);
	virtual		bool				load_game				(NET_Packet &net_packet, ClientID sender);
	virtual		void				switch_distance			(NET_Packet &net_packet, ClientID sender);
	virtual		void				teleport_object			(NET_Packet &packet, u16 id);
	virtual		void				add_restriction			(NET_Packet &packet, u16 id);
	virtual		void				remove_restriction		(NET_Packet &packet, u16 id);
	virtual		void				remove_all_restrictions	(NET_Packet &packet, u16 id);
	virtual		bool				custom_sls_default		() {return !!m_alife_simulator;};
	virtual		void				sls_default				();
	virtual		shared_str			level_name				(const shared_str &server_options) const;
	virtual		void				on_death				(CSE_Abstract *e_dest, CSE_Abstract *e_src);
				void				restart_simulator		(LPCSTR saved_game_name);

	IC			CALifeSimulator		&alife					() const
	{
		VERIFY						(m_alife_simulator);
		return						(*m_alife_simulator);
	}

	void							Perform_destroy			(CSE_Abstract* object, u32 mode);
private:
	u64 m_qwStartGameTime;
	u64 m_qwStartProcessorTime;
	u64 m_fTimeFactor;

	u64 m_qwEStartGameTime;
	u64 m_qwEStartProcessorTime;
	u64 m_fETimeFactor;
};
