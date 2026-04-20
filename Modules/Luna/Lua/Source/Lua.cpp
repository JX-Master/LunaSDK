/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Lua.hpp
* @author JXMaster
* @date 2026/2/22
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_LUA_API LUNA_EXPORT
#include "../Lua.hpp"
#include <Luna/Runtime/Module.hpp>
#include <lua.hpp>

namespace Luna
{
    namespace Lua
    {
        LUNA_LUA_API LuaStatePtr newstate(Alloc* f, void* ud)
        {
            return (LuaStatePtr)lua_newstate(f, ud, luaL_makeseed(NULL));
        }
        LUNA_LUA_API void close(LuaStatePtr state)
        {
            lua_close((lua_State*)state);
        }
        LUNA_LUA_API LuaStatePtr newthread(LuaStatePtr L)
        {
            return (LuaStatePtr)lua_newthread((lua_State*)L);
        }
        LUNA_LUA_API int closethread(LuaStatePtr L, LuaStatePtr from)
        {
            return lua_closethread((lua_State*)L, (lua_State*)from);
        }
        LUNA_LUA_API CFunction* atpanic(LuaStatePtr L, CFunction* panicf)
        {
            return (CFunction*)lua_atpanic((lua_State*)L, (lua_CFunction)panicf);
        }
        LUNA_LUA_API Version version(LuaStatePtr L)
        {
            int version = lua_version((lua_State*)L);
            Version ret;
            ret.major = (u32)(version / 100);
            ret.minor = (u32)(version % 100);
            ret.patch = 0;
            return ret;
        }

        LUNA_LUA_API int absindex(LuaStatePtr L, int idx)
        {
            return lua_absindex((lua_State*)L, idx);
        }
        LUNA_LUA_API int gettop(LuaStatePtr L)
        {
            return lua_gettop((lua_State*)L);
        }
        LUNA_LUA_API void settop(LuaStatePtr L, int idx)
        {
            lua_settop((lua_State*)L, idx);
        }
        LUNA_LUA_API void pushvalue(LuaStatePtr L, int idx)
        {
            lua_pushvalue((lua_State*)L, idx);
        }
        LUNA_LUA_API void rotate(LuaStatePtr L, int idx, int n)
        {
            lua_rotate((lua_State*)L, idx, n);
        }
        LUNA_LUA_API void copy(LuaStatePtr L, int fromidx, int toidx)
        {
            lua_copy((lua_State*)L, fromidx, toidx);
        }
        LUNA_LUA_API int checkstack(LuaStatePtr L, int n)
        {
            return lua_checkstack((lua_State*)L, n);
        }
        LUNA_LUA_API void xmove(LuaStatePtr from, LuaStatePtr to, int n)
        {
            lua_xmove((lua_State*)from, (lua_State*)to, n);
        }

        LUNA_LUA_API bool isnumber(LuaStatePtr L, int idx)
        {
            return lua_isnumber((lua_State*)L, idx) != 0;
        }
        LUNA_LUA_API bool isstring(LuaStatePtr L, int idx)
        {
            return lua_isstring((lua_State*)L, idx) != 0;
        }
        LUNA_LUA_API bool iscfunction(LuaStatePtr L, int idx)
        {
            return lua_iscfunction((lua_State*)L, idx) != 0;
        }
        LUNA_LUA_API bool isinteger(LuaStatePtr L, int idx)
        {
            return lua_isinteger((lua_State*)L, idx) != 0;
        }
        LUNA_LUA_API bool isuserdata(LuaStatePtr L, int idx)
        {
            return lua_isuserdata((lua_State*)L, idx) != 0;
        }
        LUNA_LUA_API Type type(LuaStatePtr L, int idx)
        {
            return (Type)lua_type((lua_State*)L, idx);
        }
        LUNA_LUA_API const char* gettypename(LuaStatePtr L, Type tp)
        {
            return lua_typename((lua_State*)L, (int)tp);
        }

