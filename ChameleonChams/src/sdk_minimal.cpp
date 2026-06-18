#include <Windows.h>

#include "SDK/Basic.hpp"
#include "SDK/CoreUObject_classes.hpp"

SDK_NAMESPACE_START

uintptr_t InSDKUtils::GetImageBase()
{
    return reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
}

UClass* BasicFilesImplUtils::FindClassByName(const std::string& Name, bool bByFullName)
{
    return bByFullName ? UObject::FindClass(Name) : UObject::FindClassFast(Name);
}

UClass* BasicFilesImplUtils::FindClassByFullName(const std::string& Name)
{
    return UObject::FindClass(Name);
}

std::string BasicFilesImplUtils::GetObjectName(UClass* Class)
{
    return Class ? Class->GetName() : "None";
}

int32 BasicFilesImplUtils::GetObjectIndex(UClass* Class)
{
    return Class ? Class->Index : 0;
}

uint64 BasicFilesImplUtils::GetObjFNameAsUInt64(UClass* Class)
{
    return Class ? *reinterpret_cast<uint64*>(&Class->Name) : 0;
}

UObject* BasicFilesImplUtils::GetObjectByIndex(int32 Index)
{
    return UObject::GObjects->GetByIndex(Index);
}

UFunction* BasicFilesImplUtils::FindFunctionByFName(const FName* Name)
{
    if (!Name)
        return nullptr;

    for (int32 i = 0; i < UObject::GObjects->Num(); ++i)
    {
        UObject* Object = UObject::GObjects->GetByIndex(i);
        if (Object && Object->Name == *Name && Object->IsA(EClassCastFlags::Function))
            return static_cast<UFunction*>(Object);
    }

    return nullptr;
}

FName BasicFilesImplUtils::StringToName(const wchar_t*)
{
    return FName();
}

UObject* BasicFilesImplUtils::GetDefaultObjectImpl(UClass* Class)
{
    return Class ? Class->ClassDefaultObject : nullptr;
}

const FName& GetStaticName(const wchar_t*, FName& StaticName)
{
    return StaticName;
}

UObject* FWeakObjectPtr::Get() const
{
    return UObject::GObjects->GetByIndex(ObjectIndex);
}

UObject* FWeakObjectPtr::operator->() const
{
    return UObject::GObjects->GetByIndex(ObjectIndex);
}

bool FWeakObjectPtr::operator==(const FWeakObjectPtr& Other) const
{
    return ObjectIndex == Other.ObjectIndex;
}

bool FWeakObjectPtr::operator!=(const FWeakObjectPtr& Other) const
{
    return ObjectIndex != Other.ObjectIndex;
}

bool FWeakObjectPtr::operator==(const UObject* Other) const
{
    return Other && ObjectIndex == Other->Index;
}

bool FWeakObjectPtr::operator!=(const UObject* Other) const
{
    return !Other || ObjectIndex != Other->Index;
}

SDK_NAMESPACE_END
