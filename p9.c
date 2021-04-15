#include <u.h>
#include <libc.h>
#include <ctype.h>

#include <lua.h>
#include <lauxlib.h>

static int
error(lua_State *L, char *fmt, ...)
{
	va_list varg;
	int n;
	char *buf;
	luaL_Buffer b;
	
	lua_pushnil(L);
	buf = luaL_buffinitsize(L, &b, 512);
	va_start(varg, fmt);
	n = vsnprint(buf, 512, fmt, varg);
	va_end(varg);
	luaL_pushresultsize(&b, n);
	return 2;
}

/* Memory allocator associated with Lua state */
static void*
lalloc(lua_State *L, void *ptr, usize sz)
{
	void *ud;
	
	if((ptr = (lua_getallocf(L, &ud))(ud, ptr, LUA_TUSERDATA, sz)) == nil){
		lua_pushliteral(L, "out of memory");
		lua_error(L);
	}
	memset(ptr, 0, sz);
	setmalloctag(ptr, getcallerpc(&L));
	return ptr;
}

/* 
 * Various functions in this library require a
 * variably sized buffer for their operation.
 * Rather than allocating one for each call
 * we preallocate a shared buffer of reasonable
 * size and grow it as needed.
 * The buffer gets associated with a Lua state
 * at library load time.
 * getbuffer(L, sz) returns a pointer to the
 * memory area of at least sz bytes.
 * 
 * To avoid stepping on each other's toes the
 * buffer use must be constrained to a single
 * call.
 */

typedef struct Buf {
	usize sz;
	char *b;
} Buf;

static Buf*
resizebuffer(lua_State *L, Buf *buf, usize sz)
{
	if(buf == nil){
		buf = lalloc(L, nil, sizeof(Buf));
		buf->b = nil;
		buf->sz = 0;
	}
	if(buf->sz < sz){
		buf->b = lalloc(L, buf->b, sz);
		buf->sz = sz;
	}
	return buf;
}

static char*
getbuffer(lua_State *L, usize sz)
{
	Buf *buf;
	
	lua_getfield(L, LUA_REGISTRYINDEX, "p9-buffer");
	buf = lua_touserdata(L, -1);
	return resizebuffer(L, buf, sz)->b;
}

#include "fs.c"
#include "ns.c"
#include "proc.c"

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
	{"close", p9_close},
	{"read", p9_read},
	{"write", p9_write},
	{"seek", p9_seek},
	{"remove", p9_remove},
	{"fd2path", p9_fd2path},
	
	{"stat", p9_stat},
	{"walk", p9_walk},
	
	{"bind", p9_bind},
	{"mount", p9_mount},
	{"unmount", p9_unmount},
	
	{"rfork", p9_rfork},
	
	{nil, nil}
};

int
luaopen_p9(lua_State *L)
{
	Buf *buf;
	Data *d;
	
	buf = resizebuffer(L, nil, 8192);
	lua_pushlightuserdata(L, buf);
	lua_setfield(L, LUA_REGISTRYINDEX, "p9-buffer");
	
	static luaL_Reg walkmt[] = {
		{"__close", p9_walkclose},
		{nil, nil},
	};
	luaL_newmetatable(L, "p9-Walk");
	luaL_setfuncs(L, walkmt, 0);
	
	luaL_newlib(L, p9func);
	for(d = p9data; d->key != nil; d++){
		lua_pushinteger(L, d->val);
		lua_setfield(L, -2, d->key);
	}
	return 1;
}