        LUNA_LUA_API number tonumberx(LuaStatePtr L, int idx, int* isnum)
        {
            return lua_tonumberx((lua_State*)L, idx, isnum);
        }
        LUNA_LUA_API integer tointegerx(LuaStatePtr L, int idx, int* isnum)
        {
            return lua_tointegerx((lua_State*)L, idx, isnum);
        }
        LUNA_LUA_API int toboolean(LuaStatePtr L, int idx)
        {
            return lua_toboolean((lua_State*)L, idx);
        }
        LUNA_LUA_API const char* tolstring(LuaStatePtr L, int idx, usize* len)
        {
            return lua_tolstring((lua_State*)L, idx, (size_t*)len);
        }
        LUNA_LUA_API unsigned_integer rawlen(LuaStatePtr L, int idx)
        {
            return lua_rawlen((lua_State*)L, idx);
        }
        LUNA_LUA_API CFunction* tocfunction(LuaStatePtr L, int idx)
        {
            return (CFunction*)lua_tocfunction((lua_State*)L, idx);
        }
        LUNA_LUA_API void* touserdata(LuaStatePtr L, int idx)
        {
            return lua_touserdata((lua_State*)L, idx);
        }
        LUNA_LUA_API LuaStatePtr tothread(LuaStatePtr L, int idx)
        {
            return (LuaStatePtr)lua_tothread((lua_State*)L, idx);
        }
        LUNA_LUA_API const void* topointer(LuaStatePtr L, int idx)
        {
            return lua_topointer((lua_State*)L, idx);
        }

        LUNA_LUA_API void arith(LuaStatePtr L, ArithOp op)
        {
            lua_arith((lua_State*)L, (int)op);
        }
        LUNA_LUA_API int rawequal(LuaStatePtr L, int idx1, int idx2)
        {
            return lua_rawequal((lua_State*)L, idx1, idx2);
        }
        LUNA_LUA_API int compare(LuaStatePtr L, int idx1, int idx2, CompareOp op)
        {
            return lua_compare((lua_State*)L, idx1, idx2, (int)op);
        }

        LUNA_LUA_API void pushnil(LuaStatePtr L)
        {
            lua_pushnil((lua_State*)L);
        }
        LUNA_LUA_API void pushnumber(LuaStatePtr L, number n)
        {
            lua_pushnumber((lua_State*)L, n);
        }
        LUNA_LUA_API void pushinteger(LuaStatePtr L, integer n)
        {
            lua_pushinteger((lua_State*)L, n);
        }
        LUNA_LUA_API const char* pushlstring(LuaStatePtr L, const char* s, usize len)
        {
            return lua_pushlstring((lua_State*)L, s, len);
        }
        LUNA_LUA_API const char* pushexternalstring(LuaStatePtr L, const char* s, usize len, Alloc* falloc, void* ud)
        {
            return lua_pushexternalstring((lua_State*)L, s, len, (lua_Alloc)falloc, ud);
        }
        LUNA_LUA_API const char* pushstring(LuaStatePtr L, const char* s)
        {
            return lua_pushstring((lua_State*)L, s);
        }
        LUNA_LUA_API const char* pushvfstring(LuaStatePtr L, const char* fmt,
            va_list argp)
        {
            return lua_pushvfstring((lua_State*)L, fmt, argp);
        }
        LUNA_LUA_API const char* pushfstring(LuaStatePtr L, const char* fmt, ...)
        {
            va_list argp;
            va_start(argp, fmt);
            const char* ret = lua_pushvfstring((lua_State*)L, fmt, argp);
            va_end(argp);
            return ret;
        }
        LUNA_LUA_API void pushcclosure(LuaStatePtr L, CFunction* fn, int n)
        {
            lua_pushcclosure((lua_State*)L, (lua_CFunction)fn, n);
        }
        LUNA_LUA_API void pushboolean(LuaStatePtr L, int b)
        {
            lua_pushboolean((lua_State*)L, b);
        }
        LUNA_LUA_API void pushlightuserdata(LuaStatePtr L, void* p)
        {
            lua_pushlightuserdata((lua_State*)L, p);
        }
        LUNA_LUA_API int pushthread(LuaStatePtr L)
        {
            return lua_pushthread((lua_State*)L);
        }

