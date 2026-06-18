#include <Windows.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>

#include "SDK/CoreUObject_classes.hpp"
#include "SDK/Engine_classes.hpp"

namespace
{
using namespace SDK;
using namespace std::chrono_literals;

constexpr int32 kStencilValue = 252;

std::atomic_bool g_Running{true};
std::mutex g_RescanMutex;

struct SetRenderCustomDepthParams
{
    bool bValue;
};

struct SetCustomDepthStencilValueParams
{
    int32 Value;
};

struct ViewInOutFunctionParams
{
    int32 Stencil_Value;
};

bool Contains(const std::string& Text, const char* Needle)
{
    return Text.find(Needle) != std::string::npos;
}

bool IsUsableObject(const UObject* Object)
{
    return Object && Object->Class && !Object->IsDefaultObject();
}

bool ClassChainContains(const UClass* Class, const char* Needle)
{
    for (const UStruct* Current = Class; Current; Current = Current->SuperStruct)
    {
        if (Contains(Current->GetName(), Needle))
            return true;
    }

    return false;
}

bool IsTargetActor(AActor* Actor)
{
    if (!IsUsableObject(Actor))
        return false;

    const UClass* Class = Actor->Class;

    if (Actor->IsA(ACharacter::StaticClass()))
        return ClassChainContains(Class, "FirstPersonCharacter")
            || ClassChainContains(Class, "AI_Base")
            || ClassChainContains(Class, "BigPen")
            || ClassChainContains(Class, "MiniPenguin")
            || ClassChainContains(Class, "Penguin");

    return ClassChainContains(Class, "BP_Simple_VAT_Player");
}

UFunction* FindFunctionInClassChain(const UClass* Class, const char* FunctionName)
{
    if (!Class)
        return nullptr;

    for (const UStruct* CurrentClass = Class; CurrentClass; CurrentClass = CurrentClass->SuperStruct)
    {
        for (UField* Field = CurrentClass->Children; Field; Field = Field->Next)
        {
            if (Field->HasTypeFlag(EClassCastFlags::Function) && Field->GetName() == FunctionName)
                return static_cast<UFunction*>(Field);
        }
    }

    return nullptr;
}

void CallViewInOut(AActor* Actor)
{
    UFunction* Function = FindFunctionInClassChain(Actor->Class, "View In Out Function");
    if (!Function)
        return;

    ViewInOutFunctionParams Params{kStencilValue};
    Actor->ProcessEvent(Function, &Params);
}

void SetPrimitiveChams(UPrimitiveComponent* Component)
{
    if (!IsUsableObject(Component))
        return;

    if (UFunction* SetStencil = Component->Class->GetFunction("PrimitiveComponent", "SetCustomDepthStencilValue"))
    {
        SetCustomDepthStencilValueParams Params{kStencilValue};
        Component->ProcessEvent(SetStencil, &Params);
    }

    if (UFunction* SetCustomDepth = Component->Class->GetFunction("PrimitiveComponent", "SetRenderCustomDepth"))
    {
        SetRenderCustomDepthParams Params{true};
        Component->ProcessEvent(SetCustomDepth, &Params);
    }
}

void ApplyActorChams(AActor* Actor)
{
    CallViewInOut(Actor);

    if (Actor->IsA(ACharacter::StaticClass()))
    {
        auto* Character = static_cast<ACharacter*>(Actor);
        SetPrimitiveChams(Character->Mesh);
        return;
    }

    if (Actor->IsA(AActor::StaticClass()))
    {
        UFunction* GetComponents = Actor->Class->GetFunction("Actor", "K2_GetComponentsByClass");
        if (!GetComponents)
            return;

        struct K2GetComponentsByClassParams
        {
            TSubclassOf<UActorComponent> ComponentClass;
            TArray<UActorComponent*> ReturnValue;
        };

        K2GetComponentsByClassParams Params{};
        Params.ComponentClass = UPrimitiveComponent::StaticClass();

        Actor->ProcessEvent(GetComponents, &Params);

        for (UActorComponent* Component : Params.ReturnValue)
        {
            SetPrimitiveChams(static_cast<UPrimitiveComponent*>(Component));
        }
    }
}

int32 RescanAndApply()
{
    std::lock_guard Lock(g_RescanMutex);

    int32 AppliedActors = 0;
    const int32 ObjectCount = UObject::GObjects->Num();

    for (int32 i = 0; i < ObjectCount; ++i)
    {
        UObject* Object = UObject::GObjects->GetByIndex(i);
        if (!IsUsableObject(Object) || !Object->IsA(AActor::StaticClass()))
            continue;

        AActor* Actor = static_cast<AActor*>(Object);
        if (!IsTargetActor(Actor))
            continue;

        ApplyActorChams(Actor);
        ++AppliedActors;
    }

    return AppliedActors;
}

DWORD WINAPI WorkerThread(void* Module)
{
    Sleep(5000);
    RescanAndApply();

    bool InsertWasDown = false;
    bool EndWasDown = false;

    while (g_Running.load(std::memory_order_relaxed))
    {
        const bool InsertDown = (GetAsyncKeyState(VK_INSERT) & 0x8000) != 0;
        if (InsertDown && !InsertWasDown)
            RescanAndApply();
        InsertWasDown = InsertDown;

        const bool EndDown = (GetAsyncKeyState(VK_END) & 0x8000) != 0;
        if (EndDown && !EndWasDown)
            g_Running.store(false, std::memory_order_relaxed);
        EndWasDown = EndDown;

        std::this_thread::sleep_for(50ms);
    }

    FreeLibraryAndExitThread(static_cast<HMODULE>(Module), 0);
}
}

BOOL APIENTRY DllMain(HMODULE Module, DWORD Reason, void*)
{
    if (Reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(Module);

        HANDLE Thread = CreateThread(nullptr, 0, WorkerThread, Module, 0, nullptr);
        if (Thread)
            CloseHandle(Thread);
    }
    else if (Reason == DLL_PROCESS_DETACH)
    {
        g_Running.store(false, std::memory_order_relaxed);
    }

    return TRUE;
}
