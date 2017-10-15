#pragma once
#include "../xrCore/client_id.h"

class	NET_Packet;
class	CGameObject;
class	CUIGameCustom;

struct SZoneMapEntityData{
	Fvector	pos;
	u32		color;
	SZoneMapEntityData(){pos.set(.0f,.0f,.0f);color = 0xff00ff00;}
};


class game_cl_GameState: public ISheduled
{
protected:
	CUIGameCustom*						m_game_ui_custom;
	bool								m_bServerControlHits;

protected:
	virtual		shared_str			shedule_Name			() const		{ return shared_str("game_cl_GameState"); };
	virtual		float				shedule_Scale			()				{ return 1.0f;};
	virtual		bool				shedule_Needed			()				{ return true;};

				void				sv_GameEventGen			(NET_Packet& P);
				void				sv_EventSend			(NET_Packet& P);
public:
									game_cl_GameState		();
	virtual							~game_cl_GameState		();
	virtual		CUIGameCustom*		createGameUI			() {return nullptr;};
	virtual		void				SetGameUI				(CUIGameCustom*){};

	virtual		void				shedule_Update			(u32 dt);

	void							u_EventGen				(NET_Packet& P, u16 type, u16 dest);
	void							u_EventSend				(NET_Packet& P);

	virtual		void				OnRender				()	{}; // ќх уж этот новый Mixed, где-то Debug, где Any
	virtual		bool				IsServerControlHits		()	{return m_bServerControlHits;};

	virtual		void				OnSpawn					(CObject* pObj)	{};
	virtual		void				OnDestroy				(CObject* pObj)	{};
	virtual		void				OnConnected				();
};
