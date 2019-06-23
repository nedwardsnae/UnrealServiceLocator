///////////////////////////////////////////////////////////////////////////
// ServiceLocatorAccessors.h
///////////////////////////////////////////////////////////////////////////

#pragma once

// Engine
#include "CoreMinimal.h"
#include "Templates/Casts.h"
#include "Templates/ChooseClass.h"
#include "Templates/IsPointer.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/SoftObjectPtr.h"

// Forward Declarations
class IServiceLocatorInterface;

///////////////////////////////////////////////////////////////////////////

namespace ServiceLocatorAccessors_Private
{

	UNREALSERVICELOCATOR_API UObject* GetServiceFromObjectInternal(const UClass* ServiceType, const UObject& Object);
	UNREALSERVICELOCATOR_API UObject* GetServiceFromObjectInternal(const UClass* ServiceType, const IServiceLocatorInterface& Object);
	UNREALSERVICELOCATOR_API UObject* GetServiceInterfaceFromObjectInternal(const UClass* ServiceInterfaceType, const UObject& Object);
	UNREALSERVICELOCATOR_API UObject* GetServiceInterfaceFromObjectInternal(const UClass* ServiceInterfaceType, const IServiceLocatorInterface& Object);

	UNREALSERVICELOCATOR_API UObject* GetServiceFromGameStateInternal(const UClass* ServiceType, const UObject& Object);
	UNREALSERVICELOCATOR_API UObject* GetServiceInterfaceFromGameStateInternal(const UClass* ServiceInterfaceType, const UObject& Object);

	///////////////////////////////////////////////////////////////////////

	template<typename ObjectType>
	using TOverloadChooser =
		TChooseClass<
			TPointerIsConvertibleFromTo<ObjectType, const volatile IServiceLocatorInterface>::Value,
			IServiceLocatorInterface,
			UObject
		>;

	///////////////////////////////////////////////////////////////////////

	template<typename ServiceType, typename ObjectType, bool bIsInterface = TIsIInterface<ServiceType>::Value>
	struct TGetServiceFromObject;

	template<typename ServiceType, typename ObjectType>
	struct TGetServiceFromObject<ServiceType, ObjectType, true>
	{
		FORCEINLINE_DEBUGGABLE static ServiceType* Execute(const ObjectType* Object)
		{
			if (Object == nullptr)
				return nullptr;

			UClass* ServiceTypeClass = ::StaticClass<ServiceType::UClassType>();
			UObject* Service = GetServiceInterfaceFromObjectInternal(ServiceTypeClass, (const TOverloadChooser<ObjectType>::Result&)*Object);
			if (Service == nullptr)
				return nullptr;

			return (ServiceType*)Service->GetInterfaceAddress(ServiceTypeClass);
		}
	};

	template<typename ServiceType, typename ObjectType>
	struct TGetServiceFromObject<ServiceType, ObjectType, false>
	{
		FORCEINLINE_DEBUGGABLE static ServiceType* Execute(const ObjectType* Object)
		{
			if (Object == nullptr)
				return nullptr;

			UClass* ServiceTypeClass = ::StaticClass<ServiceType>();
			UObject* Service = GetServiceFromObjectInternal(ServiceTypeClass, (const TOverloadChooser<ObjectType>::Result&)*Object);
			if (Service == nullptr)
				return nullptr;

			return (ServiceType*)Service;
		}
	};	

	///////////////////////////////////////////////////////////////////////

	template<typename ServiceType, typename ObjectType, bool bIsInterface = TIsIInterface<ServiceType>::Value>
	struct TGetServiceFromGameState;

	template<typename ServiceType, typename ObjectType>
	struct TGetServiceFromGameState<ServiceType, ObjectType, true>
	{
		FORCEINLINE_DEBUGGABLE static ServiceType* Execute(const UObject* WorldContextObject)
		{
			if (WorldContextObject == nullptr)
				return nullptr;

			UClass* ServiceTypeClass = ::StaticClass<ServiceType::UClassType>();
			UObject* Service = GetServiceInterfaceFromGameStateInternal(ServiceTypeClass, *WorldContextObject);
			if (Service == nullptr)
				return nullptr;

			return (ServiceType*)Service->GetInterfaceAddress(ServiceTypeClass);
		}
	};
	
	template<typename ServiceType, typename ObjectType>
	struct TGetServiceFromGameState<ServiceType, ObjectType, false>
	{
		FORCEINLINE_DEBUGGABLE static ServiceType* Execute(const UObject* WorldContextObject)
		{
			if (WorldContextObject == nullptr)
				return nullptr;

			UClass* ServiceTypeClass = ::StaticClass<ServiceType>();
			UObject* Service = GetServiceFromGameStateInternal(ServiceTypeClass, *WorldContextObject);
			if (Service == nullptr)
				return nullptr;

			return (ServiceType*)Service;
		}
	};

} // ServiceLocatorAccessors_Private

///////////////////////////////////////////////////////////////////////////

template<typename ServiceType, typename ObjectType>
FORCEINLINE_DEBUGGABLE static ServiceType* GetServiceFromObject(const ObjectType* Object)
{
	return ServiceLocatorAccessors_Private::TGetServiceFromObject<ServiceType, ObjectType>::Execute(Object);
}

template<typename ServiceType, typename ObjectType>
FORCEINLINE_DEBUGGABLE static ServiceType* GetServiceFromObject(const TWeakObjectPtr<ObjectType>& Object)
{
	return GetServiceFromObject(Object.Get());
}

///////////////////////////////////////////////////////////////////////////

template<typename ServiceType>
FORCEINLINE_DEBUGGABLE static ServiceType* GetServiceFromGameState(const UObject* WorldContextObject)
{
	return ServiceLocatorAccessors_Private::TGetServiceFromGameState<ServiceType>::Execute(WorldContextObject);
}

///////////////////////////////////////////////////////////////////////////

