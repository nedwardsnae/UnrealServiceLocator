// UnrealServiceLocator.cpp

// Engine
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "ServiceLocatorTypes.h"
#include "ServiceDescriptorCustomization.h"

class FUnrealServiceLocatorEditorModule : public IModuleInterface
{
public:

	void StartupModule() override final
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
		PropertyEditorModule.RegisterCustomPropertyTypeLayout(::StaticStruct<FServiceDescriptor>()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FServiceLocatorCustomization::MakeInstance));
	}

	void ShutdownModule() override final
	{

	}

};

IMPLEMENT_MODULE(FUnrealServiceLocatorEditorModule, UnrealServiceLocatorEditor)