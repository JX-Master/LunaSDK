/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Lua.hpp
* @author JXMaster
* @date 2026/2/22
*/
#pragma once
#include <Luna/Runtime/Base.hpp>
#ifndef LUNA_LUA_API
#define LUNA_LUA_API
#endif

namespace Luna
{
    namespace Lua
    {
        using LuaStatePtr = opaque_t; // Maps to LuaStatePtr
        using KContext = isize;

        using CFunction = int(LuaStatePtr);
        using KFunction = int(LuaStatePtr L, int status, KContext ctx);
        using Alloc = void*(void* ud, void* ptr, usize osize, usize nsize);

        using Reader = const char*(LuaStatePtr L, void* ud, usize* sz);
        using Writer = int(LuaStatePtr L, const void* p, usize sz, void* ud);

        using WarnFunction = void(void* ud, const char* msg, int tocont);

        constexpr usize EXTRASPACE = sizeof(void*);

        using number = f64;
        using integer = i64;
        using unsigned_integer = u64;

        constexpr int MULTRET = -1;
        constexpr int REGISTRYINDEX = -(INT_MAX/2 + 1000);
        constexpr int OK = 0;
        constexpr int YIELD = 1;
        constexpr int ERRRUN = 2;
        constexpr int ERRSYNTAX = 3;
        constexpr int ERRMEM = 4;
        constexpr int ERRERR = 5;
        constexpr int MINSTACK = 20;
        constexpr usize N2SBUFFSZ = 64;

        inline int upvalueindex(int idx)
        {
            return REGISTRYINDEX - idx;
        }

        enum class Type : int
        {
            none = -1,
            nil = 0,
            boolean = 1,
            lightuserdata = 2,
            number = 3,
            string = 4,
            table = 5,
            function = 6,
            userdata = 7,
            thread = 8
        };

        /* predefined values in the registry */
        /* index 1 is reserved for the reference mechanism */
        constexpr int RIDX_GLOBALS = 2;
        constexpr int RIDX_MAINTHREAD = 3;
        constexpr int RIDX_LAST = 3;

        /*
        ** state manipulation
        */
        LUNA_LUA_API LuaStatePtr newstate(Alloc* f, void* ud);
        LUNA_LUA_API void close(LuaStatePtr state);
        LUNA_LUA_API LuaStatePtr newthread(LuaStatePtr L);
        LUNA_LUA_API int closethread(LuaStatePtr L, LuaStatePtr from);
        LUNA_LUA_API CFunction* atpanic(LuaStatePtr L, CFunction* panicf);
        LUNA_LUA_API Version version(LuaStatePtr L);

        /*
        ** basic stack manipulation
        */

        LUNA_LUA_API int  absindex(LuaStatePtr L, int idx);
        LUNA_LUA_API int  gettop(LuaStatePtr L);
        LUNA_LUA_API void settop(LuaStatePtr L, int idx);
        LUNA_LUA_API void pushvalue(LuaStatePtr L, int idx);
        LUNA_LUA_API void rotate(LuaStatePtr L, int idx, int n);
        LUNA_LUA_API void copy(LuaStatePtr L, int fromidx, int toidx);
        LUNA_LUA_API int  checkstack(LuaStatePtr L, int n);

        LUNA_LUA_API void xmove(LuaStatePtr from, LuaStatePtr to, int n);

        /*
        ** access functions (stack -> C)
        */

        LUNA_LUA_API bool isnumber(LuaStatePtr L, int idx);
        LUNA_LUA_API bool isstring(LuaStatePtr L, int idx);
        LUNA_LUA_API bool iscfunction(LuaStatePtr L, int idx);
        LUNA_LUA_API bool isinteger(LuaStatePtr L, int idx);
        LUNA_LUA_API bool isuserdata(LuaStatePtr L, int idx);
        LUNA_LUA_API Type type(LuaStatePtr L, int idx);
        LUNA_LUA_API const char* gettypename(LuaStatePtr L, Type tp);

