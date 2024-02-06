#include <includes/Android/Logger.h>

#include <Dobby/include/dobby.h>

#include <jni.h>

#include <dlfcn.h>

#include <string_view>

#define LUA_TNONE (-1)
#define LUA_TNIL 0
#define LUA_TBOOLEAN 1
#define LUA_TLIGHTUSERDATA 2
#define LUA_TNUMBER 3
#define LUA_TSTRING 4
#define LUA_TTABLE 5
#define LUA_TFUNCTION 6
#define LUA_TUSERDATA 7
#define LUA_TTHREAD 8
#define LUA_GLOBALSINDEX (-10002)

#define ResolveMethod(FunctionName)                                                        \
    FunctionName = reinterpret_cast<decltype(FunctionName)>(dlsym(result, #FunctionName)); \
    if (nullptr == FunctionName)                                                           \
    {                                                                                      \
        LogDebug("[-] Function %s not found", #FunctionName);                              \
        break;                                                                             \
    }

using lua_State = void;
using lua_Number = double;
typedef int (*lua_CFunction)(lua_State *L);

int (*lua_pcall)(lua_State *L, int nargs, int nresults, int msgh) = nullptr;
void (*lua_call)(lua_State *L, int nargs, int nresults) = nullptr;
int (*lua_getfield)(lua_State *L, int index, const char *k) = nullptr;
void (*lua_setfield)(lua_State *L, int index, const char *k) = nullptr;
int (*lua_isuserdata)(lua_State *L, int index) = nullptr;
int (*lua_type)(lua_State *L, int index) = nullptr;
int (*lua_gettop)(lua_State *L) = nullptr;
void (*lua_settop)(lua_State *L, int index) = nullptr;
void (*lua_pushcclosure)(lua_State *L, lua_CFunction fn, int n) = nullptr;
void (*lua_rawset)(lua_State *L, int index) = nullptr;
const char *(*lua_pushstring)(lua_State *L, const char *s) = nullptr;
void (*lua_pushvalue)(lua_State *L, int index) = nullptr;
lua_Number (*lua_tonumber)(lua_State *L, int index) = nullptr;
void (*lua_pushnumber)(lua_State *L, lua_Number n) = nullptr;

int SetPlayerAttrFromOutBattleHook(lua_State *L)
{
    LogDebug("[=] SetPlayerAttrFromOutBattleHook -> state:%p stack:%d", L, lua_gettop(L));
    auto stack = lua_gettop(L);

    LogDebug("[*] SetPlayerAttrFromOutBattleHook -> Change luck to 100...");
    lua_pushnumber(L, 100);
    lua_setfield(L, 2, "luck");
    LogDebug("[*] SetPlayerAttrFromOutBattleHook -> Change speed to 150...");
    lua_pushnumber(L, 150);
    lua_setfield(L, 2, "speed");
    lua_settop(L, stack);

    lua_getfield(L, LUA_GLOBALSINDEX, "OriSetPlayerAttrFromOutBattle");
    for (int i = 1; i <= stack; ++i)
        lua_pushvalue(L, i);
    lua_call(L, stack, 0);

    lua_getfield(L, 1, "_attr");
    if (LUA_TTABLE == lua_type(L, -1))
    {
        auto attrIndex = lua_gettop(L);

        lua_getfield(L, attrIndex, "level");
        LogDebug("[=] SetPlayerAttrFromOutBattleHook -> level:%lf", lua_tonumber(L, -1));
        lua_getfield(L, attrIndex, "maxHP");
        LogDebug("[=] SetPlayerAttrFromOutBattleHook -> maxHP:%lf", lua_tonumber(L, -1));
        lua_getfield(L, attrIndex, "HPRate");
        LogDebug("[=] SetPlayerAttrFromOutBattleHook -> HPRate:%lf", lua_tonumber(L, -1));
        lua_getfield(L, attrIndex, "DMGRate");
        LogDebug("[=] SetPlayerAttrFromOutBattleHook -> DMGRate:%lf", lua_tonumber(L, -1));
        lua_getfield(L, attrIndex, "luck");
        LogDebug("[=] SetPlayerAttrFromOutBattleHook -> luck:%lf", lua_tonumber(L, -1));
        lua_getfield(L, attrIndex, "velocity");
        LogDebug("[=] SetPlayerAttrFromOutBattleHook -> velocity:%lf", lua_tonumber(L, -1));
    }
    lua_settop(L, stack);

    return 0;
}

int (*ori_lua_getfield)(lua_State *L, int index, const char *k) = nullptr;
int lua_getfield_hook(lua_State *L, int index, const char *k)
{
    // LogDebug("[=] lua_getfield_hook -> state:%p index:%d k:%s", L, index, k);

    static bool isInitialize = false;

    if (!isInitialize)
    {
        ori_lua_getfield(L, LUA_GLOBALSINDEX, "ys");
        while (LUA_TTABLE == lua_type(L, -1))
        {
            auto ysIndex = lua_gettop(L);

            ori_lua_getfield(L, -1, "Battle");
            if (LUA_TTABLE != lua_type(L, -1))
            {
                lua_settop(L, ysIndex);
                break;
            }

            ori_lua_getfield(L, -1, "BattleAttr");
            if (LUA_TTABLE != lua_type(L, -1))
            {
                lua_settop(L, ysIndex);
                break;
            }
            auto battleAttrIndex = lua_gettop(L);

            ori_lua_getfield(L, battleAttrIndex, "SetPlayerAttrFromOutBattle");
            if (LUA_TFUNCTION != lua_type(L, -1))
            {
                lua_settop(L, ysIndex);
                break;
            }
            lua_setfield(L, LUA_GLOBALSINDEX, "OriSetPlayerAttrFromOutBattle");

            lua_pushstring(L, "SetPlayerAttrFromOutBattle");
            lua_pushcclosure(L, SetPlayerAttrFromOutBattleHook, 0);
            lua_rawset(L, battleAttrIndex);

            lua_settop(L, ysIndex);
            isInitialize = true;
            LogDebug("[+] Hook ys.BattleAttr.SetPlayerAttrFromOutBattle");
            break;
        }
        lua_settop(L, -2);
    }

    auto result = ori_lua_getfield(L, index, k);
    if (isInitialize)
        DobbyDestroy(reinterpret_cast<void *>(lua_getfield));
    return result;
}

void *(*ori_dlopen)(const char *__filename, int __flag) = nullptr;
void *dlopen_hook(const char *__filename, int __flag)
{
    auto result = ori_dlopen(__filename, __flag);
    while (std::string_view::npos != std::string_view{__filename}.find("libtolua.so"))
    {
        ResolveMethod(lua_pcall);
        ResolveMethod(lua_call);
        ResolveMethod(lua_getfield);
        ResolveMethod(lua_setfield);
        ResolveMethod(lua_isuserdata);
        ResolveMethod(lua_type);
        ResolveMethod(lua_gettop);
        ResolveMethod(lua_settop);
        ResolveMethod(lua_pushcclosure);
        ResolveMethod(lua_rawset);
        ResolveMethod(lua_pushstring);
        ResolveMethod(lua_pushvalue);
        ResolveMethod(lua_tonumber);
        ResolveMethod(lua_pushnumber);

        if (0 != DobbyHook(reinterpret_cast<void *>(lua_getfield), reinterpret_cast<void *>(lua_getfield_hook), reinterpret_cast<void **>(&ori_lua_getfield)))
        {
            LogDebug("[-] DobbyHook failed");
            break;
        }

        DobbyDestroy(reinterpret_cast<void *>(dlopen));
        LogDebug("[+] Initialize libtolua success");
        break;
    }

    return result;
}

__attribute__((constructor)) static void Initialize()
{
    LogDebug("[=] =============================================Injected so has been loaded.=============================================");

    if (0 != DobbyHook(reinterpret_cast<void *>(dlopen), reinterpret_cast<void *>(dlopen_hook), reinterpret_cast<void **>(&ori_dlopen)))
        LogDebug("[-] DobbyHook failed");
}
