#include "stdafx.h"
#ifndef XRGAME_EXPORTS
#pragma hdrstop

#include "gametype_chooser.h"
#include "xrServer_Objects_Abstract.h"

void  GameTypeChooser::FillProp(LPCSTR pref, PropItemVec& items)
{
	PHelper().CreateGameType		(items, PrepareKey(pref, "Game Type"), this);
 }
#endif // #ifndef XRGAME_EXPORTS