///////////////////////////////////////////////////////////////////////////
// ServiceLocatorContainer.h
///////////////////////////////////////////////////////////////////////////

#pragma once

// Engine
#include "UObject/Object.h"
#include "UObject/SoftObjectPtr.h"

// UnrealServiceLocator
#include "ServiceLocatorHelpers.h"
#include "ServiceLocatorContainer.generated.h"

// Forward Declarations
class AActor;
class UActorComponent;
class UServiceLocatorConfig;
enum class EServiceLocateBehaviour : uint8;

///////////////////////////////////////////////////////////////////////////

UCLASS(DefaultToInstanced)
class UNREALSERVICELOCATOR_API UServiceLocatorContainer : public UObject
{
	GENERATED_BODY()

public:

	//////////////////////////////////////////////
	// Functions

	/////////////////////
	// Member Functions

	/**
	 * According to the ServiceLocatorConfig, finds and/or creates services for retrieval
	 */
	void LocateAndCreateServices();

	/**
	 * Returns an instance of the service specified by the ServiceType template parameter
	 * @return	ServiceType*	The service instance
	 */
	template<typename ServiceType>
	ServiceType* GetService() const;

	/////////////////////
	// Static Functions

	/**
	 * Returns an instance of the service specified by the ServiceType template parameter
	 * @param	Object			The object to find the service locator container in
	 * @return	ServiceType*	The service instance
	 */
	template<typename ServiceType, typename ObjectType>
	static ServiceType* GetService(const ObjectType* Object);

	/**
	 * Returns an instance of the service specified by the ServiceType template parameter, with
	 * a PreCastTransform callable which can be used to find another object to find the service locator
	 * container in
	 * @param	Object				The object to pass to the PreCastTransform callable
	 * @param	PreCastTransform	The callable to use to transform the input object to another object where the service locator container can be found
	 * @return	ServiceType*
	 */
	template<typename ServiceType, typename ObjectType, typename PreCastTransformType>
	static ServiceType* GetService(const ObjectType* Object, PreCastTransformType&& PreCastTransform);

protected:

	UObject* GetServiceInternal(const UClass* ServiceClass) const;

	UObject* LocateOrCreateService(UClass& ServiceConcreteType, EServiceLocateBehaviour LocateBehaviour);
	AActor* LocateOrCreateActorService(UClass& ServiceConcreteType, EServiceLocateBehaviour LocateBehaviour);
	UActorComponent* LocateOrCreateComponentService(UClass& ServiceConcreteType, EServiceLocateBehaviour LocateBehaviour);
	UObject* LocateOrCreateObjectService(UClass& ServiceConcreteType, EServiceLocateBehaviour LocateBehaviour);

	//////////////////////////////////////////////
	// Tweakables

	UPROPERTY(EditAnywhere)
	UServiceLocatorConfig* Config = nullptr;

	//////////////////////////////////////////////
	// Data

	UPROPERTY(Transient)
	TMap<UClass*, UObject*> Services;

};

///////////////////////////////////////////////////////////////////////

template<typename ServiceType>
ServiceType* UServiceLocatorContainer::GetService() const
{
	UClass* ServiceTypeClass = TGetServiceClassType<ServiceType>::Execute();
	check(ServiceTypeClass != nullptr);

	return TGetServicePointer<ServiceType>::Execute(GetServiceInternal(ServiceTypeClass), ServiceTypeClass);
}

///////////////////////////////////////////////////////////////////////

template<typename ServiceType, typename ObjectType>
ServiceType* UServiceLocatorContainer::GetService(const ObjectType* Object)
{
	return GetService<ServiceType, ObjectType>(Object, FIdentityFunctor());
}

///////////////////////////////////////////////////////////////////////

template<typename ServiceType, typename ObjectType, typename PreCastTransformType>
ServiceType* UServiceLocatorContainer::GetService(const ObjectType* Object, PreCastTransformType&& PreCastTransform)
{
	const IServiceLocatorInterface* ObjectAsSLI = TGetObjectAsSLI<ObjectType, PreCastTransformType>::Execute(Object, Forward<PreCastTransformType>(PreCastTransform));
	if (ObjectAsSLI == nullptr)
	{
		return nullptr;
	}

	UServiceLocatorContainer* Container = ObjectAsSLI->GetContainer();
	if (Container == nullptr)
	{
		return nullptr;
	}

	return Container->GetService<ServiceType>();
}

///////////////////////////////////////////////////////////////////////////
