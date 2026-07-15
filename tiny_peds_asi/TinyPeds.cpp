#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>

namespace {

struct CMatrixLink {
    float rightX;
    float rightY;
    float rightZ;
    unsigned int flags;
    float upX;
    float upY;
    float upZ;
    unsigned int pad1;
    float atX;
    float atY;
    float atZ;
    unsigned int pad2;
    float posX;
    float posY;
    float posZ;
    unsigned int pad3;
    void* m_pAttachMatrix;
    bool m_bOwnsAttachedMatrix;

    void SetScale(float scale) {
        using Fn = void(__thiscall*)(CMatrixLink*, float);
        reinterpret_cast<Fn>(0x59AED0)(this, scale);
    }

    void UpdateRW() {
        using Fn = void(__thiscall*)(CMatrixLink*);
        reinterpret_cast<Fn>(0x59BBB0)(this);
    }
};

struct CPed {
    unsigned char pad0[0x14];
    CMatrixLink* m_matrix;
    unsigned char pad1[0x06];
    short m_nModelIndex;
};

struct PoolObjectFlags {
    union {
        struct {
            unsigned char nId : 7;
            bool bEmpty : 1;
        };
        unsigned char nValue;
    };
};

template <typename T, typename B = T>
struct CPoolLite {
    B* m_pObjects;
    PoolObjectFlags* m_byteMap;
    int m_nSize;
    int m_nFirstFree;
    bool m_bOwnsAllocations;
    bool field_11;
};

constexpr uintptr_t kAddrPedPool = 0xB74490;
constexpr uintptr_t kAddrFindPlayerPed = 0x56E210;
constexpr float kTinyScale = 0.12f;
constexpr DWORD kLoopDelayMs = 16;
constexpr int kMaxTrackedPeds = 512;

template <typename Ret, typename... Args>
Ret CallCdecl(uintptr_t address, Args... args) {
    return reinterpret_cast<Ret(__cdecl*)(Args...)>(address)(args...);
}

CPoolLite<CPed, CPed>* PedPool() {
    return *reinterpret_cast<CPoolLite<CPed, CPed>**>(kAddrPedPool);
}

CPed* PlayerPed() {
    return reinterpret_cast<CPed*>(CallCdecl<void*, int>(kAddrFindPlayerPed, -1));
}

int Male01ModelId() {
    static int modelId = -1;
    if (modelId == -1) {
        using Fn = void* (__cdecl*)(const char*, int*);
        reinterpret_cast<Fn>(0x4C5940)("male01", &modelId);
    }
    return modelId;
}

void TinyifyPed(CPed* ped) {
    if (!ped) {
        return;
    }

    if (ped == PlayerPed()) {
        return;
    }

    if (ped->m_nModelIndex != Male01ModelId()) {
        return;
    }

    if (!ped->m_matrix) {
        return;
    }

    ped->m_matrix->SetScale(kTinyScale);
    ped->m_matrix->UpdateRW();
}

void ProcessTinyPeds() {
    auto* pool = PedPool();
    if (!pool || !pool->m_pObjects || !pool->m_byteMap) {
        return;
    }

    static CPed* trackedPeds[kMaxTrackedPeds]{};
    static int trackedCount = 0;
    CPed* player = PlayerPed();
    for (int i = 0; i < pool->m_nSize; ++i) {
        if (pool->m_byteMap[i].bEmpty) {
            continue;
        }

        CPed* ped = &pool->m_pObjects[i];
        if (ped == player || ped->m_nModelIndex != Male01ModelId()) {
            continue;
        }

        bool alreadyTracked = false;
        for (int j = 0; j < trackedCount; ++j) {
            if (trackedPeds[j] == ped) {
                alreadyTracked = true;
                break;
            }
        }

        if (!alreadyTracked) {
            TinyifyPed(ped);
            if (trackedCount < kMaxTrackedPeds) {
                trackedPeds[trackedCount++] = ped;
            }
        }
    }

    // Keep the tracked list compact by dropping deleted entries.
    int writeIndex = 0;
    for (int i = 0; i < trackedCount; ++i) {
        CPed* ped = trackedPeds[i];
        if (!ped || ped == player) {
            continue;
        }

        bool stillExists = false;
        for (int j = 0; j < pool->m_nSize; ++j) {
            if (!pool->m_byteMap[j].bEmpty && &pool->m_pObjects[j] == ped) {
                stillExists = true;
                break;
            }
        }

        if (stillExists) {
            trackedPeds[writeIndex++] = ped;
        }
    }

    trackedCount = writeIndex;
}

DWORD WINAPI MainThread(LPVOID) {
    Sleep(1000);

    while (true) {
        __try {
            ProcessTinyPeds();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            // Safe fallback: skip a bad ped and keep the game alive.
        }
        Sleep(kLoopDelayMs);
    }

    return 0;
}

} // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
    }

    return TRUE;
}
