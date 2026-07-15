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

struct CPhysical;
struct CPed;
struct CPedIntelligence;
struct CTaskSimpleJetPack;
struct CEntity;
struct CObject;

constexpr int kModelMale01 = 7;
constexpr int kPedTypeCivMale = 4;
constexpr int kModelJetpack = 370;
constexpr int kPedStateIdle = 1;
constexpr int kMoveStateNone = 0;
constexpr int kTaskPrimaryDefault = 4;
constexpr int kStreamingMissionRequired = 4;
constexpr unsigned char kMissionPed = 2;

constexpr DWORD kCooldownMs = 2000;
constexpr DWORD kLifetimeMs = 15000;
constexpr float kSpawnDistance = 3.0f;
constexpr float kSpawnHeight = 1.0f;
constexpr float kForwardSpeed = 22.0f;
constexpr float kClimbSpeed = 1.8f;
constexpr float kCleanupDistance = 420.0f;

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
constexpr uintptr_t kAddrSetMoveState = 0x5DEC00;
constexpr uintptr_t kAddrSetPedState = 0x5E4500;
constexpr uintptr_t kAddrClearWeapons = 0x5E6320;
constexpr uintptr_t kAddrObjectAllocate = 0x5A1EE0;
constexpr uintptr_t kAddrObjectCtor = 0x5A1D70;
constexpr uintptr_t kAddrObjectDtor = 0x59F660;
constexpr uintptr_t kAddrObjectDelete = 0x5A1F20;
constexpr uintptr_t kAddrWorldAdd = 0x563220;
constexpr uintptr_t kAddrWorldRemove = 0x563280;
constexpr uintptr_t kAddrTaskAllocate = 0x61A5A0;
constexpr uintptr_t kAddrJetpackTaskCtor = 0x67B4E0;
constexpr uintptr_t kAddrSetTask = 0x681AF0;
constexpr uintptr_t kAddrFlushTasksImmediately = 0x6818A0;
constexpr uintptr_t kAddrGetTaskJetPack = 0x601110;
constexpr uintptr_t kAddrGetBonePosition = 0x5E4280;
constexpr uintptr_t kAddrClearSpaceForMissionEntity = 0x486B00;
constexpr uintptr_t kAddrCleanUpThisPed = 0x486300;
constexpr uintptr_t kAddrRemoveThisPed = 0x486240;

constexpr size_t kSizeCivilianPed = 0x79C;
constexpr size_t kSizeJetpackTask = 0x70;
constexpr size_t kSizeObject = 0x17C;

constexpr size_t kOffPedIntelligence = 0x47C;
constexpr size_t kOffPedHealth = 0x540;
constexpr size_t kOffPedMaxHealth = 0x544;
constexpr size_t kOffPedArmour = 0x548;
constexpr size_t kOffPedHeadingCurrent = 0x558;
constexpr size_t kOffPedHeadingGoal = 0x55C;
constexpr size_t kOffPedRemovalDistMultiplier = 0x790;

constexpr size_t kOffPhysicalMoveSpeed = 0x44;
constexpr size_t kOffPhysicalTurnSpeed = 0x50;

constexpr unsigned int kBoneUpperTorso = 4;
constexpr float kJetpackBackOffset = 0.24f;
constexpr float kJetpackUpOffset = 0.08f;

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

struct RocketState {
    CPed* ped = nullptr;
    CObject* jetpackObject = nullptr;
    CVector startPos{};
    CVector position{};
    CVector direction{};
    float heading = 0.0f;
    DWORD spawnTick = 0;
    DWORD lastUpdateTick = 0;
};

RocketState gRocket;
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

void SetMoveState(CPed* ped, int moveState) {
    CallThis<void, CPed*, int>(kAddrSetMoveState, ped, moveState);
}

void SetPedState(CPed* ped, int pedState) {
    CallThis<void, CPed*, int>(kAddrSetPedState, ped, pedState);
}

void ClearWeapons(CPed* ped) {
    CallThis<void, CPed*>(kAddrClearWeapons, ped);
}

void AddToWorld(CPed* ped) {
    CallCdecl<void, CPed*>(kAddrWorldAdd, ped);
}

void AddToWorld(CObject* object) {
    CallCdecl<void, CObject*>(kAddrWorldAdd, object);
}

void RemoveFromWorld(CObject* object) {
    CallCdecl<void, CObject*>(kAddrWorldRemove, object);
}

void SetEntityPosition(CObject* object, const CVector& pos) {
    CallThis<void, CObject*, float, float, float>(kAddrSetPosn, object, pos.x, pos.y, pos.z);
}

void SetEntityHeading(CObject* object, float heading) {
    CallThis<void, CObject*, float>(kAddrSetHeading, object, heading);
}

void GetBonePosition(CPed* ped, CVector& outPosition, unsigned int boneId, bool updateSkinBones) {
    CallThis<void, CPed*, CVector&, unsigned int, bool>(kAddrGetBonePosition, ped, outPosition, boneId, updateSkinBones);
}

void* GetTaskManager(CPedIntelligence* intelligence) {
    return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(intelligence) + 0x4);
}

