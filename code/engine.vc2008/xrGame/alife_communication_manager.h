////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_communication_manager.h
//	Created 	: 03.09.2003
//  Modified 	: 14.05.2004
//	Author		: Dmitriy Iassenev
//	Description : ALife communication manager
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "alife_simulator_base.h"

class CSE_ALifeSchedulable;
class CSE_ALifeHumanAbstract;
class CSE_ALifeTrader;
class CSE_ALifeTraderAbstract;
#define FAST_OWNERSHIP

class CALifeCommunicationManager : public virtual CALifeSimulatorBase {
public:
						CALifeCommunicationManager		(game_sv_Single *server, LPCSTR section);
};