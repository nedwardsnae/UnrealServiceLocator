///////////////////////////////////////////////////////////////////////////
// ServiceLocatorContainer.cpp
///////////////////////////////////////////////////////////////////////////

// UnrealServiceLocator
#include "ServiceLocatorContainer.h"
#include "ServiceLocatorConfig.h"

// Engine
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Stats/Stats2.h"

///////////////////////////////////////////////////////////////////////////
// Logging

DEFINE_LOG_CATEGORY(LogUnrealServiceLocator);

///////////////////////////////////////////////////////////////////////////
// Stats

DECLARE_STATS_GROUP(TEXT("UnrealServiceLocator"), STATGROUP_UnrealServiceLocator, STATCAT_Advanced);
DECLARE_DWORD_COUNTER_STAT(TEXT("UServiceLocatorContainer::GetServiceInternal Frame Calls"), STAT_UServiceLocatorContainer_GetServiceInternal_FrameCalls, STATGROUP_UnrealServiceLocator);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("UServiceLocatorContainer::GetServiceInternal Total Calls"), STAT_UServiceLocatorContainer_GetServiceInternal_TotalCalls, STATGROUP_UnrealServiceLocator);
DECLARE_CYCLE_STAT(TEXT("UServiceLocatorContainer::GetServiceInternal"), STAT_UServiceLocatorContainer_GetServiceInternal, STATGROUP_UnrealServiceLocator);
DECLARE_CYCLE_STAT(TEXT("UServiceLocatorContainer::LocateAndCreateServices"), STAT_UServiceLocatorContainer_LocateAndCreateServices, STATGROUP_UnrealServiceLocator);
DECLARE_CYCLE_STAT(TEXT("UServiceLocatorContainer::LocateOrCreateActorService"), STAT_UServiceLocatorContainer_LocateOrCreateActorService, STATGROUP_UnrealServiceLocator);
DECLARE_CYCLE_STAT(TEXT("UServiceLocatorContainer::LocateOrCreateComponentService"), STAT_UServiceLocatorContainer_LocateOrCreateComponentService, STATGROUP_UnrealServiceLocator);
DECLARE_CYCLE_STAT(TEXT("UServiceLocatorContainer::LocateOrCreateObjectService"), STAT_UServiceLocatorContainer_LocateOrCreateObjectService, STATGROUP_UnrealServiceLocator);

///////////////////////////////////////////////////////////////////////////