CTaskSimpleJetPack* CreateJetpackTask(float cruiseHeight) {
    void* raw = CallCdecl<void*, unsigned int>(kAddrTaskAllocate, static_cast<unsigned int>(kSizeJetpackTask));
    if (!raw) {
        return nullptr;
    }

    return CallThis<CTaskSimpleJetPack*, void*, const CVector*, float, int>(
        kAddrJetpackTaskCtor,
        raw,
        nullptr,
        cruiseHeight,
        0
    );
}

void SetPrimaryTask(CPed* ped, CTaskSimpleJetPack* task) {
    CPedIntelligence* intelligence = Field<CPedIntelligence*>(ped, kOffPedIntelligence);
    if (!intelligence || !task) {
        return;
    }

    CallThis<void, void*, void*, int, bool>(kAddrSetTask, GetTaskManager(intelligence), task, kTaskPrimaryDefault, false);
}

void FlushTasksImmediately(CPed* ped) {
    CPedIntelligence* intelligence = Field<CPedIntelligence*>(ped, kOffPedIntelligence);
    if (!intelligence) {
        return;
    }

    CallThis<void, void*>(kAddrFlushTasksImmediately, GetTaskManager(intelligence));
}

bool HasJetpackTask(CPed* ped) {
    CPedIntelligence* intelligence = Field<CPedIntelligence*>(ped, kOffPedIntelligence);
    if (!intelligence) {
        return false;
    }

    return CallThis<CTaskSimpleJetPack*, CPedIntelligence*>(kAddrGetTaskJetPack, intelligence) != nullptr;
}

void ClearSpaceForMissionEntity(const CVector& pos, CPed* ped) {
    CallCdecl<void, const CVector&, CPed*>(kAddrClearSpaceForMissionEntity, pos, ped);
}

void CleanUpThisPed(CPed* ped) {
    CallCdecl<void, CPed*>(kAddrCleanUpThisPed, ped);
}

void RemoveThisPed(CPed* ped) {
    CallCdecl<void, CPed*>(kAddrRemoveThisPed, ped);
}

float DistanceBetween(const CVector& a, const CVector& b) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    const float dz = a.z - b.z;
    return std::sqrt((dx * dx) + (dy * dy) + (dz * dz));
}

CVector ForwardFromHeading(float heading) {
    // GTA SA ped forward is local +Y.
    return { -std::sinf(heading), std::cosf(heading), 0.0f };
}

void ResetRocket() {
    gRocket = {};
}

bool IsPlayerReady(CPed* player) {
    return player != nullptr && Field<float>(player, kOffPedHealth) > 0.0f;
}

void PreparePed(CPed* ped, const CVector& spawnPos, float heading) {
    Field<float>(ped, kOffPedHealth) = 1000.0f;
    Field<float>(ped, kOffPedMaxHealth) = 1000.0f;
    Field<float>(ped, kOffPedArmour) = 1000.0f;
    Field<float>(ped, kOffPedHeadingCurrent) = heading;
    Field<float>(ped, kOffPedHeadingGoal) = heading;
    Field<float>(ped, kOffPedRemovalDistMultiplier) = 8.0f;
    Field<CVector>(ped, kOffPhysicalMoveSpeed) = { 0.0f, 0.0f, 0.0f };
    Field<CVector>(ped, kOffPhysicalTurnSpeed) = { 0.0f, 0.0f, 0.0f };

    SetPedCreatedBy(ped);
    SetEntityPosition(ped, spawnPos);
    SetEntityHeading(ped, heading);
    UpdateRwMatrix(ped);
    ClearWeapons(ped);
    SetMoveState(ped, kMoveStateNone);
}

CObject* CreateJetpackObject(const CVector& spawnPos, float heading) {
    void* raw = CallCdecl<void*, unsigned int>(kAddrObjectAllocate, static_cast<unsigned int>(kSizeObject));
    if (!raw) {
        return nullptr;
    }

    auto object = reinterpret_cast<CObject*>(raw);
    CallThis<void, CObject*, int, bool>(kAddrObjectCtor, object, kModelJetpack, true);
    SetEntityPosition(object, spawnPos);
    SetEntityHeading(object, heading);
    return object;
}

void UpdateJetpackObjectPosition(CPed* ped, CObject* object, float heading) {
    if (!ped || !object) {
        return;
    }

    CVector torsoPos{};
    GetBonePosition(ped, torsoPos, kBoneUpperTorso, true);

    const CVector forward = ForwardFromHeading(heading);
    const CVector backOffset{
        -forward.x * kJetpackBackOffset,
        -forward.y * kJetpackBackOffset,
        kJetpackUpOffset
    };

    SetEntityPosition(object, {
        torsoPos.x + backOffset.x,
        torsoPos.y + backOffset.y,
        torsoPos.z + backOffset.z
    });
    SetEntityHeading(object, heading + 180.0f);
}

