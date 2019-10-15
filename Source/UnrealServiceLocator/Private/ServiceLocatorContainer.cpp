///////////////////////////////////////////////////////////////////////////
// ServiceLocatorContainer.cpp
///////////////////////////////////////////////////////////////////////////

// UnrealServiceLocator
#include "ServiceLocatorContainer.h"
#include "ServiceLocatorConfig.h"

// Engine
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

///////////////////////////////////////////////////////////////////////////

DEFINE_LOG_CATEGORY_STATIC(LogServiceLocatorContainer, Warning, All);

///////////////////////////////////////////////////////////////////////////

void UServiceLocatorContainer::LocateAndCreateServices()
{
	if (Config == nullptr)
	{
		UE_LOG(LogServiceLocatorContainer, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateServices: Config is null on container '%s' with outer '%s'"),
			*GetNameSafe(this), *GetNameSafe(GetOuter()));
		return;
	}

	const TArray<FServiceDescriptor>& ServiceDescriptors = Config->ServiceDescriptors;

	for (TArray<FServiceDescriptor>::TConstIterator Iter = ServiceDescriptors.CreateConstIterator(); Iter; ++Iter)
	{
		const FServiceDescriptor& ServiceDescriptor = *Iter;

		UClass* ConcreteType = ServiceDescriptor.ConcreteType;
		if (ConcreteType == nullptr)
		{
			UE_LOG(LogServiceLocatorContainer, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateServices: ConcreteType is null for element '%d' in config '%s'"),
				Iter.GetIndex(), *GetNameSafe(Config));
			continue;
		}

		if (ConcreteType->HasAnyClassFlags(CLASS_Abstract | CLASS_Interface))
		{
			UE_LOG(LogServiceLocatorContainer, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateServices: ConcreteType has Abstract or Interface class flags for element '%d' in config '%s'"),
				Iter.GetIndex(), *GetNameSafe(Config));
			continue;
		}

		UObject* ConcreteServiceInstance = LocateOrCreateService(*ConcreteType, ServiceDescriptor.LocateBehaviour);
		if (ConcreteServiceInstance == nullptr)
		{
			continue;
		}

		Services.Emplace(ConcreteType, ConcreteServiceInstance);

		for (UClass* InterfaceType : ServiceDescriptor.InterfaceTypes)
		{
			if (InterfaceType == nullptr)
			{
				continue;
			}

			if (ConcreteType->HasAnyClassFlags(CLASS_Interface))
			{
				if (!ConcreteType->ImplementsInterface(InterfaceType))
				{
					UE_LOG(LogServiceLocatorContainer, Error, TEXT("UServiceLocatorContainer::LocateOrCreateServices: ConcreteType '%s' doesn't implement InterfaceType '%s'"),
						*GetNameSafe(ConcreteType), *GetNameSafe(InterfaceType));
					continue;
				}
			}
			else if (!ConcreteType->IsChildOf(InterfaceType))
			{
				UE_LOG(LogServiceLocatorContainer, Error, TEXT("UServiceLocatorContainer::LocateOrCreateServices: ConcreteType '%s' isn't a child of InterfaceType '%s'"),
					*GetNameSafe(ConcreteType), *GetNameSafe(InterfaceType));
				continue;
			}

			Services.Emplace(InterfaceType, ConcreteServiceInstance);
		}
	}
}

///////////////////////////////////////////////////////////////////////////

UObject* UServiceLocatorContainer::GetServiceInternal(const UClass* ServiceClass) const
{
	if (!IsValid(ServiceClass))
	{
		return nullptr;
	}

	UObject* const* FoundService = Services.Find(ServiceClass);
	if (FoundService != nullptr)
	{
		return *FoundService;
	}

	return nullptr;
}

///////////////////////////////////////////////////////////////////////////

UObject* UServiceLocatorContainer::LocateOrCreateService(UClass& ServiceConcreteType, EServiceLocateBehaviour LocateBehaviour)
{
	if (ServiceConcreteType.IsChildOf<AActor>())
	{
		return LocateOrCreateActorService(ServiceConcreteType, LocateBehaviour);
	}

	if (ServiceConcreteType.IsChildOf<UActorComponent>())
	{
		return LocateOrCreateComponentService(ServiceConcreteType, LocateBehaviour);
	}

	return LocateOrCreateObjectService(ServiceConcreteType, LocateBehaviour);
}

///////////////////////////////////////////////////////////////////////////

