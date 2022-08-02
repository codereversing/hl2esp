#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "D3d9.lib")
#pragma comment(lib, "D3dx9.lib")

#include <Windows.h>

#include <D3D9.h>
#include <D3dx9core.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <set>
#include <string>
#include <thread>

#include "InternalFunctions.h"
#include "HookEngine.h"

#include "IServerToolsWrapper.h"
#include "IVEngineClient013Wrapper.h"
#include "VectorWrapper.h"
#include "Source.h"

bool IsEntityEnemy(const std::string& entityName) {

    static std::set<std::string> enemyNPCs{
    "npc_antlion", "npc_antlion_grub", "npc_antlionguard",
    "npc_barnacle", "npc_fastzombie", "npc_fastzombie_torso",
    "npc_headcrab", "npc_headcrab_black", "npc_headcrab_fast",
    "npc_hunter", "npc_ichthyosaur", "npc_metropolice",
    "npc_poisonzombie", "npc_rollermine", "npc_sniper",
    "npc_stalker", "npc_strider", "npc_turret_ceiling",
    "npc_turret_floor", "npc_turret_ground", "npc_vortigaunt",
    "npc_zombie", "npc_zombie_torso", "npc_zombine"
    };

    return enemyNPCs.contains(entityName);
}

void DrawTextGDI(const Vector2& screenPosition, const std::string text) {

    auto windowHandle = FindWindow(L"Valve001", L"HALF-LIFE 2 - Direct3D 9");
    auto windowDC = GetDC(windowHandle);

    SetBkColor(windowDC, RGB(0, 0, 0));
    SetBkMode(windowDC, TRANSPARENT);
    SetTextColor(windowDC, RGB(0xFF, 0xA5, 0x00));

    RECT rect{};
    DrawTextA(windowDC, text.c_str(), text.length(), &rect, DT_CALCRECT);
    auto size{ rect.right -= rect.left };
    rect.left = static_cast<LONG>(screenPosition.x - size / 2.0f);
    rect.right = static_cast<LONG>(screenPosition.x + size / 2.0f);
    rect.top = static_cast<LONG>(screenPosition.y - 20);
    rect.bottom = rect.top + size;

    DrawTextA(windowDC, text.c_str(), -1, &rect, DT_NOCLIP);
}

ID3DXFont* GetFont(IDirect3DDevice9* device) {

    static ID3DXFont* font{};

    if (font != nullptr) {
        return font;
    }

    if (device == nullptr) {
        std::cerr << "No device to create font for."
            << std::endl;
        return nullptr;
    }

    D3DXFONT_DESC fontDesc {
        .Height = 30,
        .Width = 0,
        .Weight = FW_REGULAR,
        .MipLevels = 0,
        .Italic = false,
        .CharSet = DEFAULT_CHARSET,
        .OutputPrecision = OUT_DEFAULT_PRECIS,
        .Quality = DEFAULT_QUALITY,
        .PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE,
        .FaceName = L"Consolas"
    };

    auto result{ D3DXCreateFontIndirect(device, &fontDesc, &font) };
    if (FAILED(result))
    {
        std::cerr << "Could not create font. Error = "
            << std::hex << result
            << std::endl;
    }

    return font;
}

void DrawTextD3D9(const Vector2& screenPosition, const std::string text, IDirect3DDevice9* device) {

    RECT rect{};
    GetFont(device)->DrawTextA(nullptr, text.c_str(), text.length(), &rect, DT_CALCRECT, D3DCOLOR_XRGB(0, 0, 0));
    int size{ rect.right -= rect.left };
    rect.left = static_cast<LONG>(screenPosition.x - size / 2.0f);
    rect.right = static_cast<LONG>(screenPosition.x + size / 2.0f);
    rect.top = static_cast<LONG>(screenPosition.y - 20);
    rect.bottom = rect.top + size;
    GetFont(device)->DrawTextA(nullptr, text.c_str(), -1, &rect, DT_NOCLIP, D3DCOLOR_XRGB(0xFF, 0xA5, 0x00));
}