void CleanupRocketPed() {
    if (!gRocket.ped) {
        return;
    }

    CPed* ped = gRocket.ped;
    gRocket.ped = nullptr;

    FlushTasksImmediately(ped);
    if (gRocket.jetpackObject) {
        RemoveFromWorld(gRocket.jetpackObject);
        CallThis<void, CObject*>(kAddrObjectDtor, gRocket.jetpackObject);
        CallCdecl<void, void*>(kAddrObjectDelete, gRocket.jetpackObject);
        gRocket.jetpackObject = nullptr;
    }
    CleanUpThisPed(ped);
    RemoveThisPed(ped);
    SetModelIsDeletable(kModelMale01);
    SetModelIsDeletable(kModelJetpack);
    ResetRocket();
}

bool SpawnRocketPed() {
    CPed* player = FindPlayerPedSafe();
    if (!IsPlayerReady(player)) {
        return false;
    }

    // Load the ped model first so spawn stays safe even on a fresh session.
    RequestModel(kModelMale01);
    RequestModel(kModelJetpack);
    LoadAllRequestedModels();

    const CVector playerPos = FindPlayerCoorsSafe();
    // Read heading directly from the player ped after pointer/health validation.
    const float heading = Field<float>(player, kOffPedHeadingCurrent);
    const CVector dir = ForwardFromHeading(heading);

    CVector spawnPos{
        playerPos.x + (dir.x * kSpawnDistance),
        playerPos.y + (dir.y * kSpawnDistance),
        playerPos.z + kSpawnHeight
    };

    CPed* ped = CreateCivilianPed();
    if (!ped) {
        return false;
    }

    PreparePed(ped, spawnPos, heading);
    ClearSpaceForMissionEntity(spawnPos, ped);
    AddToWorld(ped);
    SetPrimaryTask(ped, CreateJetpackTask(spawnPos.z + 60.0f));

    gRocket.jetpackObject = CreateJetpackObject(spawnPos, heading);
    if (gRocket.jetpackObject) {
        AddToWorld(gRocket.jetpackObject);
        UpdateJetpackObjectPosition(ped, gRocket.jetpackObject, heading);
    }

    gRocket.ped = ped;
    gRocket.startPos = spawnPos;
    gRocket.position = spawnPos;
    gRocket.direction = dir;
    gRocket.heading = heading;
    gRocket.spawnTick = GetTickCount();
    gRocket.lastUpdateTick = gRocket.spawnTick;
    gLastSpawnTick = gRocket.spawnTick;
    return true;
}

void UpdateRocketPed() {
    if (!gRocket.ped) {
        return;
    }

    CPed* ped = gRocket.ped;
    const DWORD now = GetTickCount();

    if (Field<float>(ped, kOffPedHealth) <= 0.0f) {
        CleanupRocketPed();
        return;
    }

    if ((now - gRocket.spawnTick) >= kLifetimeMs ||
        DistanceBetween(gRocket.startPos, gRocket.position) >= kCleanupDistance) {
        CleanupRocketPed();
        return;
    }

    DWORD deltaMs = now - gRocket.lastUpdateTick;
    if (deltaMs > 100) {
        deltaMs = 100;
    }
    gRocket.lastUpdateTick = now;

    const float dt = static_cast<float>(deltaMs) / 1000.0f;
    gRocket.position.x += gRocket.direction.x * kForwardSpeed * dt;
    gRocket.position.y += gRocket.direction.y * kForwardSpeed * dt;
    gRocket.position.z += kClimbSpeed * dt;

    // Continuous manual steering keeps the ped going straight without wiping the jetpack task state.
    SetEntityPosition(ped, gRocket.position);
    SetEntityHeading(ped, gRocket.heading);
    UpdateRwMatrix(ped);

    if (gRocket.jetpackObject) {
        UpdateJetpackObjectPosition(ped, gRocket.jetpackObject, gRocket.heading);
    }

    Field<CVector>(ped, kOffPhysicalMoveSpeed) = {
        gRocket.direction.x * kForwardSpeed,
        gRocket.direction.y * kForwardSpeed,
        kClimbSpeed
    };
    Field<CVector>(ped, kOffPhysicalTurnSpeed) = { 0.0f, 0.0f, 0.0f };
    Field<float>(ped, kOffPedHeadingCurrent) = gRocket.heading;
    Field<float>(ped, kOffPedHeadingGoal) = gRocket.heading;

    if (!HasJetpackTask(ped)) {
        SetPrimaryTask(ped, CreateJetpackTask(gRocket.position.z + 60.0f));
    }
}

void ProcessHotkey() {
    const bool pressed = (GetAsyncKeyState('T') & 0x8000) != 0;
    if (pressed && !gPrevTPressed) {
        const DWORD now = GetTickCount();
        if (!gRocket.ped && (now - gLastSpawnTick) >= kCooldownMs) {
            SpawnRocketPed();
        }
    }
    gPrevTPressed = pressed;
}

DWORD WINAPI RocketThread(LPVOID) {
    while (gRunning) {
        __try {
            if (FindPlayerPedSafe()) {
                ProcessHotkey();
                UpdateRocketPed();
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            // If the game is still booting or changing state, skip this tick instead of crashing.
        }
        Sleep(15);
    }

    CleanupRocketPed();
    return 0;
}

} // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        gThread = CreateThread(nullptr, 0, RocketThread, nullptr, 0, nullptr);
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
