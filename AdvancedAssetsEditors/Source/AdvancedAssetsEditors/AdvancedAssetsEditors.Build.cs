// Some copyright should be here...

using UnrealBuildTool;

public class AdvancedAssetsEditors : ModuleRules
{
    public AdvancedAssetsEditors(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "UnrealEd",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Projects",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "BlueprintGraph",
                "KismetWidgets",
            }
        );

        PrivateIncludePathModuleNames.AddRange(
            new string[]
            {
                //"AssetTools",
            }
        );
    }
}