bool FrustomTransform(const VMatrix& worldToSurface, const Vector3& point, Vector2& screen) {

    screen.x = worldToSurface.m[0][0] * point.x + worldToSurface.m[0][1] * point.y + worldToSurface.m[0][2] * point.z + worldToSurface.m[0][3];
    screen.y = worldToSurface.m[1][0] * point.x + worldToSurface.m[1][1] * point.y + worldToSurface.m[1][2] * point.z + worldToSurface.m[1][3];
    auto w = worldToSurface.m[3][0] * point.x + worldToSurface.m[3][1] * point.y + worldToSurface.m[3][2] * point.z + worldToSurface.m[3][3];

    bool facing{};
    if (w < 0.001f)
    {
        facing = false;
        screen.x *= 100000;
        screen.y *= 100000;
    }
    else
    {
        facing = true;
        float invw = 1.0f / w;
        screen.x *= invw;
        screen.y *= invw;
    }

    return facing;
}

bool WorldToScreen(const Vector3& position, Vector2& screenPosition) {

    auto worldToScreenMatrix{ GetClientEngine()->WorldToScreenMatrix() };

    auto facing{ FrustomTransform(worldToScreenMatrix, position, screenPosition) };

    int screenWidth{}, screenHeight{};
    GetClientEngine()->GetScreenSize(screenWidth, screenHeight);
    screenPosition.x = 0.5f * (1.0f + screenPosition.x) * screenWidth;
    screenPosition.y = 0.5f * (1.0f - screenPosition.y) * screenHeight;

    auto visible{ (screenPosition.x >= 0 && screenPosition.x <= screenWidth) &&
        screenPosition.y >= 0 && screenPosition.y <= screenHeight };
    if (!facing || !visible)
    {
        screenPosition.x = -640;
        screenPosition.y = -640;
        return false;
    }

    return true;
}

void DrawEnemyEntityText(IDirect3DDevice9* device) {

    auto* serverEntity{ reinterpret_cast<IServerEntity*>(
    GetServerTools()->FirstEntity()) };

    if (serverEntity != nullptr) {
        do {
            auto* modelName{ serverEntity->GetModelName().ToCStr() };
            if (modelName != nullptr) {
                auto entityName{ std::string{GetEntityName(serverEntity)} };
                //std::cerr << entityName
                //	<< std::endl;

                if (IsEntityEnemy(entityName)) {
                    auto enemyEyePosition{ GetEyePosition(serverEntity) };
                    //std::cerr << "Enemy eye position "
                    //    << enemyEyePosition.x << " "
                    //    << enemyEyePosition.y << " "
                    //    << enemyEyePosition.z
                    //    << std::endl;

                    if (device != nullptr) {
                        Vector2 screenPosition{};
                        auto shouldDraw{ WorldToScreen(enemyEyePosition, screenPosition) };

                        //std::cerr << "Enemy screen position " << " "
                        //    << screenPosition.x << " "
                        //    << screenPosition.y
                        //    << std::endl;

                        if (shouldDraw) {
                            //DrawTextGDI(screenPosition, entityName);
                            DrawTextD3D9(screenPosition, entityName, device);
                        }
                    }
                    else {
                        std::cerr << "No Direct3D device to draw with"
                            << std::endl;
                    }
                }
            }

            serverEntity = reinterpret_cast<IServerEntity*>(
                GetServerTools()->NextEntity(serverEntity));

        } while (serverEntity != nullptr);
    }
}

HRESULT WINAPI EndSceneHook(IDirect3DDevice9* device) {

    DrawEnemyEntityText(device);

    using EndSceneFnc = HRESULT(WINAPI*)(IDirect3DDevice9* device);
    auto original{ (EndSceneFnc)HookEngine::GetOriginalAddressFromHook(EndSceneHook) };
    HRESULT result{};
    if (original != nullptr) {
        result = original(device);
    }

    return result;
}

__declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID reserved) {

    static HookEngine hookEngine{};

    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        if (AllocConsole()) {
            (void)freopen("CONOUT$", "w", stdout);
            (void)freopen("CONOUT$", "w", stderr);
            SetConsoleTitle(L"Console");
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            std::cerr << "DLL Loaded." << std::endl;
        }

         (void)hookEngine.Hook(reinterpret_cast<FncPtr>(GetEndSceneAddress()),
             reinterpret_cast<HookFncPtr>(EndSceneHook));
    }

    return TRUE;
}
