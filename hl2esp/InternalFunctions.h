#pragma once

#include <basetsd.h>
#include <string>

#include "VectorWrapper.h"

class IServerTools;
class IVEngineClient;
class IServerEntity;

using CBaseEntity = void;

using GetServerToolsFnc = IServerTools * (__stdcall*)();
using GetClientEngineFnc = IVEngineClient * (__stdcall*)();
using GetEyePositionFnc = Vector3(__thiscall*)(CBaseEntity* entity);

template <typename T>
T GetFunctionPointer(const std::string moduleName, const DWORD_PTR offset) {

    auto moduleBaseAddress{ GetModuleHandleA(moduleName.c_str()) };
    if (moduleBaseAddress == nullptr) {
        std::cerr << "Could not get base address of " << moduleName
            << std::endl;
        std::abort();
    }
    return reinterpret_cast<T>(
        reinterpret_cast<DWORD_PTR>(moduleBaseAddress) + offset);
}

template <typename T>
T GetClassMember(void* classPtr, const unsigned int offset)
{
    return *(reinterpret_cast<T*>(
        reinterpret_cast<char*>(
            classPtr) + offset));
}

IServerTools* GetServerTools() {

    constexpr auto globalServerToolsOffset{ 0x3FC400 };
    static GetServerToolsFnc getServerToolsFnc{ GetFunctionPointer<GetServerToolsFnc>(
        "server.dll", globalServerToolsOffset) };

    return getServerToolsFnc();
}

IVEngineClient* GetClientEngine() {

    constexpr auto globalGetClientEngineOffset{ 0xA3B30 };
    static GetClientEngineFnc getClientEngineFnc{ GetFunctionPointer<GetClientEngineFnc>(
        "engine.dll", globalGetClientEngineOffset) };

    return getClientEngineFnc();
}

Vector3 GetEyePosition(CBaseEntity* entity) {

    constexpr auto globalGetEyePositionOffset{ 0x119D00 };
    static GetEyePositionFnc getEyePositionFnc{ GetFunctionPointer<GetEyePositionFnc>(
        "server.dll", globalGetEyePositionOffset) };

    return getEyePositionFnc(entity);
}

const char* GetEntityName(IServerEntity* serverEntity)
{
    return GetClassMember<const char*>(serverEntity, 0x5C);
}

DWORD_PTR GetEndSceneAddress() {

    constexpr auto globalEndSceneOffset = 0x5C0B0;
    auto endSceneAddress{ reinterpret_cast<DWORD_PTR>(
    GetModuleHandle(L"d3d9.dll")) + globalEndSceneOffset };

    return endSceneAddress;
}
