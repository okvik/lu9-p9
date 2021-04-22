#include <u.h>
#include <libc.h>
#include <ctype.h>

#include <lua.h>
#include <lauxlib.h>

#include "../base/common.c"

#include "fs.c"
#include "walk.c"
#include "env.c"
#include "ns.c"
#include "proc.c"
#include "misc.c"

typedef struct Data {
	char *key;
	lua_Integer val;
} Data;

static Data p9data[] = {
	{"OREAD", OREAD},
	{"OWRITE", OWRITE},
	{"ORDWR", ORDWR},
	{"OEXEC", OEXEC},
	{"OTRUNC", OTRUNC},
	{"OCEXEC", OCEXEC},
	{"ORCLOSE", ORCLOSE},
	{"OEXCL", OEXCL},
	
	{"DMDIR", DMDIR},
	{"DMAPPEND", DMAPPEND},
	{"DMEXCL", DMEXCL},
	{"DMMOUNT", DMMOUNT},
	{"DMAUTH", DMAUTH},
	{"DMTMP", DMTMP},
	{"DMREAD", DMREAD},
	{"DMWRITE", DMWRITE},
	{"DMEXEC", DMEXEC},
	{"QTDIR", QTDIR},
	{"QTAPPEND", QTAPPEND},
	{"QTEXCL", QTEXCL},
	{"QTMOUNT", QTMOUNT},
	{"QTAUTH", QTAUTH},
	{"QTTMP", QTTMP},
	{"QTFILE", QTFILE},
	
	{"MREPL", MREPL},
	{"MBEFORE", MBEFORE},
	{"MAFTER", MAFTER},
	{"MCREATE", MCREATE},
	{"MCACHE", MCACHE},

	{"RFPROC", RFPROC},
	{"RFNOWAIT", RFNOWAIT},
	{"RFNAMEG", RFNAMEG},
	{"RFCNAMEG", RFCNAMEG},
	{"RFNOMNT", RFNOMNT},
	{"RFENVG", RFENVG},
	{"RFCENVG", RFCENVG},
	{"RFNOTEG", RFNOTEG},
	{"RFFDG", RFFDG},
	{"RFCFDG", RFCFDG},
	{"RFREND", RFREND},
	{"RFMEM", RFMEM},
	
	{nil, 0}
};

static luaL_Reg p9func[] = {
	{"open", p9_open},
	{"create", p9_create},
	{"file", p9_file},
	{"pipe", p9_pipe},
	{"remove", p9_remove},
	{"access", p9_access},
	
	{"stat", p9_stat},
	{"wstat", p9_wstat},
	{"walk", p9_walk},
	
	{"bind", p9_bind},
	{"mount", p9_mount},
	{"unmount", p9_unmount},
	
	{"getenv", p9_getenv},
	{"setenv", p9_setenv},
	
	{"abort", p9_abort},
	{"exits", p9_exits},
	{"fatal", p9_fatal},
	{"sleep", p9_sleep},
	{"alarm", p9_alarm},
	{"rfork", p9_rfork},
	{"wait", p9_wait},
	{"exec", p9_exec},
	{"wdir", p9_wdir},
	{"pid", p9_pid},
	{"ppid", p9_ppid},
	{"user", p9_user},
	{"sysname", p9_sysname},
	
	{"cleanname", p9_cleanname},
	
	{nil, nil}
};

int
luaopen_p9(lua_State *L)
{
	int lib;
	Buf *buf;
	Data *d;
	
	buf = resizebuffer(L, nil, Iosize);
	lua_pushlightuserdata(L, buf);
	lua_setfield(L, LUA_REGISTRYINDEX, "p9-buffer");
	
	static luaL_Reg filemt[] = {
		{"close", p9_file_close},
		{"read", p9_file_read},
		{"slurp", p9_file_slurp},
		{"write", p9_file_write},
		{"seek", p9_file_seek},
		{"iounit", p9_file_iounit},
		{"path", p9_file_path},
		{"dup", p9_file_dup},
		{nil, nil},
	};
	luaL_newmetatable(L, "p9-File");
	luaL_setfuncs(L, filemt, 0);
	lua_pop(L, 1);
	
	static luaL_Reg walkmt[] = {
		{"__close", p9_walkclose},
		{nil, nil},
	};
	luaL_newmetatable(L, "p9-Walk");
	luaL_setfuncs(L, walkmt, 0);
	lua_pop(L, 1);
	
	luaL_newlib(L, p9func);
	lib = lua_gettop(L);
	for(d = p9data; d->key != nil; d++){
		lua_pushinteger(L, d->val);
		lua_setfield(L, -2, d->key);
	}
	
	static luaL_Reg envmt[] = {
		{"__index", p9_getenv_index},
		{"__newindex", p9_setenv_newindex},
		{nil, nil},
	};
	lua_createtable(L, 0, 2);
	luaL_setfuncs(L, envmt, 0);
	lua_pushvalue(L, -1);
	lua_setmetatable(L, -2);
	lua_setfield(L, lib, "env");
	
	return 1;
}