        LUNA_LUA_API number tonumberx(LuaStatePtr L, int idx, int *isnum);
        LUNA_LUA_API integer tointegerx(LuaStatePtr L, int idx, int *isnum);
        LUNA_LUA_API int toboolean(LuaStatePtr L, int idx);
        LUNA_LUA_API const char* tolstring(LuaStatePtr L, int idx, usize *len);
        LUNA_LUA_API unsigned_integer rawlen(LuaStatePtr L, int idx);
        LUNA_LUA_API CFunction* tocfunction(LuaStatePtr L, int idx);
        LUNA_LUA_API void* touserdata(LuaStatePtr L, int idx);
        LUNA_LUA_API LuaStatePtr tothread(LuaStatePtr L, int idx);
        LUNA_LUA_API const void* topointer(LuaStatePtr L, int idx);

        /*
        ** Comparison and arithmetic functions
        */

        enum class ArithOp : int
        {
            add  = 0,
            sub  = 1,
            mul  = 2,
            mod  = 3,
            pow  = 4,
            div  = 5,
            idiv = 6,
            band = 7,
            bor  = 8,
            bxor = 9,
            shl  = 10,
            shr  = 11,
            unm  = 12,
            bnot = 13
        };
        
        LUNA_LUA_API void  arith(LuaStatePtr L, ArithOp op);

        enum class CompareOp : int
        {
            eq = 0,
            lt = 1,
            le = 2
        };

        LUNA_LUA_API int   rawequal(LuaStatePtr L, int idx1, int idx2);
        LUNA_LUA_API int   compare(LuaStatePtr L, int idx1, int idx2, CompareOp op);

        /*
        ** push functions (C -> stack)
        */
        LUNA_LUA_API void         pushnil(LuaStatePtr L);
        LUNA_LUA_API void         pushnumber(LuaStatePtr L, number n);
        LUNA_LUA_API void         pushinteger(LuaStatePtr L, integer n);
        LUNA_LUA_API const char * pushlstring(LuaStatePtr L, const char *s, usize len);
        LUNA_LUA_API const char * pushexternalstring(LuaStatePtr L, const char* s, usize len, Alloc* falloc, void* ud);
        LUNA_LUA_API const char * pushstring(LuaStatePtr L, const char *s);
        LUNA_LUA_API const char * pushvfstring(LuaStatePtr L, const char *fmt,
                                                            va_list argp);
        LUNA_LUA_API const char * pushfstring(LuaStatePtr L, const char *fmt, ...);
        LUNA_LUA_API void pushcclosure(LuaStatePtr L, CFunction* fn, int n);
        LUNA_LUA_API void pushboolean(LuaStatePtr L, int b);
        LUNA_LUA_API void pushlightuserdata(LuaStatePtr L, void *p);
        LUNA_LUA_API int  pushthread(LuaStatePtr L);


        /*
        ** get functions (Lua -> stack)
        */
        LUNA_LUA_API int getglobal(LuaStatePtr L, const char *name);
        LUNA_LUA_API int gettable(LuaStatePtr L, int idx);
        LUNA_LUA_API int getfield(LuaStatePtr L, int idx, const char *k);
        LUNA_LUA_API int geti(LuaStatePtr L, int idx, integer n);
        LUNA_LUA_API int rawget(LuaStatePtr L, int idx);
        LUNA_LUA_API int rawgeti(LuaStatePtr L, int idx, integer n);
        LUNA_LUA_API int rawgetp(LuaStatePtr L, int idx, const void *p);

        LUNA_LUA_API void   createtable(LuaStatePtr L, int narr, int nrec);
        LUNA_LUA_API void * newuserdatauv(LuaStatePtr L, usize sz, int nuvalue);
        LUNA_LUA_API int    getmetatable(LuaStatePtr L, int objindex);
        LUNA_LUA_API int getiuservalue(LuaStatePtr L, int idx, int n);


