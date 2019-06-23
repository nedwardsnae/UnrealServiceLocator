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
enum class EServiceLocateBehaviour : uint8
{
	FindOnly,
	CreateIfNotFound
};

///////////////////////////////////////////////////////////////////////////

USTRUCT()
struct FServiceDescriptor
{
	GENERATED_BODY()

public:

	// The concrete type to locate
	UPROPERTY(EditAnywhere)
	TSubclassOf<UObject> ConcreteType;

	// (Optional) An interface to expose the service via
	UPROPERTY(EditAnywhere, meta = (AllowAbstract))
	TSubclassOf<UInterface> InterfaceType;

	UPROPERTY(EditAnywhere)
	EServiceLocateBehaviour LocateBehaviour = EServiceLocateBehaviour::CreateIfNotFound;

};

///////////////////////////////////////////////////////////////////////////
