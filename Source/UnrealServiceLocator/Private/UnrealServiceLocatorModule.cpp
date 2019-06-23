// UnrealServiceLocator.cpp

// Engine
#include "Modules/ModuleManager.h"

class FUnrealServiceLocatorModule : public IModuleInterface
{
public:

	void StartupModule() override final
	{

	}

	void ShutdownModule() override final
	{

	}

};

IMPLEMENT_MODULE(FUnrealServiceLocatorModule, UnrealServiceLocator)