        /*
        ** set functions (stack -> Lua)
        */
        LUNA_LUA_API void setglobal(LuaStatePtr L, const char *name);
        LUNA_LUA_API void settable(LuaStatePtr L, int idx);
        LUNA_LUA_API void setfield(LuaStatePtr L, int idx, const char *k);
        LUNA_LUA_API void seti(LuaStatePtr L, int idx, integer n);
        LUNA_LUA_API void rawset(LuaStatePtr L, int idx);
        LUNA_LUA_API void rawseti(LuaStatePtr L, int idx, integer n);
        LUNA_LUA_API void rawsetp(LuaStatePtr L, int idx, const void *p);
        LUNA_LUA_API int  setmetatable(LuaStatePtr L, int objindex);
        LUNA_LUA_API int  setiuservalue(LuaStatePtr L, int idx, int n);

        /*
        ** 'load' and 'call' functions (load and run Lua code)
        */
        LUNA_LUA_API void callk(LuaStatePtr L, int nargs, int nresults,
                                KContext ctx, KFunction* k);

        inline void call(LuaStatePtr L, int nargs, int nresults)
        {
            callk(L, nargs, nresults, 0, nullptr);
        }

        LUNA_LUA_API int pcallk(LuaStatePtr L, int nargs, int nresults, int errfunc,
                                    KContext ctx, KFunction* k);

        inline int pcall(LuaStatePtr L, int nargs, int nresults, int errfunc)
        {
            return pcallk(L, nargs, nresults, errfunc, 0, nullptr);
        }

        LUNA_LUA_API int load(LuaStatePtr L, Reader* reader, void *dt,
                                const char *chunkname, const char *mode);

        LUNA_LUA_API int dump(LuaStatePtr L, Writer* writer, void *data, int strip);


        /*
        ** coroutine functions
        */
        LUNA_LUA_API int yieldk(LuaStatePtr L, int nresults, KContext ctx,
                               KFunction* k);
        LUNA_LUA_API int resume(LuaStatePtr L, LuaStatePtr from, int narg,
                               int *nres);
        LUNA_LUA_API int status(LuaStatePtr L);
        LUNA_LUA_API int isyieldable(LuaStatePtr L);

        inline int yield(LuaStatePtr L, int nresults)
        {
            return yieldk(L, nresults, 0, nullptr);
        }


        /*
        ** Warning-related functions
        */
        LUNA_LUA_API void lua_setwarnf(LuaStatePtr L, WarnFunction* f, void *ud);
        LUNA_LUA_API void lua_warning(LuaStatePtr L, const char *msg, int tocont);

        /*
        ** garbage-collection options
        */

        enum class GCOp
        {
            stop	 = 0,
            restart	 = 1,
            collect	 = 2,
            count	 = 3,
            countb	 = 4,
            /* step	 = 5, Moved to gc_step */ 
            isrunning = 6,
            /* gen		    = 7, Moved to gc_gen */
            /* inc		    = 8, Moved to gc_inc */
        };

        /*
        ** garbage-collection parameters
        */
        enum class GCParam : int
        {
            /* parameters for generational mode */
            minormul	= 0,  /* control minor collections */
            majorminor	= 1,  /* control shift major->minor */
            minormajor	= 2,  /* control shift minor->major */

            /* parameters for incremental mode */
            pause		= 3,  /* size of pause between successive GCs */
            stepmul		= 4,  /* GC "speed" */
            stepsize	= 5   /* GC granularity */
        };

        LUNA_LUA_API int gc(LuaStatePtr L, GCOp what);
        LUNA_LUA_API int gcparam(LuaStatePtr L, GCParam param, int value);

        LUNA_LUA_API int gcstep(LuaStatePtr L, usize n);

        enum class GCMode
        {
            gen = 7,
            inc = 8
        };

        LUNA_LUA_API GCMode gcgen(LuaStatePtr L, int minormul, int majormul);
        LUNA_LUA_API GCMode gcinc(LuaStatePtr L, int pause, int stepmul, int stepsize);

        /*
        ** miscellaneous functions
        */

        LUNA_LUA_API int  error(LuaStatePtr L);

        LUNA_LUA_API int  next(LuaStatePtr L, int idx);

