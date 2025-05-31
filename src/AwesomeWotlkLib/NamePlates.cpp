#include "NamePlates.h"
#include "GameClient.h"
#include "Hooks.h"
#include <Windows.h>
#include <Detours/detours.h>
#include <algorithm>
#include <vector>
#include <cstring>
#define NAME_PLATE_CREATED "NAME_PLATE_CREATED"
#define NAME_PLATE_UNIT_ADDED "NAME_PLATE_UNIT_ADDED"
#define NAME_PLATE_UNIT_REMOVED "NAME_PLATE_UNIT_REMOVED"

using NamePlateFlags = uint32_t;
enum NamePlateFlag_ {
    NamePlateFlag_Null = 0,
    NamePlateFlag_Created = (1 << 0),
    NamePlateFlag_Visible = (1 << 1),
    NamePlateFlag_CreatedAndVisible = NamePlateFlag_Created | NamePlateFlag_Visible,
};

struct NamePlateEntry {
    NamePlateEntry() : nameplate(NULL), guid(0), flags(NamePlateFlag_Null), updateId(0) {}
    Frame* nameplate;
    guid_t guid;
    NamePlateFlags flags;
    uint32_t updateId;
};

struct NamePlateVars {
    NamePlateVars() : updateId(1) {}
    std::vector<NamePlateEntry> nameplates;
    uint32_t updateId;
};

static Console::CVar* s_cvar_nameplateDistance;

