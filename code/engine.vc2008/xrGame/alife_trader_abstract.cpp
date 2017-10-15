////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_trader_abstract.cpp
//	Created 	: 27.10.2005
//  Modified 	: 27.10.2005
//	Author		: Dmitriy Iassenev
//	Description : ALife trader abstract class
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "alife_simulator.h"
#include "specific_character.h"
#include "ai_space.h"
#include "alife_object_registry.h"
#include "ai_debug.h"
#include "alife_graph_registry.h"
#include "game_sv_Single.h"
#include "alife_schedule_registry.h"

#ifdef DEBUG
	extern Flags32 psAI_Flags;
#endif

void CSE_ALifeTraderAbstract::spawn_supplies	()
{
	CSE_ALifeDynamicObject		*dynamic_object = smart_cast<CSE_ALifeDynamicObject*>(this);
	VERIFY						(dynamic_object);
	CSE_Abstract				*abstract = dynamic_object->alife().spawn_item("device_pda",base()->o_Position,dynamic_object->m_tNodeID,dynamic_object->m_tGraphID,base()->ID);
	CSE_ALifeItemPDA			*pda = smart_cast<CSE_ALifeItemPDA*>(abstract);
	pda->m_original_owner		= base()->ID;

#ifdef XRGAME_EXPORTS
	character_profile			();
	m_SpecificCharacter			= shared_str();
	m_community_index			= NO_COMMUNITY_INDEX;
	pda->m_specific_character	= specific_character();
#endif

	if(m_SpecificCharacter.size())
	{
		//���� � custom data ������� ����
		//������ [dont_spawn_character_supplies]
		//�� �� �������� spawn �� selected_char.SupplySpawn()
		bool specific_character_supply = true;	

		if (xr_strlen(dynamic_object->m_ini_string))
		{
#pragma warning(push)
#pragma warning(disable:4238)
			CInifile ini(&IReader((void*)(*dynamic_object->m_ini_string), xr_strlen(dynamic_object->m_ini_string)), FS.get_path("$game_config$")->m_Path);
#pragma warning(pop)

			if (ini.section_exist("dont_spawn_character_supplies")) 
				specific_character_supply = false;
		}

		if(specific_character_supply)
		{
			CSpecificCharacter selected_char;
			selected_char.Load(m_SpecificCharacter);
			dynamic_object->spawn_supplies(selected_char.SupplySpawn());
		}
	}
}

void CSE_ALifeTraderAbstract::vfInitInventory()
{
}

void CSE_ALifeDynamicObject::attach	(CSE_ALifeInventoryItem *tpALifeInventoryItem, bool bALifeRequest, bool bAddChildren)
{
	if (!bALifeRequest)
		return;

	tpALifeInventoryItem->base()->ID_Parent	= ID;

	if (!bAddChildren)
		return;

	R_ASSERT2			(std::find(children.begin(),children.end(),tpALifeInventoryItem->base()->ID) == children.end(),"Item is already inside the inventory");
	children.push_back	(tpALifeInventoryItem->base()->ID);
}

void CSE_ALifeDynamicObject::detach(CSE_ALifeInventoryItem *tpALifeInventoryItem, ALife::OBJECT_IT *I, bool bALifeRequest, bool bRemoveChildren)
{
	CSE_ALifeDynamicObject					*l_tpALifeDynamicObject1 = smart_cast<CSE_ALifeDynamicObject*>(tpALifeInventoryItem);
	R_ASSERT2								(l_tpALifeDynamicObject1,"Invalid children objects");
	l_tpALifeDynamicObject1->o_Position		= o_Position;
	l_tpALifeDynamicObject1->m_tNodeID		= m_tNodeID;
	l_tpALifeDynamicObject1->m_tGraphID		= m_tGraphID;
	l_tpALifeDynamicObject1->m_fDistance	= m_fDistance;

	if (!bALifeRequest)
		return;

	tpALifeInventoryItem->base()->ID_Parent	= 0xffff;

	if (I) {
		children.erase			(*I);
		return;
	}

	if (!bRemoveChildren)
		return;

	ALife::OBJECT_IT			i = std::find(children.begin(),children.end(),tpALifeInventoryItem->base()->ID);
	R_ASSERT2					(children.end() != i,"Can't detach an item which is not on my own");
	children.erase				(i);
}

