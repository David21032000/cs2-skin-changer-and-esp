#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <Windows.h>
#include <psapi.h>

// Fix windows macro conflicts with interface method names
#ifdef PlaySound
#undef PlaySound
#endif
#ifdef CreateFont
#undef CreateFont
#endif
#pragma comment(lib, "psapi.lib")
#include "offsets.h"
#include "math.h"

// ============================================================
// Memory & Pattern Scanning Utilities
// ============================================================

namespace Mem {
    template <typename T>
    T Read(uintptr_t addr) {
        if (!addr) return T{};
        return *reinterpret_cast<T*>(addr);
    }

    template <typename T>
    void Write(uintptr_t addr, T val) {
        if (!addr) return;
        *reinterpret_cast<T*>(addr) = val;
    }

    inline uintptr_t ResolveRelativeAddress(uintptr_t addr, int offsetOffset = 3, int instructionSize = 7) {
        int32_t offset = Read<int32_t>(addr + offsetOffset);
        return addr + instructionSize + offset;
    }

    inline uintptr_t FindPattern(uintptr_t base, size_t size, const char* pattern, const char* mask) {
        size_t patternLen = strlen(mask);
        for (size_t i = 0; i < size - patternLen; i++) {
            bool found = true;
            for (size_t j = 0; j < patternLen; j++) {
                if (mask[j] == 'x' && Read<uint8_t>(base + i + j) != static_cast<uint8_t>(pattern[j])) {
                    found = false;
                    break;
                }
            }
            if (found) return base + i;
        }
        return 0;
    }

    inline uintptr_t FindPattern(const char* moduleName, const char* pattern, const char* mask) {
        HMODULE mod = GetModuleHandleA(moduleName);
        if (!mod) return 0;

        MODULEINFO info{};
        GetModuleInformation(GetCurrentProcess(), mod, &info, sizeof(info));

        return FindPattern(reinterpret_cast<uintptr_t>(mod), info.SizeOfImage, pattern, mask);
    }

    inline uintptr_t FindPatternIDA(const char* moduleName, const char* idaPattern) {
        // Convert IDA-style pattern "48 8B 05 ? ? ? ? 48 85 C0" to binary + mask
        std::string pattern;
        std::string mask;

        std::string p = idaPattern;
        p.erase(std::remove(p.begin(), p.end(), ' '), p.end());

        for (size_t i = 0; i < p.length(); i += 2) {
            std::string byteStr = p.substr(i, 2);
            if (byteStr[0] == '?' || byteStr[1] == '?') {
                pattern += '\x00';
                mask += '?';
            } else {
                pattern += static_cast<char>(std::stoi(byteStr, nullptr, 16));
                mask += 'x';
            }
        }

        return FindPattern(moduleName, pattern.c_str(), mask.c_str());
    }

    inline uintptr_t GetAbsoluteAddress(uintptr_t addr, int offset, int size) {
        return addr + Read<int32_t>(addr + offset) + size;
    }
}

// ============================================================
// Interface Declarations
// ============================================================

class IBaseClientDLL {
public:
    using GetAllClassesFn = void*(__fastcall*)(void*);
    void* GetAllClasses() {
        return reinterpret_cast<GetAllClassesFn>(vtable[Offsets::VIndex::GetAllClasses])(this);
    }
    void** vtable;
};

class IVEngineClient {
public:
    using GetScreenSizeFn = void(__fastcall*)(void*, int&, int&);
    using GetViewAnglesFn = void(__fastcall*)(void*, Vec3&);
    using SetViewAnglesFn = void(__fastcall*)(void*, const Vec3&);
    using IsInGameFn = bool(__fastcall*)(void*);
    using IsConnectedFn = bool(__fastcall*)(void*);
    using GetPlayerInfoFn = bool(__fastcall*)(void*, int, void*);
    using GetPlayerForUserIDFn = int(__fastcall*)(void*, int);
    using ExecuteClientCmdFn = void(__fastcall*)(void*, const char*);

