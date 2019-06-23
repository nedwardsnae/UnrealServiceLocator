///////////////////////////////////////////////////////////////////////////
// ServiceLocatorInterface.h
///////////////////////////////////////////////////////////////////////////

#pragma once

// Engine
#include "UObject/Interface.h"

// UnrealServiceLocator
#include "ServiceLocatorInterface.generated.h"

// Forward Declarations
class UServiceLocatorContainer;

///////////////////////////////////////////////////////////////////////////

UINTERFACE(meta = (CannotImplementInterfaceInBlueprint))
class UNREALSERVICELOCATOR_API UServiceLocatorInterface : public UInterface
{
	GENERATED_BODY()
};

class UNREALSERVICELOCATOR_API IServiceLocatorInterface
{
	GENERATED_BODY()

public:

	virtual UServiceLocatorContainer* GetContainer() const = 0;

};

///////////////////////////////////////////////////////////////////////////