void UServiceLocatorContainer::LocateAndCreateServices()
{
	SCOPE_CYCLE_COUNTER(STAT_UServiceLocatorContainer_LocateAndCreateServices);

	if (Config == nullptr)
	{
		UE_LOG(LogUnrealServiceLocator, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateServices: Config is null on container '%s' with outer '%s'"),
			*GetNameSafe(this), *GetNameSafe(GetOuter()));
		return;
	}

	const TArray<FServiceDescriptor>& ServiceDescriptors = Config->ServiceDescriptors;

	for (TArray<FServiceDescriptor>::TConstIterator Iter = ServiceDescriptors.CreateConstIterator(); Iter; ++Iter)
	{
		const FServiceDescriptor& ServiceDescriptor = *Iter;

		UClass* ServiceType = ServiceDescriptor.ServiceType;
		if (ServiceType == nullptr)
		{
			UE_LOG(LogUnrealServiceLocator, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateServices: ServiceType is null for element '%d' in config '%s'"),
				Iter.GetIndex(), *GetNameSafe(Config));
			continue;
		}

		if (ServiceType->HasAnyClassFlags(CLASS_Abstract | CLASS_Interface))
		{
			UE_LOG(LogUnrealServiceLocator, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateServices: ServiceType has Abstract or Interface class flags for element '%d' in config '%s'"),
				Iter.GetIndex(), *GetNameSafe(Config));
			continue;
		}

		// If this is a debug only service and we're running a shipping build, skip this service
		if (ServiceDescriptor.bDebugOnly && UE_BUILD_SHIPPING)
		{
			continue;
		}

		UObject* ServiceInstance = LocateOrCreateService(ServiceDescriptor);
		if (ServiceInstance == nullptr)
		{
			continue;
		}

		Services.Emplace(ServiceInstance);

		for (UClass* MappedType : ServiceDescriptor.MappedTypes)
		{
			if (MappedType == nullptr)
			{
				continue;
			}

			if (MappedType->HasAnyClassFlags(CLASS_Interface))
			{
				if (!ServiceType->ImplementsInterface(MappedType))
				{
					UE_LOG(LogUnrealServiceLocator, Error, TEXT("UServiceLocatorContainer::LocateOrCreateServices: ServiceType '%s' doesn't implement MappedType '%s'"),
						*GetNameSafe(ServiceType), *GetNameSafe(MappedType));
					continue;
				}
			}
			else if (!ServiceType->IsChildOf(MappedType))
			{
				UE_LOG(LogUnrealServiceLocator, Error, TEXT("UServiceLocatorContainer::LocateOrCreateServices: ServiceType '%s' isn't a child of MappedType '%s'"),
					*GetNameSafe(ServiceType), *GetNameSafe(MappedType));
				continue;
			}

			UObject** ExistingMappedTypeService = MappedTypesToServices.Find(MappedType);
			if (ExistingMappedTypeService != nullptr)
			{
				UE_LOG(LogUnrealServiceLocator, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateServices: Type '%s' is already mapped to Service '%s', but will be displaced by ServiceType '%s'"),
					*GetNameSafe(MappedType), *GetNameSafe(*ExistingMappedTypeService), *GetNameSafe(ServiceType));
			}

			MappedTypesToServices.Emplace(MappedType, ServiceInstance);
		}
	}
}

///////////////////////////////////////////////////////////////////////////

UObject* UServiceLocatorContainer::GetServiceInternal(const UClass* ServiceClass) const
{
	INC_DWORD_STAT(STAT_UServiceLocatorContainer_GetServiceInternal_FrameCalls);
	INC_DWORD_STAT(STAT_UServiceLocatorContainer_GetServiceInternal_TotalCalls);
	SCOPE_CYCLE_COUNTER(STAT_UServiceLocatorContainer_GetServiceInternal);

	if (!IsValid(ServiceClass))
	{
		return nullptr;
	}

	UObject* const* FoundService = MappedTypesToServices.Find(ServiceClass);
	if (FoundService != nullptr)
	{
		return *FoundService;
	}

	return nullptr;
}

///////////////////////////////////////////////////////////////////////////

UObject* UServiceLocatorContainer::LocateOrCreateService(const FServiceDescriptor& ServiceDescriptor)
{
	if (ServiceDescriptor.ServiceType->IsChildOf<AActor>())
	{
		return LocateOrCreateActorService(ServiceDescriptor);
	}

	if (ServiceDescriptor.ServiceType->IsChildOf<UActorComponent>())
	{
		return LocateOrCreateComponentService(ServiceDescriptor);
	}

	return LocateOrCreateObjectService(ServiceDescriptor);
}

///////////////////////////////////////////////////////////////////////////