void add_online_impl						(CSE_ALifeDynamicObject *object, const bool &update_registries)
{
	NET_Packet					tNetPacket;
	ClientID					clientID;
	clientID.set				(0);

	for (auto it: object->children)
	{
		CSE_ALifeDynamicObject	*l_tpALifeDynamicObject = ai().alife().objects().object(it);
		CSE_ALifeInventoryItem	*l_tpALifeInventoryItem = smart_cast<CSE_ALifeInventoryItem*>(l_tpALifeDynamicObject);
		R_ASSERT2				(l_tpALifeInventoryItem,"Non inventory item object has parent?!");
		l_tpALifeInventoryItem->base()->s_flags.or(M_SPAWN_UPDATE);
		CSE_Abstract			*l_tpAbstract = smart_cast<CSE_Abstract*>(l_tpALifeInventoryItem);
		F_entity_Destroy(l_tpAbstract);
		l_tpALifeDynamicObject->o_Position		= object->o_Position;
		l_tpALifeDynamicObject->m_tNodeID		= object->m_tNodeID;
		object->alife().server().Process_spawn	(tNetPacket,clientID,FALSE,l_tpALifeInventoryItem->base());
		l_tpALifeDynamicObject->s_flags.and		(u16(-1) ^ M_SPAWN_UPDATE);
		l_tpALifeDynamicObject->m_bOnline		= true;
	}

	if (update_registries)
	{
		object->alife().scheduled().remove(object);
		object->alife().graph().remove(object, object->m_tGraphID, false);
	}
}

void CSE_ALifeTraderAbstract::add_online	(const bool &update_registries)
{
	CSE_ALifeDynamicObject		*object = smart_cast<CSE_ALifeDynamicObject*>(this);
	VERIFY						(object);

	add_online_impl				(object,update_registries);
}
#include "sv_idgen.hpp"
void add_offline_impl						(CSE_ALifeDynamicObject *object, const xr_vector<ALife::_OBJECT_ID> &saved_children, const bool &update_registries)
{
	for (u32 i=0, n=saved_children.size(); i<n; ++i) {
		CSE_ALifeDynamicObject	*child = smart_cast<CSE_ALifeDynamicObject*>(ai().alife().objects().object(saved_children[i],true));
		R_ASSERT				(child);
		child->m_bOnline		= false;

		CSE_ALifeInventoryItem	*inventory_item = smart_cast<CSE_ALifeInventoryItem*>(child);
		VERIFY2					(inventory_item,"Non inventory item object has parent?!");
		
		ALife::_OBJECT_ID				item_id = inventory_item->base()->ID;
		inventory_item->base()->ID		= m_tID_Generator.tfGetID(item_id);

		if (!child->can_save()) {
			object->alife().release		(child);
			--i;
			--n;
			continue;
		}

		child->clear_client_data();
		object->alife().graph().add		(child,child->m_tGraphID,false);
		object->alife().graph().attach	(*object,inventory_item,child->m_tGraphID,true);
	}

	if (!update_registries)
		return;

	object->alife().scheduled().add		(object);
	object->alife().graph().add			(object,object->m_tGraphID,false);
}

void CSE_ALifeTraderAbstract::add_offline	(const xr_vector<ALife::_OBJECT_ID> &saved_children, const bool &update_registries)
{
	CSE_ALifeDynamicObject		*object = smart_cast<CSE_ALifeDynamicObject*>(this);
	VERIFY						(object);
	add_offline_impl			(object,saved_children,update_registries);
}
