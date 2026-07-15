#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cmath>
#include <cstdint>

namespace {

struct CVector {
    float x;
    float y;
    float z;
};

struct CPed;
struct CVehicle;

constexpr int kModelMale01 = 7;
constexpr int kModelNrg500 = 522;
constexpr unsigned char kMissionVehicle = 2;
constexpr int kPedTypeCivMale = 4;
constexpr unsigned char kMissionPed = 2;
constexpr int kStreamingMissionRequired = 4;

constexpr DWORD kCooldownMs = 2000;
constexpr DWORD kLifetimeMs = 15000;
constexpr float kSpawnDistance = 3.0f;
constexpr float kSpawnHeight = 1.0f;
constexpr float kCleanupDistance = 450.0f;

constexpr uintptr_t kAddrFindPlayerPed = 0x56E210;
constexpr uintptr_t kAddrFindPlayerCoors = 0x56E010;
constexpr uintptr_t kAddrRequestModel = 0x4087E0;
constexpr uintptr_t kAddrLoadAllRequestedModels = 0x40EA10;
constexpr uintptr_t kAddrSetModelIsDeletable = 0x409C10;
constexpr uintptr_t kAddrPedAllocate = 0x5E4720;
constexpr uintptr_t kAddrCivilianPedCtor = 0x5DDB70;
constexpr uintptr_t kAddrSetPedCreatedBy = 0x5E47E0;
constexpr uintptr_t kAddrSetPosn = 0x420B80;
constexpr uintptr_t kAddrSetHeading = 0x43E0C0;
constexpr uintptr_t kAddrUpdateRwMatrix = 0x446F90;
constexpr uintptr_t kAddrClearWeapons = 0x5E6320;
constexpr uintptr_t kAddrWorldAdd = 0x563220;
constexpr uintptr_t kAddrCleanUpThisPed = 0x486300;
constexpr uintptr_t kAddrRemoveThisPed = 0x486240;
constexpr uintptr_t kAddrCreateCarForScript = 0x431F80;
constexpr uintptr_t kAddrSetPedInCarDirect = 0x650280;
constexpr uintptr_t kAddrCleanUpThisVehicle = 0x486670;

constexpr size_t kSizeCivilianPed = 0x79C;
constexpr size_t kOffPedHealth = 0x540;
constexpr size_t kOffPedMaxHealth = 0x544;
constexpr size_t kOffPedArmour = 0x548;
constexpr size_t kOffPedHeadingCurrent = 0x558;
constexpr size_t kOffPedHeadingGoal = 0x55C;
constexpr size_t kOffPhysicalMoveSpeed = 0x44;
constexpr size_t kOffPhysicalTurnSpeed = 0x50;

template <typename Ret, typename... Args>
Ret CallCdecl(uintptr_t address, Args... args) {
    return reinterpret_cast<Ret(__cdecl*)(Args...)>(address)(args...);
}

template <typename Ret, typename This, typename... Args>
Ret CallThis(uintptr_t address, This self, Args... args) {
    return reinterpret_cast<Ret(__thiscall*)(This, Args...)>(address)(self, args...);
}

template <typename T>
T& Field(void* base, size_t offset) {
    return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(base) + offset);
}

struct RcState {
    CPed* ped = nullptr;
    CVehicle* vehicle = nullptr;
    CVector spawnPos{};
    float heading = 0.0f;
    DWORD spawnTick = 0;
    DWORD lastUpdateTick = 0;
};

RcState gRc;
DWORD gLastSpawnTick = 0;
bool gPrevTPressed = false;
volatile bool gRunning = true;
HANDLE gThread = nullptr;

CPed* FindPlayerPedSafe() {
    return CallCdecl<CPed*, int>(kAddrFindPlayerPed, -1);
}

CVector FindPlayerCoorsSafe() {
    return CallCdecl<CVector, int>(kAddrFindPlayerCoors, -1);
}

void RequestModel(int modelId) {
    CallCdecl<void, int, int>(kAddrRequestModel, modelId, kStreamingMissionRequired);
}

void LoadAllRequestedModels() {
    CallCdecl<void, bool>(kAddrLoadAllRequestedModels, false);
}

void SetModelIsDeletable(int modelId) {
    CallCdecl<void, int>(kAddrSetModelIsDeletable, modelId);
}

CPed* CreateCivilianPed() {
    void* raw = CallCdecl<void*, unsigned int>(kAddrPedAllocate, static_cast<unsigned int>(kSizeCivilianPed));
    if (!raw) {
        return nullptr;
    }

    return CallThis<CPed*, void*, int, unsigned int>(kAddrCivilianPedCtor, raw, kPedTypeCivMale, kModelMale01);
}

void SetPedCreatedBy(CPed* ped) {
    CallThis<void, CPed*, unsigned char>(kAddrSetPedCreatedBy, ped, kMissionPed);
}

void SetEntityPosition(CPed* ped, const CVector& pos) {
    CallThis<void, CPed*, float, float, float>(kAddrSetPosn, ped, pos.x, pos.y, pos.z);
}

void SetEntityHeading(CPed* ped, float heading) {
    CallThis<void, CPed*, float>(kAddrSetHeading, ped, heading);
}

void UpdateRwMatrix(CPed* ped) {
    CallThis<void, CPed*>(kAddrUpdateRwMatrix, ped);
}

void ClearWeapons(CPed* ped) {
    CallThis<void, CPed*>(kAddrClearWeapons, ped);
}

void AddToWorld(CPed* ped) {
    CallCdecl<void, CPed*>(kAddrWorldAdd, ped);
}

void CleanUpThisPed(CPed* ped) {
    CallCdecl<void, CPed*>(kAddrCleanUpThisPed, ped);
}