        LUNA_LUA_API void concat(LuaStatePtr L, int n);
        LUNA_LUA_API void len(LuaStatePtr L, int idx);

        LUNA_LUA_API usize stringtonumber(LuaStatePtr L, const char *s);
        LUNA_LUA_API unsigned numbertocstring(LuaStatePtr L, int idx, char* buff);

        LUNA_LUA_API Alloc* getallocf(LuaStatePtr L, void **ud);
        LUNA_LUA_API void setallocf(LuaStatePtr L, Alloc* f, void *ud);

        LUNA_LUA_API void toclose(LuaStatePtr L, int idx);
        LUNA_LUA_API void closeslot(LuaStatePtr L, int idx);
        LUNA_LUA_API void* upvalueid(LuaStatePtr L, int fidx, int n);
        LUNA_LUA_API void upvaluejoin(LuaStatePtr L, int fidx1, int n1, int fidx2, int n2);

        inline int resetthread(LuaStatePtr L)
        {
            return closethread(L, nullptr);
        }

        inline void* newuserdata(LuaStatePtr L, usize sz)
        {
            return newuserdatauv(L, sz, 1);
        }
        inline int getuservalue(LuaStatePtr L, int idx)
        {
            return getiuservalue(L, idx, 1);
        }
        inline int setuservalue(LuaStatePtr L, int idx)
        {
            return setiuservalue(L, idx, 1);
        }


        /*
        ** {==============================================================
        ** some useful macros
        ** ===============================================================
        */

        inline void* getextraspace(LuaStatePtr L)
        {
            return (void *)((char *)(L) - EXTRASPACE);
        }
        inline number tonumber(LuaStatePtr L, int idx)
        {
            return tonumberx(L, idx, nullptr);
        }
        inline integer tointeger(LuaStatePtr L, int idx)
        {
            return tointegerx(L, idx, nullptr);
        }
        inline void pop(LuaStatePtr L, int n)
        {
            settop(L, -n - 1);
        }
        inline void newtable(LuaStatePtr L)
        {
            createtable(L, 0, 0);
        }
        inline void pushcfunction(LuaStatePtr L, CFunction* f)
        {
            pushcclosure(L, f, 0);
        }
        inline void registerglobalfunction(LuaStatePtr L, const char *name, CFunction* f)
        {
            pushcfunction(L, f);
            setglobal(L, name);
        }
        inline bool isfunction(LuaStatePtr L, int idx)
        {
            return type(L, idx) == Type::function;
        }
        inline bool istable(LuaStatePtr L, int idx)
        {
            return type(L, idx) == Type::table;
        }
        inline bool islightuserdata(LuaStatePtr L, int idx)
        {
            return type(L, idx) == Type::lightuserdata;
        }
        inline bool isnil(LuaStatePtr L, int idx)
        {
            return type(L, idx) == Type::nil;
        }
        inline bool isboolean(LuaStatePtr L, int idx)
        {
            return type(L, idx) == Type::boolean;
        }
        inline bool isthread(LuaStatePtr L, int idx)
        {
            return type(L, idx) == Type::thread;
        }
        inline bool isnone(LuaStatePtr L, int idx)
        {
            return type(L, idx) == Type::none;
        }
        inline bool isnoneornil(LuaStatePtr L, int idx)
        {
            return (int)type(L, idx) <= 0;
        }

        inline void pushglobaltable(LuaStatePtr L)
        {
            rawgeti(L, REGISTRYINDEX, RIDX_GLOBALS);
        }

        inline const char* tostring(LuaStatePtr L, int idx)
        {
            return tolstring(L, idx, nullptr);
        }

        inline void insert(LuaStatePtr L, int idx)
        {
            rotate(L, idx, 1);
        }

        inline void remove(LuaStatePtr L, int idx)
        {
            rotate(L, idx, -1);
            pop(L, 1);
        }

        inline void replace(LuaStatePtr L, int idx)
        {
            copy(L, -1, idx);
            pop(L, 1);
        }

        /* }============================================================== */
    }

    struct Module;
    LUNA_LUA_API Module* module_lua();
}