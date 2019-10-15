///////////////////////////////////////////////////////////////////////////
// ServiceLocatorAccessors.h
///////////////////////////////////////////////////////////////////////////

#pragma once

// Engine
#include "CoreMinimal.h"

// UnrealServiceLocator
#include "ServiceLocatorContainer.h"

// Forward Declarations
class IServiceLocatorInterface;

///////////////////////////////////////////////////////////////////////////

template<typename ServiceType, typename ObjectType>
FORCEINLINE_DEBUGGABLE static ServiceType* GetServiceFromObject(const ObjectType* Object)
{
	return UServiceLocatorContainer::GetService<ServiceType, ObjectType>(Object);
}

template<typename ServiceType, typename ObjectType>
FORCEINLINE_DEBUGGABLE static ServiceType* GetServiceFromObject(const TWeakObjectPtr<ObjectType>& Object)
{
	return GetServiceFromObject(Object.Get());
}

///////////////////////////////////////////////////////////////////////////

namespace ServiceLocatorAccessors_Private
{

	extern UNREALSERVICELOCATOR_API const UObject* GetGameStateFromWorldContextObject(const UObject* WorldContextObject);
	extern UNREALSERVICELOCATOR_API const UObject* GetGameModeFromWorldContextObject(const UObject* WorldContextObject);

} // namespace ServiceLocatorAccessors_Private

///////////////////////////////////////////////////////////////////////////

template<typename ServiceType>
FORCEINLINE_DEBUGGABLE static ServiceType* GetServiceFromGameState(const UObject* WorldContextObject)
{
	return UServiceLocatorContainer::GetService<ServiceType>(ServiceLocatorAccessors_Private::GetGameStateFromWorldContextObject(WorldContextObject));
}

template<typename ServiceType>
FORCEINLINE_DEBUGGABLE static ServiceType* GetServiceFromGameMode(const UObject* WorldContextObject)
{
	return UServiceLocatorContainer::GetService<ServiceType>(ServiceLocatorAccessors_Private::GetGameModeFromWorldContextObject(WorldContextObject));
}

///////////////////////////////////////////////////////////////////////////