    void GetScreenSize(int& w, int& h) {
        reinterpret_cast<GetScreenSizeFn>(vtable[Offsets::VIndex::GetScreenSize])(this, w, h);
    }
    void GetViewAngles(Vec3& ang) {
        reinterpret_cast<GetViewAnglesFn>(vtable[Offsets::VIndex::GetViewAngles])(this, ang);
    }
    void SetViewAngles(const Vec3& ang) {
        reinterpret_cast<SetViewAnglesFn>(vtable[Offsets::VIndex::SetViewAngles])(this, ang);
    }
    bool IsInGame() {
        return reinterpret_cast<IsInGameFn>(vtable[Offsets::VIndex::IsInGame])(this);
    }
    bool IsConnected() {
        return reinterpret_cast<IsConnectedFn>(vtable[Offsets::VIndex::IsConnected])(this);
    }
    bool GetPlayerInfo(int entIndex, void* info) {
        return reinterpret_cast<GetPlayerInfoFn>(vtable[Offsets::VIndex::GetPlayerInfo])(this, entIndex, info);
    }
    int GetPlayerForUserID(int uid) {
        return reinterpret_cast<GetPlayerForUserIDFn>(vtable[Offsets::VIndex::GetPlayerForUserID])(this, uid);
    }
    void ExecuteClientCmd(const char* cmd) {
        reinterpret_cast<ExecuteClientCmdFn>(vtable[Offsets::VIndex::ExecuteClientCmd])(this, cmd);
    }
    void** vtable;
};

class IClientEntityList {
public:
    using GetClientEntityFn = void*(__fastcall*)(void*, int);
    using GetClientEntityFromHandleFn = void*(__fastcall*)(void*, uintptr_t);
    using GetHighestEntityIndexFn = int(__fastcall*)(void*);

    void* GetClientEntity(int entIndex) {
        return reinterpret_cast<GetClientEntityFn>(vtable[Offsets::VIndex::GetClientEntity])(this, entIndex);
    }
    void* GetClientEntityFromHandle(uintptr_t handle) {
        return reinterpret_cast<GetClientEntityFromHandleFn>(vtable[Offsets::VIndex::GetClientEntityFromHandle])(this, handle);
    }
    int GetHighestEntityIndex() {
        return reinterpret_cast<GetHighestEntityIndexFn>(vtable[Offsets::VIndex::GetHighestEntityIndex])(this);
    }
    void** vtable;
};

class CGlobalVars {
public:
    float realTime;
    int frameCount;
    float absFrameTime;
    float curTime;
    float frameTime;
    int maxClients;
    int tickCount;
    float intervalPerTick;
    float interpolationAmount;
    int simTicksThisFrame;
    int networkProtocol;
    void* saveData;
    bool client;
    bool remoteClient;
    int timestampNetworkingBase;
    int timestampRandomizeWindow;
};

class IInputSystem {
public:
    using EnableInputFn = void(__fastcall*)(void*, bool);
    using ResetInputStateFn = void(__fastcall*)(void*);

    void EnableInput(bool enable) {
        reinterpret_cast<EnableInputFn>(vtable[Offsets::VIndex::EnableInput])(this, enable);
    }
    void ResetInputState() {
        reinterpret_cast<ResetInputStateFn>(vtable[Offsets::VIndex::ResetInputState])(this);
    }
    void** vtable;
};

class IMaterialSystem {
public:
    using CreateMaterialFn = void*(__fastcall*)(void*, const char*, void*);
    using FindMaterialFn = void*(__fastcall*)(void*, const char*, const char*, bool);

    void* CreateMaterial(const char* name, void* kv) {
        return reinterpret_cast<CreateMaterialFn>(vtable[Offsets::VIndex::CreateMaterial])(this, name, kv);
    }
    void* FindMaterial(const char* name, const char* group, bool complain = true) {
        return reinterpret_cast<FindMaterialFn>(vtable[Offsets::VIndex::FindMaterial])(this, name, group, complain);
    }
    void** vtable;
};

