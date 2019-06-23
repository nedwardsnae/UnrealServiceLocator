///////////////////////////////////////////////////////////////////////////
// ServiceLocatorContainer.h
///////////////////////////////////////////////////////////////////////////

#pragma once

// Engine
#include "UObject/Object.h"
#include "UObject/SoftObjectPtr.h"

// UnrealServiceLocator
#include "ServiceLocatorContainer.generated.h"

// Forward Declarations
class AActor;
class UActorComponent;
class UServiceLocatorConfig;
enum class EServiceLocateBehaviour : uint8;

///////////////////////////////////////////////////////////////////////////

UCLASS()
class UNREALSERVICELOCATOR_API UServiceLocatorContainer : public UObject
{
	GENERATED_BODY()

public:

	//////////////////////////////////////////////
	// Functions

	void LocateAndCreateServices();

	// "Internal" functions exposed for use by GetServiceFromObject in ServiceLocatorAccessors
	UObject* GetService(const UClass* ServiceClass) const;
	UObject* GetServiceInterface(const UClass* ServiceInterfaceClass) const;

private:

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

	UPROPERTY(Transient)
	TMap<UClass*, UObject*> ServiceInterfaces;

};

///////////////////////////////////////////////////////////////////////////
