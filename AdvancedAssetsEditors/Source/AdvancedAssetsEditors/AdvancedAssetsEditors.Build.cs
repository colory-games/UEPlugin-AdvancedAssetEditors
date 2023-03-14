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
                "Kismet",
                "Projects",
                "Slate",
                "SlateCore",
                "ToolWidgets",
                "EditorStyle",
                "EditorSubsystem",
                "BlueprintGraph",
                "KismetWidgets",
                "PropertyEditor"
            }
        );
    }
}
