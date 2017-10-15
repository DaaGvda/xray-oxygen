////////////////////////////////////////////////////////////////////////////
//	Module 		: lua_tools.h
//	Created 	: 29.07.2014
//	Author		: Alexander Petrov
////////////////////////////////////////////////////////////////////////////
//	Module 		: lua_traceback.h
//  Created 	: 12.07.2017
//	Author		: ForserX
//	Description : Lua functionality extension
////////////////////////////////////////////////////////////////////////////
extern "C"
{
#include <lua.h>
#include <luajit.h>
};
#include "lua_traceback.hpp"


SCRIPT_API const char* get_traceback(lua_State *L, int depth)
{
	return lua_tostring(L, -1);
}