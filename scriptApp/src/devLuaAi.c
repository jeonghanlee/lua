#include "devUtil.h"

#include "stdio.h"

#include "lua.h"

#include <aiRecord.h>
#include <epicsExport.h>
#include <dbCommon.h>
#include <devSup.h>

static void pushRecord(struct aiRecord* record)
{
	Protocol* proto = (Protocol*) record->dpvt;
	lua_State* state = proto->state;
	
	lua_createtable(state, 0, 7);
	
	lua_pushstring(state, "name");
	lua_pushstring(state, record->name);
	lua_settable(state, -3);
	
	lua_pushstring(state, "desc");
	lua_pushstring(state, record->desc);
	lua_settable(state, -3);
	
	lua_pushstring(state, "scan");
	lua_pushnumber(state, record->scan);
	lua_settable(state, -3);
	
	lua_pushstring(state, "val");
	lua_pushnumber(state, record->val);
	lua_settable(state, -3);
	
	lua_pushstring(state, "egu");
	lua_pushstring(state, record->egu);
	lua_settable(state, -3);
	
	lua_pushstring(state, "aoff");
	lua_pushnumber(state, record->aoff);
	lua_settable(state, -3);
	
	lua_pushstring(state, "aslo");
	lua_pushnumber(state, record->aslo);
	lua_settable(state, -3);
}

static long readData(struct aiRecord* record)
{
	Protocol* proto = (Protocol*) record->dpvt;
	
	lua_getglobal(proto->state, proto->function_name);
	pushRecord(record);
	runFunction(proto);
	
	int type = lua_type(proto->state, -1);
	
	switch (type)
	{		
		case LUA_TNUMBER:
		{
			double val = lua_tonumber(proto->state, -1);
			
			if (record->aslo != 0.0)    { val *= record->aslo; }
			
			val += record->aoff;
			
			record->val = val;
			lua_pop(proto->state, 1);
			return 2;
		}
		
		case LUA_TNIL:
			lua_pop(proto->state, 1);
			return 0;
		
		default:
			lua_pop(proto->state, 1);
			return -1;
	}
	
	return 0;
}


static long initRecord (dbCommon* record)
{
	aiRecord* ai = (aiRecord*) record;
	
	ai->dpvt = parseINPOUT(&ai->inp);
	
	return 0;
}

struct {
    long number;
    DEVSUPFUN report;
    DEVSUPFUN init;
    DEVSUPFUN init_record;
    DEVSUPFUN get_ioint_info;
    DEVSUPFUN read;
    DEVSUPFUN special_linconv;
} devLuaAi = {
    6,
    NULL,
    NULL,
    initRecord,
    NULL,
    readData,
    NULL
};

epicsExportAddress(dset, devLuaAi);
