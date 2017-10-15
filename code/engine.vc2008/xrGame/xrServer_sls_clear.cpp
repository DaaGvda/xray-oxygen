#include "stdafx.h"
#include "game_sv_single.h"
#include "alife_simulator.h"
#include "xrServer_Objects.h"
#include "game_sv_Single.h"
#include "xrmessages.h"
#include "ai_space.h"

void game_sv_Single::Perform_destroy	(CSE_Abstract* object, u32 mode)
{
	R_ASSERT				(object);
	R_ASSERT				(object->ID_Parent == 0xffff);

#if defined(DEBUG) && defined(SLOW_VERIFY_ENTITIES)
	verify_entities			();
#endif

	while (!object->children.empty()) 
	{
		CSE_Abstract		*child = game->get_entity_from_eid(object->children.back());
		R_ASSERT2		(child, make_string("child registered but not found [%d] [%s]", object->children.back(), object->name()));
        
		Perform_reject		(child,object,2*NET_Latency);
#if defined(DEBUG) && defined(SLOW_VERIFY_ENTITIES)
		verify_entities			();
#endif
		Perform_destroy		(child,mode);
	}
	u16						object_id = object->ID;
	entity_Destroy			(object);

#if defined(DEBUG) && defined(SLOW_VERIFY_ENTITIES)
	verify_entities		();
#endif

	NET_Packet				P;
	P.w_begin				(M_EVENT);
	P.w_u32					(Device.dwTimeGlobal - 2*NET_Latency);
	P.w_u16					(GE_DESTROY);
	P.w_u16					(object_id);
	SendBroadcast			(BroadcastCID,P,mode);
}

void game_sv_Single::SLS_Clear		()
{
	u32									mode = net_flags(TRUE,TRUE);
	while (!entities.empty())
	{
		bool							found = false;
		for (auto entities_it = entities.begin(); entities_it != entities.end(); ++entities_it)
		{
			if ((*entities_it).second->ID_Parent != 0xffff)
				continue;
			found						= true;
			Perform_destroy				((*entities_it).second,mode);
			break;
		}
		if (!found)
		{
			for (auto entities_it = entities.begin(); entities_it != entities.end(); ++entities_it)
			{
				if (entities_it->second)
					Msg("! ERROR: can't destroy object [%d][%s] with parent [%d]",
						entities_it->second->ID, entities_it->second->s_name.size() ? entities_it->second->s_name.c_str() : "unknown",
						entities_it->second->ID_Parent);
				else
					Msg("! ERROR: can't destroy entity [%d][?] with parent[?]", entities_it->first);

			}
			Msg("! ERROR: FATAL: can't delete all entities !");
			entities.clear();
		}
		
	}
}
