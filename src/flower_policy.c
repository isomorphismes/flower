#include "flower_policy.h"
#include "lua.h"
#include "lauxlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { size_t used; int ticks; } PolicyBudget;
static void *allocate(void *userdata,void *pointer,size_t old_size,size_t new_size) {
    PolicyBudget *budget=userdata;
    if (!pointer) old_size=0;
    if (!new_size) { budget->used-=old_size; free(pointer); return NULL; }
    if (new_size>256*1024 || budget->used-old_size>256*1024-new_size) return NULL;
    void *next=realloc(pointer,new_size);
    if (next) budget->used=budget->used-old_size+new_size;
    return next;
}
static void instruction_limit(lua_State *state,lua_Debug *debug) {
    (void)debug;
    PolicyBudget *budget=*(PolicyBudget **)lua_getextraspace(state);
    if (++budget->ticks>100) luaL_error(state,"policy instruction budget exceeded");
}
static int set_growth(lua_State *state) {
    FlowerPolicy *policy=lua_touserdata(state,lua_upvalueindex(1));
    lua_Integer radius=luaL_checkinteger(state,2);
    if (radius<0 || radius>16) return luaL_error(state,"hop radius must be 0..16");
    FlowerPolicy next={.rate=(float)luaL_checknumber(state,1),.hop_radius=(int)radius,
        .falloff=(float)luaL_checknumber(state,3),.edge_bias=(float)luaL_checknumber(state,4),
        .radial_fraction=(float)luaL_checknumber(state,5)};
    if (!flower_policy_valid(next)) return luaL_error(state,"growth parameters out of bounds");
    *policy=next; return 0;
}
bool flower_policy_lua(FlowerPolicy *policy,const char *script,char *error,size_t error_size) {
    PolicyBudget budget={0}; FlowerPolicy next=*policy;
    lua_State *state=lua_newstate(allocate,&budget,0);
    if (!state) { if (error_size) snprintf(error,error_size,"Lua allocation failed"); return false; }
    *(PolicyBudget **)lua_getextraspace(state)=&budget;
    lua_sethook(state,instruction_limit,LUA_MASKCOUNT,1000);
    /* No standard libraries, filesystem, native pointer or simulation callbacks. */
    lua_newtable(state);
    lua_pushlightuserdata(state,&next); lua_pushcclosure(state,set_growth,1);
    lua_setfield(state,-2,"set_growth"); lua_setglobal(state,"flower");
    int result=luaL_loadbuffer(state,script,strlen(script),"flower-policy");
    if (result==LUA_OK) result=lua_pcall(state,0,0,0);
    if (result!=LUA_OK && error_size) snprintf(error,error_size,"%s",lua_tostring(state,-1));
    if (result==LUA_OK) { *policy=next; if (error_size) error[0]='\0'; }
    lua_close(state); return result==LUA_OK;
}
