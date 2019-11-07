#pragma once

// Engine
#include "Templates/Casts.h"
#include "Templates/ChooseClass.h"
#include "Templates/IsPointer.h"
#include "Templates/UnrealTemplate.h"

// UnrealServiceLocator
#include "ServiceLocatorInterface.h"

///////////////////////////////////////////////////////////////////////

template <typename T, bool bIsAUObject_IMPL = TPointerIsConvertibleFromTo<T, const volatile UObject>::Value>
struct TIsUInterface
{
	enum { Value = false };
};

template <typename T>
struct TIsUInterface<T, true>
{
	template <typename U> static char(&Resolve(U*))[(U::StaticClassFlags & CLASS_Interface) ? 2 : 1];
	template <typename U> static char(&Resolve(...))[1];

	enum { Value = sizeof(Resolve<T>(0)) - 1 };
};

///////////////////////////////////////////////////////////////////////

template<typename ObjectType, bool bObjectIsSLIDerived = TPointerIsConvertibleFromTo<ObjectType, const volatile IServiceLocatorInterface>::Value>
struct TGetObjectAsSLI;

template<typename ObjectType>
struct TGetObjectAsSLI<ObjectType, true /* bObjectIsSLIDerived */>
{
	static const IServiceLocatorInterface* Execute(const IServiceLocatorInterface* Object)
	{
		return Object;
	}
};

template<typename ObjectType>
struct TGetObjectAsSLI<ObjectType, false /* bObjectIsSLIDerived */>
{
	static const IServiceLocatorInterface* Execute(const ObjectType* Object)
	{
		return Cast<const IServiceLocatorInterface>(Object);
	}
};

///////////////////////////////////////////////////////////////////////

template<typename ServiceType, bool bIsIInterface = TIsIInterface<ServiceType>::Value, bool bIsUInterface = TIsUInterface<ServiceType>::Value>
struct TGetServiceClassType;

template<typename ServiceType>
struct TGetServiceClassType<ServiceType, true /* bIsIInterface */, false /* bIsUInterface */>
{
	static UClass* Execute()
	{
		return ::StaticClass<typename ServiceType::UClassType>();
	}
};

template<typename ServiceType>
struct TGetServiceClassType<ServiceType, false /* bIsIInterface */, false /* bIsUInterface */>
{
	static UClass* Execute()
	{
		return ::StaticClass<ServiceType>();
	}
};

template<typename ServiceType>
struct TGetServiceClassType<ServiceType, false /* bIsIInterface */, true /* bIsUInterface */>
{
	static UClass* Execute()
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