class IVModelInfo {
public:
    using GetModelIndexFn = int(__fastcall*)(void*, const char*);
    using GetModelFn = void*(__fastcall*)(void*, int);
    using GetModelNameFn = const char*(__fastcall*)(void*, void*);
    using GetStudiomodelFn = void*(__fastcall*)(void*, void*);

    int GetModelIndex(const char* name) {
        return reinterpret_cast<GetModelIndexFn>(vtable[Offsets::VIndex::GetModelIndex])(this, name);
    }
    void* GetModel(int index) {
        return reinterpret_cast<GetModelFn>(vtable[Offsets::VIndex::GetModel])(this, index);
    }
    const char* GetModelName(void* model) {
        return reinterpret_cast<GetModelNameFn>(vtable[Offsets::VIndex::GetModelName])(this, model);
    }
    void* GetStudiomodel(void* model) {
        return reinterpret_cast<GetStudiomodelFn>(vtable[Offsets::VIndex::GetStudiomodel])(this, model);
    }
    void** vtable;
};

class IPanel {
public:
    using GetNameFn = const char*(__fastcall*)(void*, void*);
    using PaintTraverseFn = void(__fastcall*)(void*, void*, bool, bool);

    const char* GetName(void* panel) {
        return reinterpret_cast<GetNameFn>(vtable[Offsets::VIndex::GetName])(this, panel);
    }
    void PaintTraverse(void* panel, bool forceRepaint, bool allowForce) {
        reinterpret_cast<PaintTraverseFn>(vtable[Offsets::VIndex::PaintTraverse])(this, panel, forceRepaint, allowForce);
    }
    void** vtable;
};

class ISurface {
public:
    using PlaySoundFn = void(__fastcall*)(void*, const char*);
    using DrawSetColorFn = void(__fastcall*)(void*, int, int, int, int);
    using DrawFilledRectFn = void(__fastcall*)(void*, int, int, int, int);
    using DrawOutlinedRectFn = void(__fastcall*)(void*, int, int, int, int);
    using DrawLineFn = void(__fastcall*)(void*, int, int, int, int);
    using DrawSetTextFontFn = void(__fastcall*)(void*, unsigned long);
    using DrawSetTextColorFn = void(__fastcall*)(void*, int, int, int, int);
    using DrawSetTextPosFn = void(__fastcall*)(void*, int, int);
    using DrawPrintTextFn = void(__fastcall*)(void*, const wchar_t*, int);
    using CreateFontFn = unsigned long(__fastcall*)(void*);
    using SetFontGlyphSetFn = bool(__fastcall*)(void*, unsigned long, const char*, int, int, int, int, int, int);

    void PlaySound(const char* name) {
        reinterpret_cast<PlaySoundFn>(vtable[Offsets::VIndex::PlaySound])(this, name);
    }
    void DrawSetColor(int r, int g, int b, int a) {
        reinterpret_cast<DrawSetColorFn>(vtable[Offsets::VIndex::DrawSetColor])(this, r, g, b, a);
    }
    void DrawFilledRect(int x1, int y1, int x2, int y2) {
        reinterpret_cast<DrawFilledRectFn>(vtable[Offsets::VIndex::DrawFilledRect])(this, x1, y1, x2, y2);
    }
    void DrawOutlinedRect(int x1, int y1, int x2, int y2) {
        reinterpret_cast<DrawOutlinedRectFn>(vtable[Offsets::VIndex::DrawOutlinedRect])(this, x1, y1, x2, y2);
    }
    void DrawLine(int x1, int y1, int x2, int y2) {
        reinterpret_cast<DrawLineFn>(vtable[Offsets::VIndex::DrawLine])(this, x1, y1, x2, y2);
    }
    void DrawSetTextFont(unsigned long font) {
        reinterpret_cast<DrawSetTextFontFn>(vtable[Offsets::VIndex::DrawSetTextFont])(this, font);
    }
    void DrawSetTextColor(int r, int g, int b, int a) {
        reinterpret_cast<DrawSetTextColorFn>(vtable[Offsets::VIndex::DrawSetTextColor])(this, r, g, b, a);
    }
    void DrawSetTextPos(int x, int y) {
        reinterpret_cast<DrawSetTextPosFn>(vtable[Offsets::VIndex::DrawSetTextPos])(this, x, y);
    }
    void DrawPrintText(const wchar_t* text, int len) {
        reinterpret_cast<DrawPrintTextFn>(vtable[Offsets::VIndex::DrawPrintText])(this, text, len);
    }
    unsigned long CreateFont() {
        return reinterpret_cast<CreateFontFn>(vtable[Offsets::VIndex::CreateFont])(this);
    }
    void SetFontGlyphSet(unsigned long font, const char* name, int tall, int weight, int blur, int scanlines, int flags, int range) {
        reinterpret_cast<SetFontGlyphSetFn>(vtable[Offsets::VIndex::SetFontGlyphSet])(this, font, name, tall, weight, blur, scanlines, flags, range);
    }
    void** vtable;
};

