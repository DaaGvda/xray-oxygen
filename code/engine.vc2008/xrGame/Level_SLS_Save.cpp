#include "stdafx.h"
#include "../xrEngine/xrLevel.h"
#include "Level.h"
#include "game_sv_Single.h"

void CLevel::net_Save(LPCSTR name)		// Game Save
{
	// 1. Create stream
	CMemoryWriter			fs;

	// 2. Description
	fs.open_chunk(fsSLS_Description);
	fs.w_stringZ(net_SessionName());
	fs.close_chunk();

	// 3. Server state
	fs.open_chunk(fsSLS_ServerState);
	Server->SLS_Save(fs);
	fs.close_chunk();

	// Save it to file
	fs.save_to(name);
}