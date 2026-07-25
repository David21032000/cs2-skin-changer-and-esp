#pragma once
#include <cstdint>
#include <string>
#include <Windows.h>
#include "offsets.h"

// ============================================================
// Schema System for CS2 NetVars
// ============================================================

struct SchemaFieldInfo {
    const char* name;       // Field name
    int32_t offset;         // Offset from base
    int32_t pad[2];         // Padding
    uint16_t type;          // Schema type
    uint16_t pad2;
    int32_t count;          // Element count (for arrays)
    int32_t pad3;
    void* typeHandle;       // Pointer to type info
    int32_t pad4;
    int32_t metadataSize;
    int32_t pad5;
};

struct SchemaStaticField {
    SchemaFieldInfo* fieldInfo;
    const char* typeName;
    void* typeHandle;
    int32_t pad;
    int32_t alignment;
    int32_t pad2;
    bool isStatic;
    int32_t pad3;
};

struct SchemaClassInfo {
    const char* className;
    const char* moduleName;
    int32_t sizeOf;
    int32_t pad;
    uint16_t fieldCount;
    uint16_t pad2;
    SchemaStaticField* staticFields;
    SchemaClassInfo* parent;
    SchemaClassInfo* child;
    SchemaClassInfo* sibling;
};

struct SchemaTable {
    SchemaClassInfo* classInfo;
    int32_t classCount;
    int32_t pad;
    SchemaTable* nextTable;
};

class CSchemaSystem {
public:
    using GetSchemaInstanceFn = CSchemaSystem*(__fastcall*)();

    SchemaTable* GetSchemaTable(uint32_t classCount) {
        // Walk schema tables to find matching class count
        SchemaTable* table = reinterpret_cast<SchemaTable*>(this + 0x0);
        return nullptr; // Placeholder; real impl would walk internal list
    }

    SchemaClassInfo* FindClassInfo(const char* className) {
        if (!this) return nullptr;

        int32_t tableIndex = 0;
        while (true) {
            SchemaTable* table = reinterpret_cast<SchemaTable*>(
                reinterpret_cast<uintptr_t>(this) + 0x50 + (tableIndex * sizeof(uintptr_t)));
            if (!table || table->classCount <= 0) break;

            for (int i = 0; i < table->classCount; i++) {
                SchemaClassInfo* info = &table->classInfo[i];
                if (!info || !info->className) continue;

                if (strcmp(info->className, className) == 0) {
                    return info;
                }
            }
            tableIndex++;
        }
        return nullptr;
    }

    static CSchemaSystem* GetInstance() {
        static uintptr_t schemaAddr = Offsets::schemasystem;
        if (!schemaAddr) {
            HMODULE mod = GetModuleHandleA("schemasystem.dll");
            schemaAddr = reinterpret_cast<uintptr_t>(mod);
        }
        if (!schemaAddr) return nullptr;

        // Pattern: 48 8B 05 ? ? ? ? 48 85 C0 74 ? 48 8B 40 ? C3
        // Use resolved offset from schemasystem module
        static uintptr_t ptr = 0;
        if (!ptr) {
            // The schema system instance is typically found by scanning for
            // "Schemasystem" string reference or a specific pattern
            uintptr_t addr = Mem::FindPattern("schemasystem.dll",
                "\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC0\x74\x00\x48\x8B\x40",
                "xxx????xxxx?xxx");
            if (addr) {
                ptr = Mem::ResolveRelativeAddress(addr);
            }
        }
        return ptr ? *reinterpret_cast<CSchemaSystem**>(ptr) : nullptr;
    }

    uintptr_t vtable;
};

// ============================================================
// NetVar Resolution Functions
// ============================================================

inline SchemaClassInfo* WalkClassHierarchy(SchemaClassInfo* info, const char* propName) {
    if (!info) return nullptr;

    // Check current class fields
    for (uint16_t i = 0; i < info->fieldCount; i++) {
        if (!info->staticFields) continue;
        SchemaFieldInfo* field = info->staticFields[i].fieldInfo;
        if (!field || !field->name) continue;

        if (strcmp(field->name, propName) == 0) {
            return info;
        }
    }

    // Check parent class
    if (info->parent) {
        SchemaClassInfo* result = WalkClassHierarchy(info->parent, propName);
        if (result) return result;
    }

    return nullptr;
}

inline int32_t FindNetVar(const char* className, const char* propName) {
    CSchemaSystem* schema = CSchemaSystem::GetInstance();
    if (!schema) return 0;

    SchemaClassInfo* classInfo = schema->FindClassInfo(className);
    if (!classInfo) return 0;

    // Walk the hierarchy to find the prop
    SchemaClassInfo* found = WalkClassHierarchy(classInfo, propName);
    if (!found) return 0;

    // Find the actual offset
    for (uint16_t i = 0; i < found->fieldCount; i++) {
        if (!found->staticFields) continue;
        SchemaFieldInfo* field = found->staticFields[i].fieldInfo;
        if (!field || !field->name) continue;

        if (strcmp(field->name, propName) == 0) {
            return field->offset;
        }
    }

    return 0;
}

// ============================================================
// Helper to auto-resolve netvars at startup
// ============================================================
namespace NetVars {
    inline void Initialize() {
        // Example usage:
        // Offsets::NetVar::m_iHealth = FindNetVar("C_CSPlayerPawn", "m_iHealth");
        // Offsets::NetVar::m_iTeamNum = FindNetVar("C_BaseEntity", "m_iTeamNum");
        // etc.
    }
}

// ============================================================
// NETVAR Macro for easy declaration
// ============================================================
// Usage:
//   NETVAR(m_flFlashDuration, "C_CSPlayerPawn", "m_flFlashDuration");
//   NETVAR(m_iHealth, "C_CSPlayerPawn", "m_iHealth");
//
// This will declare an int in namespace NetVars and auto-resolve
// the offset at runtime.

#define NETVAR(name, className, propName) \
    inline int name = 0; \
    namespace { \
        struct __NetVarInit_##name { \
            __NetVarInit_##name() { \
                name = FindNetVar(className, propName); \
            } \
        } __netvar_init_##name; \
    }

// ============================================================
// Offsets2Vec helper — resolve a Vec3 offset from an entity
// ============================================================
inline Vec3 GetVec3(void* entity, int32_t offset) {
    return *reinterpret_cast<Vec3*>(reinterpret_cast<uintptr_t>(entity) + offset);
}

inline void SetVec3(void* entity, int32_t offset, const Vec3& val) {
    *reinterpret_cast<Vec3*>(reinterpret_cast<uintptr_t>(entity) + offset) = val;
}
