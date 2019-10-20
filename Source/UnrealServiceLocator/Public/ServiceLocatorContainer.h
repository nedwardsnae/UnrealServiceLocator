///////////////////////////////////////////////////////////////////////////
// ServiceLocatorContainer.h
///////////////////////////////////////////////////////////////////////////

#pragma once

// Engine
#include "UObject/Object.h"

// UnrealServiceLocator
#include "ServiceLocatorHelpers.h"
#include "ServiceLocatorContainer.generated.h"

// Forward Declarations
class AActor;
class UActorComponent;
class UServiceLocatorConfig;
enum class EServiceLocationBehaviour : uint8;
struct FServiceDescriptor;

///////////////////////////////////////////////////////////////////////////

UNREALSERVICELOCATOR_API DECLARE_LOG_CATEGORY_EXTERN(LogUnrealServiceLocator, Warning, All);

///////////////////////////////////////////////////////////////////////////

UCLASS(DefaultToInstanced)
class UNREALSERVICELOCATOR_API UServiceLocatorContainer : public UObject
{
	GENERATED_BODY()

public:

	//////////////////////////////////////////////
	// Functions

	/////////////////////
	// Static Functions

	/**
	 * Returns an instance of the service specified by the ServiceType template parameter
	 * @param	Object			The object to find the service locator container in
	 * @return	ServiceType*	The service instance
	 */
	template<typename ServiceType, typename ObjectType>
	FORCEINLINE static ServiceType* GetService(const ObjectType* Object);

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

protected:

	UObject* GetServiceInternal(const UClass* ServiceClass) const;

	UObject* LocateOrCreateService(const FServiceDescriptor& ServiceDescriptor);
	AActor* LocateOrCreateActorService(const FServiceDescriptor& ServiceDescriptor);
	UActorComponent* LocateOrCreateComponentService(const FServiceDescriptor& ServiceDescriptor);
	UObject* LocateOrCreateObjectService(const FServiceDescriptor& ServiceDescriptor);

	//////////////////////////////////////////////
	// Tweakables

	UPROPERTY(EditAnywhere)
	UServiceLocatorConfig* Config = nullptr;

	//////////////////////////////////////////////
	// Data

	UPROPERTY(Transient)
	TArray<UObject*> Services;

	UPROPERTY(Transient)
	TMap<UClass*, UObject*> MappedTypesToServices;

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
	if (Object == nullptr)
	{
		UE_LOG(LogUnrealServiceLocator, Warning, TEXT("UServiceLocatorContainer::GetService: A null object was passed!"));
		return nullptr;
	}

	const IServiceLocatorInterface* ObjectAsSLI = TGetObjectAsSLI<ObjectType>::Execute(Object);
	if (ObjectAsSLI == nullptr)
	{
		UE_LOG(LogUnrealServiceLocator, Warning, TEXT("UServiceLocatorContainer::GetService: Object '%s' does not implement IServiceLocatorInterface!"), *GetNameSafe(Cast<const UObject>(Object)));
		return nullptr;
	}

	UServiceLocatorContainer* Container = ObjectAsSLI->GetContainer();
	if (Container == nullptr)
	{
		UE_LOG(LogUnrealServiceLocator, Warning, TEXT("UServiceLocatorContainer::GetService: Object '%s' did not return a UServiceLocatorContainer!"), *GetNameSafe(Cast<const UObject>(Object)));
		return nullptr;
	}

	return Container->GetService<ServiceType>();
}

///////////////////////////////////////////////////////////////////////////