        LUNA_LUA_API int getglobal(LuaStatePtr L, const char* name)
        {
            return lua_getglobal((lua_State*)L, name);
        }
        LUNA_LUA_API int gettable(LuaStatePtr L, int idx)
        {
            return lua_gettable((lua_State*)L, idx);
        }
        LUNA_LUA_API int getfield(LuaStatePtr L, int idx, const char* k)
        {
            return lua_getfield((lua_State*)L, idx, k);
        }
        LUNA_LUA_API int geti(LuaStatePtr L, int idx, integer n)
        {
            return lua_geti((lua_State*)L, idx, n);
        }
        LUNA_LUA_API int rawget(LuaStatePtr L, int idx)
        {
            return lua_rawget((lua_State*)L, idx);
        }
        LUNA_LUA_API int rawgeti(LuaStatePtr L, int idx, integer n)
        {
            return lua_rawgeti((lua_State*)L, idx, n);
        }
        LUNA_LUA_API int rawgetp(LuaStatePtr L, int idx, const void* p)
        {
            return lua_rawgetp((lua_State*)L, idx, p);
        }
        LUNA_LUA_API void createtable(LuaStatePtr L, int narr, int nrec)
        {
            lua_createtable((lua_State*)L, narr, nrec);
        }
        LUNA_LUA_API void* newuserdatauv(LuaStatePtr L, usize sz, int nuvalue)
        {
            return lua_newuserdatauv((lua_State*)L, sz, nuvalue);
        }
        LUNA_LUA_API int getmetatable(LuaStatePtr L, int objindex)
        {
            return lua_getmetatable((lua_State*)L, objindex);
        }
        LUNA_LUA_API int getiuservalue(LuaStatePtr L, int idx, int n)
        {
            return lua_getiuservalue((lua_State*)L, idx, n);
        }

        LUNA_LUA_API void setglobal(LuaStatePtr L, const char* name)
        {
            lua_setglobal((lua_State*)L, name);
        }
        LUNA_LUA_API void settable(LuaStatePtr L, int idx)
        {
            lua_settable((lua_State*)L, idx);
        }
        LUNA_LUA_API void setfield(LuaStatePtr L, int idx, const char* k)
        {
            lua_setfield((lua_State*)L, idx, k);
        }
        LUNA_LUA_API void seti(LuaStatePtr L, int idx, integer n)
        {
            lua_seti((lua_State*)L, idx, n);
        }
        LUNA_LUA_API void rawset(LuaStatePtr L, int idx)
        {
            lua_rawset((lua_State*)L, idx);
        }
        LUNA_LUA_API void rawseti(LuaStatePtr L, int idx, integer n)
        {
            lua_rawseti((lua_State*)L, idx, n);
        }
        LUNA_LUA_API void rawsetp(LuaStatePtr L, int idx, const void* p)
        {
            lua_rawsetp((lua_State*)L, idx, p);
        }
        LUNA_LUA_API int setmetatable(LuaStatePtr L, int objindex)
        {
            return lua_setmetatable((lua_State*)L, objindex);
        }
        LUNA_LUA_API int setiuservalue(LuaStatePtr L, int idx, int n)
        {
            return lua_setiuservalue((lua_State*)L, idx, n);
        }

        LUNA_LUA_API void callk(LuaStatePtr L, int nargs, int nresults,
            KContext ctx, KFunction* k)
        {
            lua_callk((lua_State*)L, nargs, nresults, ctx, (lua_KFunction)k);
        }
        LUNA_LUA_API int pcallk(LuaStatePtr L, int nargs, int nresults, int errfunc,
            KContext ctx, KFunction* k)
        {
            return lua_pcallk((lua_State*)L, nargs, nresults, errfunc, ctx, (lua_KFunction)k);
        }
        LUNA_LUA_API int load(LuaStatePtr L, Reader* reader, void* dt,
            const char* chunkname, const char* mode)
        {
            return lua_load((lua_State*)L, (lua_Reader)reader, dt, chunkname, mode);
        }
        LUNA_LUA_API int dump(LuaStatePtr L, Writer* writer, void* data, int strip)
        {
            return lua_dump((lua_State*)L, (lua_Writer)writer, data, strip);
        }