static NamePlateVars& lua_findorcreatevars(lua_State* L)
{
    struct Dummy {
        static int lua_gc(lua_State* L)
        {
            ((NamePlateVars*)lua_touserdata(L, -1))->~NamePlateVars();
            return 0;
        }
    };
    NamePlateVars* result = NULL;
    lua_getfield(L, LUA_REGISTRYINDEX, "nameplatevars"); // vars
    if (lua_isuserdata(L, -1))
        result = (NamePlateVars*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    if (!result) {
        result = (NamePlateVars*)lua_newuserdata(L, sizeof(NamePlateVars)); // vars
        new (result) NamePlateVars();
        
        lua_createtable(L, 0, 1); // vars, mt
        lua_pushcfunction(L, Dummy::lua_gc); // vars, mt, gc
        lua_setfield(L, -2, "__gc"); // vars, mt
        lua_setmetatable(L, -2); // vars
        lua_setfield(L, LUA_REGISTRYINDEX, "nameplatevars");
    }
    return *result;
}

static guid_t getTokenGuid(int id)
{
    NamePlateVars& vars = lua_findorcreatevars(GetLuaState());
    if (id >= vars.nameplates.size())
        return 0;
    return vars.nameplates[id].guid;
}

static NamePlateEntry* getEntryByGuid(guid_t guid)
{
    if (!guid) return NULL;
    NamePlateVars& vars = lua_findorcreatevars(GetLuaState());
    auto it = std::find_if(vars.nameplates.begin(), vars.nameplates.end(), [guid](const NamePlateEntry& entry) {
        return (entry.flags & NamePlateFlag_Visible) && entry.guid == guid;
    });
    return it != vars.nameplates.end() ? &(*it) : NULL;
}

static int getTokenId(guid_t guid)
{
    if (!guid) return -1;
    NamePlateVars& vars = lua_findorcreatevars(GetLuaState());
    for (size_t i = 0; i < vars.nameplates.size(); i++) {
        NamePlateEntry& entry = vars.nameplates[i];
        if ((entry.flags & NamePlateFlag_CreatedAndVisible) == NamePlateFlag_CreatedAndVisible && entry.guid == guid)
            return i;
    }
    return -1;
}

static int CVarHandler_NameplateDistance(Console::CVar*, const char*, const char* value, LPVOID)
{
    double f = atof(value);
    f = f > 0.f ? f : 41.f;
    *(float*)0x00ADAA7C = (float)(f * f);
    return 1;
}

static int C_NamePlate_GetNamePlates(lua_State* L)
{
    lua_createtable(L, 0, 0);
    NamePlateVars& vars = lua_findorcreatevars(L);
    int id = 1;
    for (NamePlateEntry& entry : vars.nameplates) {
        if ((entry.flags & NamePlateFlag_Visible) && entry.guid) {
            lua_pushframe(L, entry.nameplate);
            lua_rawseti(L, -2, id++);
        }
    }
    return 1;
}

static int C_NamePlate_GetNamePlateForUnit(lua_State* L)
{
    const char* token = luaL_checkstring(L, 1);
    guid_t guid = ObjectMgr::GetGuidByUnitID(token);
    if (!guid) return 0;
    NamePlateEntry* entry = getEntryByGuid(guid);
    if (!entry) return 0;
    lua_pushframe(L, entry->nameplate);
    return 1;
}

static int C_NamePlate_GetNamePlatesDistance(lua_State* L)
{
    lua_createtable(L, 0, 0);
    NamePlateVars& vars = lua_findorcreatevars(L);
    Player* player = ObjectMgr::GetPlayer();
    if (player) {
        VecXYZ posPlayer;
        player->ToUnit()->vmt->GetPosition(player->ToUnit(), &posPlayer);
        for (NamePlateEntry& entry : vars.nameplates) {
            if ((entry.flags & NamePlateFlag_Visible) && entry.guid) {      
                Unit* unit = (Unit*)ObjectMgr::Get(entry.guid, ObjectFlags_Unit);
                if (unit) {
                    VecXYZ unitPos;
                    unit->vmt->GetPosition(unit, &unitPos);
                    float distance = posPlayer.distance(unitPos);
                    lua_pushframe(L, entry.nameplate);
                    lua_pushnumber(L, distance);
                    lua_rawset(L, -3);
                }
            }
        }
    }
    return 1;
}

static int C_NamePlate_GetNamePlatesDistanceList(lua_State* L)
{
    lua_createtable(L, 0, 0);
    NamePlateVars& vars = lua_findorcreatevars(L);
    Player* player = ObjectMgr::GetPlayer();
    if (player) {
        VecXYZ posPlayer;
        player->ToUnit()->vmt->GetPosition(player->ToUnit(), &posPlayer);
        int id = 1;
        for (NamePlateEntry& entry : vars.nameplates) {
            if ((entry.flags & NamePlateFlag_Visible) && entry.guid) {
                Unit* unit = (Unit*)ObjectMgr::Get(entry.guid, ObjectFlags_Unit);
                if (unit) {
                    VecXYZ unitPos;
                    unit->vmt->GetPosition(unit, &unitPos);
                    float distance = posPlayer.distance(unitPos);
                    lua_createtable(L, 0, 2);
                    lua_pushframe(L, entry.nameplate);
                    lua_setfield(L, -2, "nameplate");
                    lua_pushnumber(L, distance);
                    lua_setfield(L, -2, "distance");
                    lua_rawseti(L, -2, id++);
                }
            }
        }
    }
    return 1;
}

static int C_NamePlate_GetDistanceForUnit(lua_State* L)
{
    const char* unitName = luaL_checkstring(L, 1);
    guid_t guid = ObjectMgr::GetGuidByUnitID(unitName);
    if (!guid) {
        lua_pushnil(L);
        return 1;
    }
    NamePlateEntry* entry = getEntryByGuid(guid);
    if (!entry) {
        lua_pushnil(L);
        return 1;
    }
    Player* player = ObjectMgr::GetPlayer();
    if (player) {
        VecXYZ posPlayer;
        player->ToUnit()->vmt->GetPosition(player->ToUnit(), &posPlayer);
        Unit* unit = (Unit*)ObjectMgr::Get(guid, ObjectFlags_Unit);
        if (unit) {
            VecXYZ unitPos;
            unit->vmt->GetPosition(unit, &unitPos);
            float distance = posPlayer.distance(unitPos);
            lua_pushnumber(L, distance);
        }
        else {
            lua_pushnil(L);
        }
    }
    else {
        lua_pushnil(L);
    }
    return 1;
}

static int C_NamePlate_GetDistanceForGUID(lua_State* L)
{
    const char* guidStr = luaL_checkstring(L, 1);
    if (!guidStr || strncmp(guidStr, "0x", 2) != 0) {
        lua_pushnil(L);
        return 1;
    }
    guid_t guid = 0;
    int matched = sscanf(guidStr, "0x%llx", &guid);
    if (matched != 1 || guid == 0) {
        lua_pushnil(L);
        return 1;
    }
    NamePlateEntry* entry = getEntryByGuid(guid);
    if (!entry) {
        lua_pushnil(L);
        return 1;
    }
    Player* player = ObjectMgr::GetPlayer();
    if (!player) {
        lua_pushnil(L);
        return 1;
    }
    VecXYZ posPlayer;
    player->ToUnit()->vmt->GetPosition(player->ToUnit(), &posPlayer);

    Unit* unit = (Unit*)ObjectMgr::Get(guid, ObjectFlags_Unit);
    if (!unit) {
        lua_pushnil(L);
        return 1;
    }
    VecXYZ unitPos;
    unit->vmt->GetPosition(unit, &unitPos);
    float distance = posPlayer.distance(unitPos);
    lua_pushnumber(L, distance);
    return 1;
}

static int C_NamePlate_GetDistanceForNamePlate(lua_State* L) {
    NamePlateVars& vars = lua_findorcreatevars(L);
    Player* player = ObjectMgr::GetPlayer();
    if (!player) {
        lua_pushnil(L);
        return 1;
    }
    if (!lua_istable(L, 1)) {
        lua_pushnil(L);
        return 1;
    }
    lua_rawgeti(L, 1, 0);
    if (!lua_isuserdata(L, -1)) {
        lua_pushnil(L);
        return 1;
    }
    void* nameplateUserdata = lua_touserdata(L, -1);
    lua_pop(L, 1);
    VecXYZ posPlayer;
    player->ToUnit()->vmt->GetPosition(player->ToUnit(), &posPlayer);
    for (NamePlateEntry& entry : vars.nameplates) {
        if ((entry.flags & NamePlateFlag_Visible) && entry.guid && entry.nameplate == nameplateUserdata) {
            Unit* unit = (Unit*)ObjectMgr::Get(entry.guid, ObjectFlags_Unit);
            if (unit) {
                VecXYZ unitPos;
                unit->vmt->GetPosition(unit, &unitPos);
                float distance = posPlayer.distance(unitPos);
                lua_pushnumber(L, distance);
                return 1;
            }
        }
    }
    lua_pushnil(L);
    return 1;
}

static int lua_openlibnameplates(lua_State* L)
{
    luaL_Reg methods[] = {
        {"GetNamePlates", C_NamePlate_GetNamePlates},
        {"GetNamePlateForUnit", C_NamePlate_GetNamePlateForUnit},
        {"GetNamePlatesDistance", C_NamePlate_GetNamePlatesDistance},
        {"GetNamePlatesDistanceList", C_NamePlate_GetNamePlatesDistanceList},
        {"GetDistanceForUnit", C_NamePlate_GetDistanceForUnit},
        {"GetDistanceForGUID", C_NamePlate_GetDistanceForGUID},
        {"GetDistanceForNamePlate", C_NamePlate_GetDistanceForNamePlate},
    };
    
    lua_createtable(L, 0, std::size(methods));
    for (size_t i = 0; i < std::size(methods); i++) {
        lua_pushcfunction(L, methods[i].func);
        lua_setfield(L, -2, methods[i].name);
    }
    lua_setglobal(L, "C_NamePlate");
    return 0;
}

static void onUpdateCallback()
{
    if (!IsInWorld()) return;

    static std::vector<std::tuple<Frame*, guid_t, float>> s_plateSort;

    lua_State* L = GetLuaState();
    NamePlateVars& vars = lua_findorcreatevars(L);

    Player* player = ObjectMgr::GetPlayer();
    VecXYZ posPlayer;
    if (player) player->ToUnit()->vmt->GetPosition(player->ToUnit(), &posPlayer);

    ObjectMgr::EnumObjects([&vars, player, &posPlayer](guid_t guid) -> bool {
        Unit* unit = (Unit*)ObjectMgr::Get(guid, ObjectFlags_Unit);
        if (!unit || !unit->nameplate) return true;
        auto it = std::find_if(vars.nameplates.begin(), vars.nameplates.end(), [nameplate = unit->nameplate](const NamePlateEntry& entry) {
            return entry.nameplate == nameplate;
        });
        if (it == vars.nameplates.end()) {
            NamePlateEntry& entry = vars.nameplates.emplace_back();
            entry.guid = guid;
            entry.nameplate = unit->nameplate;
            entry.updateId = vars.updateId;
        } else {
            if (it->guid != guid) {
                // FIXME: potential problem with silent changing real unit
                it->guid = guid;
            }
            it->updateId = vars.updateId;
        }

        if (player) {
            VecXYZ unitPos;
            unit->vmt->GetPosition(unit, &unitPos);
            s_plateSort.push_back({ unit->nameplate, guid, posPlayer.distance(unitPos) });
        }
        
        return true;
    });

    if (!s_plateSort.empty()) {
        std::sort(s_plateSort.begin(), s_plateSort.end(), [targetGuid = ObjectMgr::GetTargetGuid()](auto& a1, auto& a2) {
            auto& [frame1, guid1, distance1] = a1;
            auto& [frame2, guid2, distance2] = a2;
            if (guid1 == targetGuid) return false;
            if (guid2 == targetGuid) return true;
            return distance1 > distance2;
        });

        int level = 10;
        for (auto& entry : s_plateSort)
            CFrame::SetFrameLevel(std::get<0>(entry), level++, 1);

        s_plateSort.clear();
    }

    for (size_t i = 0; i < vars.nameplates.size(); i++) {
        NamePlateEntry& entry = vars.nameplates[i];
        if (entry.updateId == vars.updateId) {
            if (!(entry.flags & NamePlateFlag_Created)) {
                lua_pushstring(L, NAME_PLATE_CREATED); // tbl, event
                lua_pushframe(L, entry.nameplate); // tbl,  event, frame
                FrameScript::FireEvent_inner(FrameScript::GetEventIdByName(NAME_PLATE_CREATED), L, 2); // tbl
                lua_pop(L, 2);
                entry.flags |= NamePlateFlag_Created;
            }

            if (!(entry.flags & NamePlateFlag_Visible)) {
                entry.flags |= NamePlateFlag_Visible;
                char token[16];
                snprintf(token, std::size(token), "nameplate%d", i + 1);
                FrameScript::FireEvent(NAME_PLATE_UNIT_ADDED, "%s", token);
            }
        } else {
            if (entry.flags & NamePlateFlag_Visible) {
                char token[16];
                snprintf(token, std::size(token), "nameplate%d", i + 1);
                FrameScript::FireEvent(NAME_PLATE_UNIT_REMOVED, "%s", token);
                entry.guid = 0;
                entry.flags &= ~NamePlateFlag_Visible;
            }
        }
    }

    vars.updateId++;
}

LPVOID PatchNamePlateLevelUpdate_orig = (LPVOID)0x0098E9F9;
void __declspec(naked) PatchNamePlateLevelUpdate_hk()
{
    __asm {
        push edi;
        push 0x0098EA27;
        ret;
    }
}

void NamePlates::initialize()
{
    Hooks::FrameXML::registerLuaLib(lua_openlibnameplates);
    Hooks::FrameXML::registerEvent(NAME_PLATE_CREATED);
    Hooks::FrameXML::registerEvent(NAME_PLATE_UNIT_ADDED);
    Hooks::FrameXML::registerEvent(NAME_PLATE_UNIT_REMOVED);
    Hooks::FrameXML::registerCVar(&s_cvar_nameplateDistance, "nameplateDistance", NULL, (Console::CVarFlags)1, "41", CVarHandler_NameplateDistance);
    Hooks::FrameScript::registerToken("nameplate", getTokenGuid, getTokenId);
    Hooks::FrameScript::registerOnUpdate(onUpdateCallback);

    DetourAttach(&(LPVOID&)PatchNamePlateLevelUpdate_orig, PatchNamePlateLevelUpdate_hk);
}