AActor* UServiceLocatorContainer::LocateOrCreateActorService(const FServiceDescriptor& ServiceDescriptor)
{
	SCOPE_CYCLE_COUNTER(STAT_UServiceLocatorContainer_LocateOrCreateActorService);

	UWorld* LocalWorld = GetWorld();
	if (LocalWorld == nullptr)
	{
		UE_LOG(LogUnrealServiceLocator, Error, TEXT("UServiceLocatorContainer::LocateOrCreateActorService: World is null for container '%s' with outer '%s'"),
			*GetNameSafe(this), *GetNameSafe(GetOuter()));
		return nullptr;
	}

	// Look for an instance of this service already in the world
	for (AActor* ServiceInstance : TActorRange<AActor>(LocalWorld, ServiceDescriptor.ServiceType))
	{
		return ServiceInstance;
	}

	// We need to bail here if we can only find the service, not create it
	if (ServiceDescriptor.LocateBehaviour == EServiceLocationBehaviour::FindOnly)
	{
		UE_LOG(LogUnrealServiceLocator, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateActorService: Unable to find instance of actor service with type '%s'"),
			*GetNameSafe(ServiceDescriptor.ServiceType));
		return nullptr;
	}

	// If we should only create this if we're the server, but we're not the server, bail
	if ((ServiceDescriptor.LocateBehaviour == EServiceLocationBehaviour::CreateIfNotFoundServerOnly) && !LocalWorld->IsServer())
	{
		UE_LOG(LogUnrealServiceLocator, Verbose, TEXT("UServiceLocatorContainer::LocateOrCreateActorService: Unable to find instance of actor service with type '%s', unable to create due to not being the server"),
			*GetNameSafe(ServiceDescriptor.ServiceType));
		return nullptr;
	}

	// If we should only create this if we're a client, but we're the server, bail
	if ((ServiceDescriptor.LocateBehaviour == EServiceLocationBehaviour::CreateIfNotFoundClientOnly) && LocalWorld->IsServer())
	{
		UE_LOG(LogUnrealServiceLocator, Verbose, TEXT("UServiceLocatorContainer::LocateOrCreateActorService: Unable to find instance of actor service with type '%s', unable to create due to being the server"),
			*GetNameSafe(ServiceDescriptor.ServiceType));
		return nullptr;
	}

	// Create a new instance of the service
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.ObjectFlags = RF_Transient;
	AActor* ServiceInstance = LocalWorld->SpawnActor<AActor>(ServiceDescriptor.ServiceType, ActorSpawnParameters);
	if (ServiceInstance == nullptr)
	{
		UE_LOG(LogUnrealServiceLocator, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateActorService: Unable to spawn instance of actor service with type '%s'"),
			*GetNameSafe(ServiceDescriptor.ServiceType));
		return nullptr;
	}

	return ServiceInstance;
}

///////////////////////////////////////////////////////////////////////////

UActorComponent* UServiceLocatorContainer::LocateOrCreateComponentService(const FServiceDescriptor& ServiceDescriptor)
{
	SCOPE_CYCLE_COUNTER(STAT_UServiceLocatorContainer_LocateOrCreateComponentService);

	// Traverse the outer chain of this container to look for an Actor in which to find/create the component service
	AActor* OuterAsActor = GetTypedOuter<AActor>();
	if (OuterAsActor == nullptr)
	{
		UE_LOG(LogUnrealServiceLocator, Error, TEXT("UServiceLocatorContainer::LocateOrCreateComponentService: Outer '%s' and outer chain of container '%s' doesn't have an object of type AActor to add component to!"),
			*GetNameSafe(GetOuter()), *GetNameSafe(this));
		return nullptr;
	}

	UActorComponent* ServiceInstance = nullptr;

	// Look for an instance of this service already in the actor
	ServiceInstance = OuterAsActor->FindComponentByClass(ServiceDescriptor.ServiceType);
	if (ServiceInstance != nullptr)
	{
		return ServiceInstance;
	}

	// We need to bail here if we can only find the service, not create it
	if (ServiceDescriptor.LocateBehaviour == EServiceLocationBehaviour::FindOnly)
	{
		UE_LOG(LogUnrealServiceLocator, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateComponentService: Unable to find instance of component service with type '%s'"),
			*GetNameSafe(ServiceDescriptor.ServiceType));
		return nullptr;
	}

	// If we should only create this if we're the server, but we're not the server, bail
	if ((ServiceDescriptor.LocateBehaviour == EServiceLocationBehaviour::CreateIfNotFoundServerOnly) && ((OuterAsActor->GetWorld() == nullptr) || !OuterAsActor->GetWorld()->IsServer()))
	{
		UE_LOG(LogUnrealServiceLocator, Verbose, TEXT("UServiceLocatorContainer::LocateOrCreateActorService: Unable to find instance of component service with type '%s', unable to create due to not being the server"),
			*GetNameSafe(ServiceDescriptor.ServiceType));
		return nullptr;
	}

	// If we should only create this if we're a client, but we're the server, bail
	if ((ServiceDescriptor.LocateBehaviour == EServiceLocationBehaviour::CreateIfNotFoundClientOnly) && ((OuterAsActor->GetWorld() == nullptr) || OuterAsActor->GetWorld()->IsServer()))
	{
		UE_LOG(LogUnrealServiceLocator, Verbose, TEXT("UServiceLocatorContainer::LocateOrCreateActorService: Unable to find instance of component service with type '%s', unable to create due to being the server"),
			*GetNameSafe(ServiceDescriptor.ServiceType));
		return nullptr;
	}

	ServiceInstance = NewObject<UActorComponent>(OuterAsActor, ServiceDescriptor.ServiceType, NAME_None, RF_Transient);
	if (ServiceInstance == nullptr)
	{
		UE_LOG(LogUnrealServiceLocator, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateComponentService: Unable to create instance of component service with type '%s'"),
			*GetNameSafe(ServiceDescriptor.ServiceType));
		return nullptr;
	}

	ServiceInstance->CreationMethod = EComponentCreationMethod::Instance;
	ServiceInstance->RegisterComponent();

	return ServiceInstance;
}

