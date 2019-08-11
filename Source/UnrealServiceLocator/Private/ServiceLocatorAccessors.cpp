///////////////////////////////////////////////////////////////////////////
// ServiceLocatorAccessors.cpp
///////////////////////////////////////////////////////////////////////////

// UnrealServiceLocator
#include "ServiceLocatorAccessors.h"
#include "ServiceLocatorContainer.h"
#include "ServiceLocatorInterface.h"

// Engine
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"

///////////////////////////////////////////////////////////////////////////

UObject* ServiceLocatorAccessors_Private::GetGameStateFromWorldContextObject(const UObject* WorldContextObject)
{
	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World == nullptr)
		return nullptr;
	
	return World->GetGameState();
}

///////////////////////////////////////////////////////////////////////////
