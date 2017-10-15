#include "stdafx.h"
#include "level.h"
#include "game_sv_Single.h"

void	CLevel::SLS_Default				()					// Default/Editor Load
{
	// Signal main actor spawn
	Server->SLS_Default();
}
