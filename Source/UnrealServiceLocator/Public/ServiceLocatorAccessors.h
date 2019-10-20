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
FORCEINLINE_DEBUGGABLE static ServiceType* GetService(const ObjectType* Object)
{
	return UServiceLocatorContainer::GetService<ServiceType, ObjectType>(Object);
}

template<typename ServiceType, typename ObjectType>
FORCEINLINE_DEBUGGABLE static ServiceType* GetService(const TWeakObjectPtr<ObjectType>& Object)
{
	return GetService(Object.Get());
}

///////////////////////////////////////////////////////////////////////////

namespace ServiceLocatorAccessors_Private
{

	extern UNREALSERVICELOCATOR_API const IServiceLocatorInterface* GetGameStateService_GetGameStateSLIFromWorldContextObject(const UObject* WorldContextObject);
	extern UNREALSERVICELOCATOR_API const IServiceLocatorInterface* GetGameModeService_GetGameModeSLIFromWorldContextObject(const UObject* WorldContextObject);

} // namespace ServiceLocatorAccessors_Private

///////////////////////////////////////////////////////////////////////////

template<typename ServiceType>
FORCEINLINE_DEBUGGABLE static ServiceType* GetGameStateService(const UObject* WorldContextObject)
{
	const IServiceLocatorInterface* ServiceLocatorInterface = ServiceLocatorAccessors_Private::GetGameStateService_GetGameStateSLIFromWorldContextObject(WorldContextObject);
	if (ServiceLocatorInterface == nullptr)
		return nullptr;

	return UServiceLocatorContainer::GetService<ServiceType>(ServiceLocatorInterface);
}

template<typename ServiceType>
FORCEINLINE_DEBUGGABLE static ServiceType* GetGameModeService(const UObject* WorldContextObject)
{
	const IServiceLocatorInterface* ServiceLocatorInterface = ServiceLocatorAccessors_Private::GetGameModeService_GetGameModeSLIFromWorldContextObject(WorldContextObject);
	if (ServiceLocatorInterface == nullptr)
		return nullptr;

	return UServiceLocatorContainer::GetService<ServiceType>(ServiceLocatorInterface);
}

///////////////////////////////////////////////////////////////////////////

