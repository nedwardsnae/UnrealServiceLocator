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

namespace ServiceLocatorAccessors_Private
{

	const UObject* GetGameStateFromWorldContextObject(const UObject* WorldContextObject)
	{
		UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
		if (World == nullptr)
			return nullptr;
		
		return World->GetGameState();
	}

	const UObject* GetGameModeFromWorldContextObject(const UObject* WorldContextObject)
	{
		UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
		if (World == nullptr)
			return nullptr;

		return World->GetAuthGameMode();
	}

} // namespace ServiceLocatorAccessors_Private

///////////////////////////////////////////////////////////////////////////
