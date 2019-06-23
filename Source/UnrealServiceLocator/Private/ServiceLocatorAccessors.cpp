///////////////////////////////////////////////////////////////////////////
// ServiceLocatorAccessors.cpp
///////////////////////////////////////////////////////////////////////////

// UnrealServiceLocator
#include "ServiceLocatorAccessors.h"
#include "ServiceLocatorContainer.h"
#include "ServiceLocatorInterface.h"

// Engine
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"

///////////////////////////////////////////////////////////////////////////

UObject* ServiceLocatorAccessors_Private::GetServiceFromObjectInternal(const UClass* ServiceType, const UObject& Object)
{
	const IServiceLocatorInterface* ObjectAsServiceLocatorInterface = Cast<const IServiceLocatorInterface>(&Object);
	if (ObjectAsServiceLocatorInterface == nullptr)
		return nullptr;

	return GetServiceFromObjectInternal(ServiceType, *ObjectAsServiceLocatorInterface);
}

///////////////////////////////////////////////////////////////////////////

UObject* ServiceLocatorAccessors_Private::GetServiceFromObjectInternal(const UClass* ServiceType, const IServiceLocatorInterface& Object)
{
	UServiceLocatorContainer* Container = Object.GetContainer();
	if (Container == nullptr)
		return nullptr;

	return Container->GetService(ServiceType);
}

///////////////////////////////////////////////////////////////////////////

UObject* ServiceLocatorAccessors_Private::GetServiceInterfaceFromObjectInternal(const UClass* ServiceInterfaceType, const UObject& Object)
{
	const IServiceLocatorInterface* ObjectAsServiceLocatorInterface = Cast<const IServiceLocatorInterface>(&Object);
	if (ObjectAsServiceLocatorInterface == nullptr)
		return nullptr;

	return GetServiceInterfaceFromObjectInternal(ServiceInterfaceType, *ObjectAsServiceLocatorInterface);
}

///////////////////////////////////////////////////////////////////////////

UObject* ServiceLocatorAccessors_Private::GetServiceInterfaceFromObjectInternal(const UClass* ServiceInterfaceType, const IServiceLocatorInterface& Object)
{
	UServiceLocatorContainer* Container = Object.GetContainer();
	if (Container == nullptr)
		return nullptr;

	return Container->GetServiceInterface(ServiceInterfaceType);
}

///////////////////////////////////////////////////////////////////////////

UObject* ServiceLocatorAccessors_Private::GetServiceFromGameStateInternal(const UClass* ServiceType, const UObject& WorldContextObject)
{
	UWorld* const World = GEngine->GetWorldFromContextObject(&WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World == nullptr)
		return nullptr;

	AGameStateBase* GameStateBase = World->GetGameState();
	if (GameStateBase == nullptr)
		return nullptr;

	return GetServiceFromObjectInternal(ServiceType, *GameStateBase);
}

///////////////////////////////////////////////////////////////////////////

UObject* ServiceLocatorAccessors_Private::GetServiceInterfaceFromGameStateInternal(const UClass* ServiceInterfaceType, const UObject& WorldContextObject)
{
	UWorld* const World = GEngine->GetWorldFromContextObject(&WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World == nullptr)
		return nullptr;

	AGameStateBase* GameStateBase = World->GetGameState();
	if (GameStateBase == nullptr)
		return nullptr;

	return GetServiceInterfaceFromObjectInternal(ServiceInterfaceType, *GameStateBase);
}

///////////////////////////////////////////////////////////////////////////
