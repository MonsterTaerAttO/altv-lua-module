#include <Main.h>

void CLuaAltFuncDefs::Init(lua_State* L)
{
	//lua_newtable(L);

	lua_globalfunction(L, "logInfo", CLuaAltFuncDefs::LogInfo);
	lua_globalfunction(L, "logDebug", CLuaAltFuncDefs::LogDebug);
	lua_globalfunction(L, "logWarning", CLuaAltFuncDefs::LogWarning);
	lua_globalfunction(L, "logError", CLuaAltFuncDefs::LogError);
	lua_globalfunction(L, "logColored", CLuaAltFuncDefs::LogColored);

	lua_beginclass(L, "Log");

	lua_classfunction(L, "info", "logInfo");

	/*lua_classfunction(L, "debug", "logDebug");
	lua_classfunction(L, "warning", "logWarning");
	lua_classfunction(L, "error", "logError");
	lua_classfunction(L, "colored", "logColored");*/

	lua_endclass(L);

	/*lua_pushstring(L, "logInfo");
	lua_pushcfunction(L, CLuaAltFuncDefs::LogInfo);
	lua_settable(L, -3);

	lua_pushstring(L, "logDebug");
	lua_pushcfunction(L, &CLuaAltFuncDefs::LogDebug);
	lua_settable(L, -3);

	lua_pushstring(L, "logWarning");
	lua_pushcfunction(L, &CLuaAltFuncDefs::LogWarning);
	lua_settable(L, -3);

	lua_pushstring(L, "logError");
	lua_pushcfunction(L, &CLuaAltFuncDefs::LogError);
	lua_settable(L, -3);

	lua_pushstring(L, "logColored");
	lua_pushcfunction(L, &CLuaAltFuncDefs::LogColored);
	lua_settable(L, -3);

	lua_setglobal(L, "alt");*/

	//Override default print function to behave as LogInfo
	lua_getglobal(L, "_G");
	lua_pushstring(L, "print");
	lua_pushcfunction(L, CLuaAltFuncDefs::LogInfo);
	lua_rawset(L, -3);


	//Hacky way to implement custom __pairs and __ipairs meta events
	//Note: both __pairs and __ipairs metaevents should return only a table
	//For example:
	/*
	int CLuaVehicleDefs::ipairs(lua_State* L)
	{
		lua_newtable(L);
		for (size_t i = 0; i < 3; i++)
		{
			//Keep in mind that ipairs only accept tables with index values
			//and need to keep the sequence i.e: (1, 2, 3, 5) will loop through only the first 3 index, because index 4 is missing

			lua_pushnumber(L, i + 1);
			lua_pushstring(L, "somevalue");
			lua_rawset(L, -3);
		}

		return 1;
	}
	*/

	lua_pushstring(L, "ipairsaux");
	lua_pushcfunction(L, CLuaAltFuncDefs::ipairsaux);
	lua_rawset(L, -3);

	lua_pushstring(L, "inext");
	lua_pushcfunction(L, CLuaAltFuncDefs::inext);
	lua_rawset(L, -3);

	lua_pushstring(L, "_pairs");
	lua_getglobal(L, "pairs");
	lua_rawset(L, -3);

	lua_pushstring(L, "_ipairs");
	lua_getglobal(L, "ipairs");
	lua_rawset(L, -3);

	lua_pushstring(L, "pairs");
	lua_getglobal(L, "inext");
	lua_pushcclosure(L, CLuaAltFuncDefs::pairs, 1);
	lua_rawset(L, -3);

	lua_pushstring(L, "ipairs");
	lua_getglobal(L, "ipairsaux");
	lua_pushcclosure(L, CLuaAltFuncDefs::ipairs, 1);
	lua_rawset(L, -3);

	lua_pop(L, 1);
}

int CLuaAltFuncDefs::inext(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_settop(L, 2);

	if (lua_next(L, 1))
		return 2;
	else
	{
		lua_pushnil(L);
		return 1;
	}
}

int CLuaAltFuncDefs::pairs(lua_State* L)
{
	luaL_getmetafield(L, 1, "__pairs");

	if (lua_isfunction(L, -1))
	{
		lua_call(L, 0, 1);
		lua_replace(L, 1);
	}

	lua_pushvalue(L, lua_upvalueindex(1));
	lua_pushvalue(L, 1);
	lua_pushnil(L);

	return 3;
}

int CLuaAltFuncDefs::ipairsaux(lua_State* L)
{
	Core->LogInfo("CLuaAltFuncDefs::ipairsaux");

	int i = luaL_checkint(L, 2);
	luaL_checktype(L, 1, LUA_TTABLE);
	i++;  /* next value */
	lua_pushinteger(L, i);
	lua_rawgeti(L, 1, i);
	lua_stacktrace(L, "CLuaAltFuncDefs::ipairsaux");

	return (lua_isnil(L, -1)) ? 0 : 2;
}

int CLuaAltFuncDefs::ipairs(lua_State* L)
{
	Core->LogInfo("CLuaAltFuncDefs::ipairs");
	luaL_getmetafield(L, 1, "__ipairs");
	
	if (lua_isfunction(L, -1))
	{
		lua_call(L, 0, 1);
		lua_replace(L, 1);
	}

	lua_pushvalue(L, lua_upvalueindex(1));
	lua_pushvalue(L, 1);
	lua_pushnumber(L, 0);

	lua_stacktrace(L, "CLuaAltFuncDefs::ipairs1");

	return 3;
}

int CLuaAltFuncDefs::tostringtest(lua_State* L)
{
	Core->LogInfo("CLuaAltFuncDefs::tostringtest");

	return 2;
}

int CLuaAltFuncDefs::Log(lua_State* L, LogType logType)
{
	std::string message;

	CArgReader argReader(L);
	argReader.ReadString(message);

	if (argReader.HasAnyError())
	{
		argReader.GetErrorMessages();
		return 0;
	}

	switch (logType)
	{
	case LogType::LOG_COLORED:
		Core->LogColored(message);
		break;

	case LogType::LOG_DEBUG:
		Core->LogDebug(message);
		break;

	case LogType::LOG_ERROR:
		Core->LogError(message);
		break;

	case LogType::LOG_INFO:
		Core->LogInfo(message);
		break;

	case LogType::LOG_WARNING:
		Core->LogWarning(message);
		break;
	}

	return 0;
}

int CLuaAltFuncDefs::LogInfo(lua_State* L)
{
	return CLuaAltFuncDefs::Log(L, LogType::LOG_INFO);
}

int CLuaAltFuncDefs::LogDebug(lua_State* L)
{
	return CLuaAltFuncDefs::Log(L, LogType::LOG_DEBUG);
}

int CLuaAltFuncDefs::LogWarning(lua_State* L)
{
	return CLuaAltFuncDefs::Log(L, LogType::LOG_WARNING);
}

int CLuaAltFuncDefs::LogError(lua_State* L)
{
	return CLuaAltFuncDefs::Log(L, LogType::LOG_ERROR);
}

int CLuaAltFuncDefs::LogColored(lua_State* L)
{
	return CLuaAltFuncDefs::Log(L, LogType::LOG_COLORED);
}