// ============================================================
// Global Interface Pointers
// ============================================================
namespace Interfaces {
    inline IBaseClientDLL* client = nullptr;
    inline IVEngineClient* engine = nullptr;
    inline IClientEntityList* entityList = nullptr;
    inline CGlobalVars* globalVars = nullptr;
    inline IInputSystem* inputSystem = nullptr;
    inline IMaterialSystem* materialSystem = nullptr;
    inline IVModelInfo* modelInfo = nullptr;
    inline IPanel* panel = nullptr;
    inline ISurface* surface = nullptr;

    // ============================================================
    // Interface Grabber
    // ============================================================
    inline void* GrabInterface(const char* moduleName, const char* interfaceName) {
        HMODULE mod = GetModuleHandleA(moduleName);
        if (!mod) return nullptr;

        using CreateInterfaceFn = void*(__fastcall*)(const char*, int*);
        CreateInterfaceFn createInterface =
            reinterpret_cast<CreateInterfaceFn>(GetProcAddress(mod, "CreateInterface"));
        if (!createInterface) return nullptr;

        return createInterface(interfaceName, nullptr);
    }

    inline void GrabInputSystem() {
        if (!Offsets::dwInputSystem) return;
        uintptr_t addr = Mem::Read<uintptr_t>(Offsets::client + Offsets::dwInputSystem);
        if (addr) {
            inputSystem = reinterpret_cast<IInputSystem*>(addr);
        }
    }

    inline void InitializeAll() {
        // Standard interface creation via CreateInterface
        client = reinterpret_cast<IBaseClientDLL*>(
            GrabInterface("client.dll", "Source2Client002"));
        engine = reinterpret_cast<IVEngineClient*>(
            GrabInterface("engine2.dll", "Source2EngineToClient001"));
        entityList = reinterpret_cast<IClientEntityList*>(
            GrabInterface("client.dll", "VClientEntityList001"));
        materialSystem = reinterpret_cast<IMaterialSystem*>(
            GrabInterface("materialsystem2.dll", "VMaterialSystem2_001"));
        modelInfo = reinterpret_cast<IVModelInfo*>(
            GrabInterface("engine2.dll", "VModelInfoClient004"));
        panel = reinterpret_cast<IPanel*>(
            GrabInterface("vguimatsurface.dll", "VGUI_Panel001"));
        surface = reinterpret_cast<ISurface*>(
            GrabInterface("vguimatsurface.dll", "VGUI_Surface001"));

        // GlobalVars from resolved offset
        if (Offsets::dwGlobalVars) {
            globalVars = reinterpret_cast<CGlobalVars*>(
                Mem::Read<uintptr_t>(Offsets::client + Offsets::dwGlobalVars));
        }

        // IInputSystem from dedicated offset
        GrabInputSystem();
    }
} // namespace Interfaces
