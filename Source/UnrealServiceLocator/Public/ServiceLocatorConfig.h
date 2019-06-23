///////////////////////////////////////////////////////////////////////////
// ServiceLocatorConfig.h
///////////////////////////////////////////////////////////////////////////

#pragma once

// Engine
#include "Engine/DataAsset.h"

// UnrealServiceLocator
#include "ServiceLocatorTypes.h"
#include "ServiceLocatorConfig.generated.h"

///////////////////////////////////////////////////////////////////////////

UCLASS()
class UNREALSERVICELOCATOR_API UServiceLocatorConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	//////////////////////////////////////////////

	UPROPERTY(EditAnywhere)
	TArray<FServiceDescriptor> ServiceDescriptors;

	//////////////////////////////////////////////

};

///////////////////////////////////////////////////////////////////////////