///////////////////////////////////////////////////////////////////////////

UObject* UServiceLocatorContainer::LocateOrCreateObjectService(const FServiceDescriptor& ServiceDescriptor)
{
	SCOPE_CYCLE_COUNTER(STAT_UServiceLocatorContainer_LocateOrCreateObjectService);

	UObject* ServiceInstance = static_cast<UObject*>(FindObjectWithOuter(GetOuter(), ServiceDescriptor.ServiceType));
	if (ServiceInstance != nullptr)
	{
		return ServiceInstance;
	}

	// We need to bail here if we can only find the service, not create it
	if (ServiceDescriptor.LocateBehaviour == EServiceLocationBehaviour::FindOnly)
	{
		UE_LOG(LogUnrealServiceLocator, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateObjectService: Unable to find instance of object service with type '%s'"),
			*GetNameSafe(ServiceDescriptor.ServiceType));
		return nullptr;
	}

	// If we should only create this if we're the server, but we're not the server, bail
	if ((ServiceDescriptor.LocateBehaviour == EServiceLocationBehaviour::CreateIfNotFoundServerOnly) && ((GetWorld() == nullptr) || !GetWorld()->IsServer()))
	{
		UE_LOG(LogUnrealServiceLocator, Verbose, TEXT("UServiceLocatorContainer::LocateOrCreateActorService: Unable to find instance of object service with type '%s', unable to create due to not being the server"),
			*GetNameSafe(ServiceDescriptor.ServiceType));
		return nullptr;
	}

	// If we should only create this if we're a client, but we're the server, bail
	if ((ServiceDescriptor.LocateBehaviour == EServiceLocationBehaviour::CreateIfNotFoundClientOnly) && ((GetWorld() == nullptr) || GetWorld()->IsServer()))
	{
		UE_LOG(LogUnrealServiceLocator, Verbose, TEXT("UServiceLocatorContainer::LocateOrCreateActorService: Unable to find instance of object service with type '%s', unable to create due to being the server"),
			*GetNameSafe(ServiceDescriptor.ServiceType));
		return nullptr;
	}

	ServiceInstance = NewObject<UObject>(GetOuter(), ServiceDescriptor.ServiceType, NAME_None, RF_Transient);
	if (ServiceInstance == nullptr)
	{
		UE_LOG(LogUnrealServiceLocator, Warning, TEXT("UServiceLocatorContainer::LocateOrCreateObjectService: Unable to create instance of object service with type '%s'"),
			*GetNameSafe(ServiceDescriptor.ServiceType));
		return nullptr;
	}

	return ServiceInstance;
}

///////////////////////////////////////////////////////////////////////////
