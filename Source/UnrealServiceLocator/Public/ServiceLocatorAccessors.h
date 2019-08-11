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

// UnrealServiceLocator
#include "ServiceLocatorContainer.h"
#include "ServiceLocatorInterface.h"

// Forward Declarations
class IServiceLocatorInterface;

///////////////////////////////////////////////////////////////////////////

namespace ServiceLocatorAccessors_Private
{

	///////////////////////////////////////////////////////////////////////

	template <typename T, bool bIsAUObject_IMPL = TPointerIsConvertibleFromTo<T, const volatile UObject>::Value>
	struct TIsUInterface
	{
		enum { Value = false };
	};

	template <typename T>
	struct TIsUInterface<T, true>
	{
		template <typename U> static char(&Resolve(typename U*))[(U::StaticClassFlags & CLASS_Interface) ? 2 : 1];
		template <typename U> static char(&Resolve(...))[1];

		enum { Value = sizeof(Resolve<T>(0)) - 1 };
	};

	///////////////////////////////////////////////////////////////////////

	template<typename ObjectType, typename PreCastTransformType, bool bObjectIsSLIDerived = TPointerIsConvertibleFromTo<ObjectType, const volatile IServiceLocatorInterface>::Value>
	struct TGetObjectAsSLI;

	template<typename ObjectType, typename PreCastTransformType>
	struct TGetObjectAsSLI<ObjectType, PreCastTransformType, true /* bObjectIsSLIDerived */>
	{
		static const IServiceLocatorInterface* Execute(const ObjectType* Object, PreCastTransformType&& PreCastTransform)
		{
			return Object;
		}
	};

	template<typename ObjectType, typename PreCastTransformType>
	struct TGetObjectAsSLI<ObjectType, PreCastTransformType, false /* bObjectIsSLIDerived */>
	{
		static const IServiceLocatorInterface* Execute(const ObjectType* Object, PreCastTransformType&& PreCastTransform)
		{
			return Cast<const IServiceLocatorInterface>(Invoke(PreCastTransform, Object));
		}
	};

	///////////////////////////////////////////////////////////////////////

	template<typename ServiceType, bool bIsIInterface = TIsIInterface<ServiceType>::Value, bool bIsUInterface = TIsUInterface<ServiceType>::Value>
	struct TGetServiceTypeClass;

	template<typename ServiceType>
	struct TGetServiceTypeClass<ServiceType, true /* bIsIInterface */, false /* bIsUInterface */>
	{
		static UClass* Execute()
		{
			return ::StaticClass<ServiceType::UClassType>();
		}
	};

	template<typename ServiceType>
	struct TGetServiceTypeClass<ServiceType, false /* bIsIInterface */, false /* bIsUInterface */>
	{
		static UClass* Execute()
		{
			return ::StaticClass<ServiceType>();
		}
	};

	template<typename ServiceType>
	struct TGetServiceTypeClass<ServiceType, false /* bIsIInterface */, true /* bIsUInterface */>
	{
		static UClass Execute()
		{
			static_assert(TIsSame<ServiceType, ServiceType*>::Value, "Please use the I-prefix interface type instead of the U-prefix type!");
			return nullptr;
		}
	};

	///////////////////////////////////////////////////////////////////////

	template<typename ServiceType, bool bIsIInterface = TIsIInterface<ServiceType>::Value, bool bIsUInterface = TIsUInterface<ServiceType>::Value>
	struct TGetServicePointer;

	template<typename ServiceType>
	struct TGetServicePointer<ServiceType, true /* bIsIInterface */, false /* bIsUInterface */>
	{
		static ServiceType* Execute(UObject* ServiceObject, UClass* ServiceTypeClass)
		{
			return (ServiceObject != nullptr) ? (ServiceType*)ServiceObject->GetInterfaceAddress(ServiceTypeClass) : nullptr;
		}
	};

	template<typename ServiceType>
	struct TGetServicePointer<ServiceType, false /* bIsIInterface */, false /* bIsUInterface */>
	{
		static ServiceType* Execute(UObject* ServiceObject, UClass* ServiceTypeClass)
		{
			return (ServiceType*)ServiceObject;
		}
	};

	template<typename ServiceType>
	struct TGetServicePointer<ServiceType, false /* bIsIInterface */, true /* bIsUInterface */>
	{
		static ServiceType* Execute(UObject* ServiceObject, UClass* ServiceTypeClass)
		{
			static_assert(TIsSame<ServiceType, ServiceType*>::Value, "Please use the I-prefix interface type instead of the U-prefix type!");
			return nullptr;
		}
	};

	///////////////////////////////////////////////////////////////////////

	template<typename ServiceType, typename ObjectType, typename PreCastTransformType = decltype(FIdentityFunctor())>
	static ServiceType* GetServiceFromObjectInternal(const ObjectType* Object, PreCastTransformType&& PreCastTransform = FIdentityFunctor())
	{
		const IServiceLocatorInterface* ObjectAsSLI = TGetObjectAsSLI<ObjectType, PreCastTransformType>::Execute(Object, MoveTempIfPossible(PreCastTransform));
		if (ObjectAsSLI == nullptr)
		{
			return nullptr;
		}

		UServiceLocatorContainer* SLC = ObjectAsSLI->GetContainer();
		if (SLC == nullptr)
		{
			return nullptr;
		}

		UClass* ServiceTypeClass = TGetServiceTypeClass<ServiceType>::Execute();
		check(ServiceTypeClass != nullptr);

		return TGetServicePointer<ServiceType>::Execute(SLC->GetService(ServiceTypeClass), ServiceTypeClass);
	}

	///////////////////////////////////////////////////////////////////////

	extern UNREALSERVICELOCATOR_API UObject* GetGameStateFromWorldContextObject(const UObject* WorldContextObject);

	///////////////////////////////////////////////////////////////////////

} // ServiceLocatorAccessors_Private

///////////////////////////////////////////////////////////////////////////

template<typename ServiceType, typename ObjectType>
FORCEINLINE_DEBUGGABLE static ServiceType* GetServiceFromObject(const ObjectType* Object)
{
	return ServiceLocatorAccessors_Private::GetServiceFromObjectInternal<ServiceType, ObjectType>(Object);
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
	return ServiceLocatorAccessors_Private::GetServiceFromObjectInternal<ServiceType>(WorldContextObject, &ServiceLocatorAccessors_Private::GetGameStateFromWorldContextObject);
}

///////////////////////////////////////////////////////////////////////////