AActor* UServiceLocatorContainer::LocateOrCreateActorService(UClass& ServiceConcreteType, EServiceLocateBehaviour LocateBehaviour)
{
	UWorld* LocalWorld = GetWorld();
	if (LocalWorld == nullptr)
	{
		UE_LOG(LogServiceLocatorContainer, Error, TEXT("UServiceLocatorContainer::LocateOrCreateActorService: World is null for container '%s' with outer '%s'"),
			*GetNameSafe(this), *GetNameSafe(GetOuter()));
		return nullptr;
	}

	// Look for an instance of this service already in the world
	for (AActor* ServiceInstance : TActorRange<AActor>(LocalWorld, &ServiceConcreteType))
	{
		return ServiceInstance;
	}

	// We need to bail here if we can only find the service, not create it
	if (LocateBehaviour == EServiceLocateBehaviour::FindOnly)
	{
		UE_LOG(LogServiceLocatorContainer, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateActorService: Unable to find instance of actor service with type '%s'"),
			*ServiceConcreteType.GetName());
		return nullptr;
	}

	// Create a new instance of the service
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.ObjectFlags = RF_Transient;
	AActor* ServiceInstance = LocalWorld->SpawnActor<AActor>(&ServiceConcreteType, ActorSpawnParameters);
	if (ServiceInstance == nullptr)
	{
		UE_LOG(LogServiceLocatorContainer, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateActorService: Unable to spawn instance of actor service with type '%s'"),
			*ServiceConcreteType.GetName());
		return nullptr;
	}

	return ServiceInstance;
}

///////////////////////////////////////////////////////////////////////////

UActorComponent* UServiceLocatorContainer::LocateOrCreateComponentService(UClass& ServiceConcreteType, EServiceLocateBehaviour LocateBehaviour)
{
	// Traverse the outer chain of this container to look for an Actor in which to find/create the component service
	AActor* OuterAsActor = GetTypedOuter<AActor>();
	if (OuterAsActor == nullptr)
	{
		UE_LOG(LogServiceLocatorContainer, Error, TEXT("UServiceLocatorContainer::LocateOrCreateComponentService: Outer '%s' and outer chain of container '%s' doesn't have an object of type AActor to add component t!"),
			*GetNameSafe(GetOuter()), *GetNameSafe(this));
		return nullptr;
	}

	UActorComponent* ServiceInstance = nullptr;

	// Look for an instance of this service already in the actor
	ServiceInstance = OuterAsActor->FindComponentByClass(&ServiceConcreteType);
	if (ServiceInstance != nullptr)
	{
		return ServiceInstance;
	}

	// We need to bail here if we can only find the service, not create it
	if (LocateBehaviour == EServiceLocateBehaviour::FindOnly)
	{
		UE_LOG(LogServiceLocatorContainer, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateComponentService: Unable to find instance of component service with type '%s'"),
			*ServiceConcreteType.GetName());
		return nullptr;
	}

	ServiceInstance = NewObject<UActorComponent>(OuterAsActor, &ServiceConcreteType, NAME_None, RF_Transient);
	if (ServiceInstance == nullptr)
	{
		UE_LOG(LogServiceLocatorContainer, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateComponentService: Unable to create instance of component service with type '%s'"),
			*ServiceConcreteType.GetName());
		return nullptr;
	}

	ServiceInstance->CreationMethod = EComponentCreationMethod::Instance;
	ServiceInstance->RegisterComponent();

	return ServiceInstance;
}

///////////////////////////////////////////////////////////////////////////

UObject* UServiceLocatorContainer::LocateOrCreateObjectService(UClass& ServiceConcreteType, EServiceLocateBehaviour LocateBehaviour)
{
	UObject* ServiceInstance = static_cast<UObject*>(FindObjectWithOuter(GetOuter(), &ServiceConcreteType));
	if (ServiceInstance != nullptr)
	{
		return ServiceInstance;
	}

	// We need to bail here if we can only find the service, not create it
	if (LocateBehaviour == EServiceLocateBehaviour::FindOnly)
	{
		UE_LOG(LogServiceLocatorContainer, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateObjectService: Unable to find instance of object service with type '%s'"),
			*ServiceConcreteType.GetName());
		return nullptr;
	}

	ServiceInstance = NewObject<UObject>(GetOuter(), &ServiceConcreteType, NAME_None, RF_Transient);
	if (ServiceInstance == nullptr)
	{
		UE_LOG(LogServiceLocatorContainer, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateObjectService: Unable to create instance of object service with type '%s'"),
			*ServiceConcreteType.GetName());
		return nullptr;
	}

	return ServiceInstance;
}

///////////////////////////////////////////////////////////////////////////