        LUNA_LUA_API int yieldk(LuaStatePtr L, int nresults, KContext ctx,
            KFunction* k)
        {
            return lua_yieldk((lua_State*)L, nresults, ctx, (lua_KFunction)k);
        }
        LUNA_LUA_API int resume(LuaStatePtr L, LuaStatePtr from, int narg,
            int* nres)
        {
            return lua_resume((lua_State*)L, (lua_State*)from, narg, nres);
        }
        LUNA_LUA_API int status(LuaStatePtr L)
        {
            return lua_status((lua_State*)L);
        }
        LUNA_LUA_API int isyieldable(LuaStatePtr L)
        {
            return lua_isyieldable((lua_State*)L);
        }

        LUNA_LUA_API void lua_setwarnf(LuaStatePtr L, WarnFunction* f, void* ud)
        {
            ::lua_setwarnf((lua_State*)L, (lua_WarnFunction)f, ud);
        }
        LUNA_LUA_API void lua_warning(LuaStatePtr L, const char* msg, int tocont)
        {
            ::lua_warning((lua_State*)L, msg, tocont);
        }

        LUNA_LUA_API int gc(LuaStatePtr L, GCOp what)
        {
            return lua_gc((lua_State*)L, (int)what);
        }
        LUNA_LUA_API int gcparam(LuaStatePtr L, GCParam param, int value)
        {
            return lua_gc((lua_State*)L, LUA_GCPARAM, (int)param, value);
        }
        LUNA_LUA_API int gcstep(LuaStatePtr L, usize n)
        {
            return lua_gc((lua_State*)L, LUA_GCSTEP, (int)n);
        }
        LUNA_LUA_API GCMode gcgen(LuaStatePtr L, int minormul, int majormul)
        {
            return (GCMode)lua_gc((lua_State*)L, LUA_GCGEN, minormul, majormul);
        }
        LUNA_LUA_API GCMode gcinc(LuaStatePtr L, int pause, int stepmul, int stepsize)
        {
            return (GCMode)lua_gc((lua_State*)L, LUA_GCINC, pause, stepmul, stepsize);
        }

        LUNA_LUA_API int error(LuaStatePtr L)
        {
            return lua_error((lua_State*)L);
        }
        LUNA_LUA_API int next(LuaStatePtr L, int idx)
        {
            return lua_next((lua_State*)L, idx);
        }
        LUNA_LUA_API void concat(LuaStatePtr L, int n)
        {
            lua_concat((lua_State*)L, n);
        }
        LUNA_LUA_API void len(LuaStatePtr L, int idx)
        {
            lua_len((lua_State*)L, idx);
        }
        LUNA_LUA_API usize stringtonumber(LuaStatePtr L, const char* s)
        {
            return lua_stringtonumber((lua_State*)L, s);
        }
        LUNA_LUA_API unsigned numbertocstring(LuaStatePtr L, int idx, char* buff)
        {
            return lua_numbertocstring((lua_State*)L, idx, buff);
        }
        LUNA_LUA_API Alloc* getallocf(LuaStatePtr L, void** ud)
        {
            return (Alloc*)lua_getallocf((lua_State*)L, ud);
        }
        LUNA_LUA_API void setallocf(LuaStatePtr L, Alloc* f, void* ud)
        {
            lua_setallocf((lua_State*)L, f, ud);
        }
        LUNA_LUA_API void toclose(LuaStatePtr L, int idx)
        {
            lua_toclose((lua_State*)L, idx);
        }
        LUNA_LUA_API void closeslot(LuaStatePtr L, int idx)
        {
            lua_closeslot((lua_State*)L, idx);
        }
        LUNA_LUA_API void* upvalueid(LuaStatePtr L, int fidx, int n)
        {
            return lua_upvalueid((lua_State*)L, fidx, n);
        }
        LUNA_LUA_API void upvaluejoin(LuaStatePtr L, int fidx1, int n1, int fidx2, int n2)
        {
            lua_upvaluejoin((lua_State*)L, fidx1, n1, fidx2, n2);
        }

        struct LuaModule : public Module
        {
            virtual const c8* get_name() override { return "Lua"; }
        };
    }

    LUNA_LUA_API Module* module_lua()
    {
        static Lua::LuaModule m;
        return &m;
    }
}