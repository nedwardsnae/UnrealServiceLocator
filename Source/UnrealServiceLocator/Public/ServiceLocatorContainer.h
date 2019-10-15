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
	FORCEINLINE ServiceType* GetService() const;

	/////////////////////
	// Static Functions

	/**
	 * Returns an instance of the service specified by the ServiceType template parameter
	 * @param	Object			The object to find the service locator container in
	 * @return	ServiceType*	The service instance
	 */
	template<typename ServiceType, typename ObjectType>
	FORCEINLINE static ServiceType* GetService(const ObjectType* Object);

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
FORCEINLINE ServiceType* UServiceLocatorContainer::GetService() const
{
	UClass* ServiceTypeClass = TGetServiceClassType<ServiceType>::Execute();
	check(ServiceTypeClass != nullptr);

	return TGetServicePointer<ServiceType>::Execute(GetServiceInternal(ServiceTypeClass), ServiceTypeClass);
}

///////////////////////////////////////////////////////////////////////

template<typename ServiceType, typename ObjectType>
FORCEINLINE ServiceType* UServiceLocatorContainer::GetService(const ObjectType* Object)
{
	const IServiceLocatorInterface* ObjectAsSLI = TGetObjectAsSLI<ObjectType>::Execute(Object);
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
