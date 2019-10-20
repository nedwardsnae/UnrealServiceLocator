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

	const IServiceLocatorInterface* GetGameStateService_GetGameStateSLIFromWorldContextObject(const UObject* WorldContextObject)
	{
		if (WorldContextObject == nullptr)
		{
			UE_LOG(LogUnrealServiceLocator, Warning, TEXT("GetGameStateService: WorldContextObject is null!"));
			return nullptr;
		}

		UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
		if (World == nullptr)
		{
			UE_LOG(LogUnrealServiceLocator, Warning, TEXT("GetGameStateService: Could not obtain World from context object '%s'"), *GetNameSafe(WorldContextObject));
			return nullptr;
		}
		
		AGameStateBase* GameStateBase = World->GetGameState();
		if (GameStateBase == nullptr)
		{
			UE_LOG(LogUnrealServiceLocator, Warning, TEXT("GetGameStateService: Could not obtain Game State from World '%s'"), *GetNameSafe(World));
			return nullptr;
		}

		const IServiceLocatorInterface* GameStateAsSLI = Cast<const IServiceLocatorInterface>(GameStateBase);
		if (GameStateAsSLI == nullptr)
		{
			UE_LOG(LogUnrealServiceLocator, Warning, TEXT("GetGameStateService: GameStateBase '%s' does not implement IServiceLocatorInterface!"), *GetNameSafe(GameStateBase));
			return nullptr;
		}

		return GameStateAsSLI;
	}

	const IServiceLocatorInterface* GetGameModeService_GetGameModeSLIFromWorldContextObject(const UObject* WorldContextObject)
	{
		if (WorldContextObject == nullptr)
		{
			UE_LOG(LogUnrealServiceLocator, Warning, TEXT("GetGameModeService: WorldContextObject is null!"));
			return nullptr;
		}

		UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
		if (World == nullptr)
		{
			UE_LOG(LogUnrealServiceLocator, Warning, TEXT("GetGameModeService: Could not obtain World from context object '%s'"), *GetNameSafe(WorldContextObject));
			return nullptr;
		}
		
		AGameModeBase* GameModeBase = World->GetAuthGameMode();
		if (GameModeBase == nullptr)
		{
			UE_LOG(LogUnrealServiceLocator, Warning, TEXT("GetGameModeService: Could not obtain Game Mode from World '%s'"), *GetNameSafe(World));
			return nullptr;
		}

		const IServiceLocatorInterface* GameModeAsSLI = Cast<const IServiceLocatorInterface>(GameModeBase);
		if (GameModeAsSLI == nullptr)
		{
			UE_LOG(LogUnrealServiceLocator, Warning, TEXT("GetGameModeService: GameModeBase '%s' does not implement IServiceLocatorInterface!"), *GetNameSafe(GameModeBase));
			return nullptr;
		}

		return GameModeAsSLI;
	}

} // namespace ServiceLocatorAccessors_Private

///////////////////////////////////////////////////////////////////////////