void RemoveThisPed(CPed* ped) {
    CallCdecl<void, CPed*>(kAddrRemoveThisPed, ped);
}

void CleanUpThisVehicle(CVehicle* vehicle) {
    CallCdecl<void, CVehicle*>(kAddrCleanUpThisVehicle, vehicle);
}

CVehicle* CreateRcVehicle(const CVector& pos) {
    return CallCdecl<CVehicle*, int, CVector, unsigned char>(
        kAddrCreateCarForScript,
        kModelNrg500,
        pos,
        kMissionVehicle
    );
}

void PutPedInVehicle(CPed* ped, CVehicle* vehicle) {
    CallCdecl<void, CPed*, CVehicle*, int, bool>(kAddrSetPedInCarDirect, ped, vehicle, 0, true);
}

float DistanceBetween(const CVector& a, const CVector& b) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    const float dz = a.z - b.z;
    return std::sqrt((dx * dx) + (dy * dy) + (dz * dz));
}

CVector ForwardFromHeading(float heading) {
    return { -std::sinf(heading), std::cosf(heading), 0.0f };
}

bool IsPlayerReady(CPed* player) {
    return player != nullptr && Field<float>(player, kOffPedHealth) > 0.0f;
}

void ResetState() {
    gRc = {};
}

void CleanupRcPilot() {
    if (gRc.ped) {
        CleanUpThisPed(gRc.ped);
        RemoveThisPed(gRc.ped);
        gRc.ped = nullptr;
    }

    if (gRc.vehicle) {
        CleanUpThisVehicle(gRc.vehicle);
        gRc.vehicle = nullptr;
    }

    SetModelIsDeletable(kModelMale01);
    SetModelIsDeletable(kModelNrg500);
    ResetState();
}

bool SpawnRcPilot() {
    CPed* player = FindPlayerPedSafe();
    if (!IsPlayerReady(player)) {
        return false;
    }

    RequestModel(kModelMale01);
    RequestModel(kModelNrg500);
    LoadAllRequestedModels();

    const CVector playerPos = FindPlayerCoorsSafe();
    const float heading = Field<float>(player, kOffPedHeadingCurrent);
    const CVector forward = ForwardFromHeading(heading);

    CVector spawnPos{
        playerPos.x + (forward.x * kSpawnDistance),
        playerPos.y + (forward.y * kSpawnDistance),
        playerPos.z + kSpawnHeight
    };

    CVehicle* vehicle = CreateRcVehicle(spawnPos);
    if (!vehicle) {
        return false;
    }

    CPed* ped = CreateCivilianPed();
    if (!ped) {
        CleanUpThisVehicle(vehicle);
        return false;
    }

    SetPedCreatedBy(ped);
    Field<float>(ped, kOffPedHealth) = 1000.0f;
    Field<float>(ped, kOffPedMaxHealth) = 1000.0f;
    Field<float>(ped, kOffPedArmour) = 0.0f;
    Field<float>(ped, kOffPedHeadingCurrent) = heading;
    Field<float>(ped, kOffPedHeadingGoal) = heading;
    Field<CVector>(ped, kOffPhysicalMoveSpeed) = { 0.0f, 0.0f, 0.0f };
    Field<CVector>(ped, kOffPhysicalTurnSpeed) = { 0.0f, 0.0f, 0.0f };

    SetEntityPosition(ped, spawnPos);
    SetEntityHeading(ped, heading);
    UpdateRwMatrix(ped);
    ClearWeapons(ped);

    AddToWorld(ped);
    PutPedInVehicle(ped, vehicle);

    gRc.ped = ped;
    gRc.vehicle = vehicle;
    gRc.spawnPos = spawnPos;
    gRc.heading = heading;
    gRc.spawnTick = GetTickCount();
    gRc.lastUpdateTick = gRc.spawnTick;
    gLastSpawnTick = gRc.spawnTick;
    return true;
}

void UpdateRcPilot() {
    if (!gRc.vehicle) {
        return;
    }

    const DWORD now = GetTickCount();
    if ((now - gRc.spawnTick) >= kLifetimeMs) {
        CleanupRcPilot();
        return;
    }

    const CPed* player = FindPlayerPedSafe();
    if (player && DistanceBetween(gRc.spawnPos, FindPlayerCoorsSafe()) > kCleanupDistance) {
        CleanupRcPilot();
        return;
    }

    DWORD deltaMs = now - gRc.lastUpdateTick;
    if (deltaMs > 100) {
        deltaMs = 100;
    }
    gRc.lastUpdateTick = now;

    if (gRc.ped) {
        Field<float>(gRc.ped, kOffPedHeadingCurrent) = gRc.heading;
        Field<float>(gRc.ped, kOffPedHeadingGoal) = gRc.heading;
        UpdateRwMatrix(gRc.ped);
    }

    (void)deltaMs;
}

void ProcessHotkey() {
    const bool pressed = (GetAsyncKeyState('T') & 0x8000) != 0;
    if (pressed && !gPrevTPressed) {
        const DWORD now = GetTickCount();
        if (!gRc.vehicle && (now - gLastSpawnTick) >= kCooldownMs) {
            SpawnRcPilot();
        }
    }
    gPrevTPressed = pressed;
}

DWORD WINAPI MainThread(LPVOID) {
    while (gRunning) {
        __try {
            if (FindPlayerPedSafe()) {
                ProcessHotkey();
                UpdateRcPilot();
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            // Skip this tick if the game is still booting or changing state.
        }
        Sleep(15);
    }

    CleanupRcPilot();
    return 0;
}

} // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        gThread = CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
    } else if (reason == DLL_PROCESS_DETACH) {
        gRunning = false;
        if (gThread) {
            WaitForSingleObject(gThread, 500);
            CloseHandle(gThread);
            gThread = nullptr;
        }
    }
    return TRUE;
}
