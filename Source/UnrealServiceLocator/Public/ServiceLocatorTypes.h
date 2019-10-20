///////////////////////////////////////////////////////////////////////////
// ServiceLocatorTypes.h
///////////////////////////////////////////////////////////////////////////

#pragma once

// Engine
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"

// UnrealServiceLocator
#include "ServiceLocatorTypes.generated.h"

// Forward Declarations
class UInterface;

///////////////////////////////////////////////////////////////////////////

UENUM()
enum class EServiceLocationBehaviour : uint8
{
	// [FindOnly] will attempt to find the service (and store it in the container if it is found), but will otherwise do nothing.
	// If the service is missing, any call to GetSservice() for this type will return a nullptr.
	FindOnly,

	// [CreateIfNotFound] will attempt to find the service, but if it fails to find it, it will create
	// a new instance, storing it in the container as a result, such that any future call to GetService() should succeed.
	// Created on both the Server and Client
	CreateIfNotFound,

	// [CreateIfNotFoundServerOnly] will do the same as [CreateIfNotFound], but will only create a new instance on the server
	CreateIfNotFoundServerOnly,

	// [CreateIfNotFoundClientOnly] will do the same as [CreateIfNotFound], but will only create a new instance on the client
	CreateIfNotFoundClientOnly
};

///////////////////////////////////////////////////////////////////////////

USTRUCT()
struct FServiceDescriptor
{
	GENERATED_BODY()

public:

	// The concrete type to locate
	UPROPERTY(EditAnywhere)
	UClass*						ServiceType		= nullptr;

	// (Optional) Types to map to this service (both concrete and abstract classes allowed)
	UPROPERTY(EditAnywhere, meta = (AllowAbstract))
	TArray<UClass*>				MappedTypes;

	// The behaviour used for locating this particular service
	UPROPERTY(EditAnywhere)
	EServiceLocationBehaviour	LocateBehaviour	= EServiceLocationBehaviour::CreateIfNotFound;

	// Whether the service will only be located in non-shipping builds
	UPROPERTY(EditAnywhere)
	bool						bDebugOnly		= false;

};

///////////////////////////////////////////////////////////////////////////
