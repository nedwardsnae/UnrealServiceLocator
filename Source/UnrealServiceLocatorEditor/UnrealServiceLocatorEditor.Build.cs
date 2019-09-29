using UnrealBuildTool;

public class UnrealServiceLocatorEditor : ModuleRules
{
	public UnrealServiceLocatorEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;

        PublicDependencyModuleNames.AddRange
		(
			new string[]
			{
                "UnrealServiceLocator",

                "Core",
				"CoreUObject",
				"Engine",
				"Slate",
                "SlateCore",
                "ApplicationCore",
				"InputCore",
				"PropertyEditor",
			}
		);
	}
}
