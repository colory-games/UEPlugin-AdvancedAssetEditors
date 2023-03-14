// Fill out your copyright notice in the Description page of Project Settings.

#include "AdvancedStructureAssetsActions.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Engine/UserDefinedStruct.h"
#include "BlueprintEditorModule.h"

#include "AdvancedStructureAssetSubsystem.h"

#define LOCTEXT_NAMESPACE "AdvancedStructureAsset"


FAdvancedStructureAssetsActions::FAdvancedStructureAssetsActions()
{
}

void FAdvancedStructureAssetsActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
    EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

    for (auto It = InObjects.CreateConstIterator(); It; ++It)
    {
        auto ScriptStruct = Cast<UUserDefinedStruct>(*It);
        if (ScriptStruct != nullptr)
        {
            FBlueprintEditorModule* BlueprintEditorModule = FModuleManager::GetModulePtr<FBlueprintEditorModule>("Kismet");
            auto EditorToolkit = BlueprintEditorModule->CreateUserDefinedStructEditor(Mode, EditWithinLevelEditor, ScriptStruct);
        }
    }
}

bool FAdvancedStructureAssetsActions::HasActions(const TArray<UObject*>& InObjects) const
{
    return true;
}


void FAdvancedStructureAssetsActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
    FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);

    if (InObjects.Num() != 1)
    {
        return;
    }

    auto UserDefinedStruct = Cast<UUserDefinedStruct>(InObjects[0]);
    if (UserDefinedStruct == nullptr)
    {
        return;
    }

    MenuBuilder.AddMenuEntry(
        LOCTEXT("EditAdvanceConfiguration", "Edit Advance Configuration"),
        LOCTEXT("EditAdvanceConfigurationTip", "Edit the advance configuration of the structure asset."),
        FSlateIcon(),
        FUIAction(
            FExecuteAction::CreateLambda([=]
            {
                TArray<UObject*> Objects = {InObjects[0]};
                UAdvancedStructureAssetSubsystem* Subsystem = GEditor->GetEditorSubsystem<UAdvancedStructureAssetSubsystem>();
                Subsystem->OpenAdvancedEditor(Objects);
            }),
            FCanExecuteAction::CreateLambda([=]
            {
                return true;
            })
        )
    );
}

uint32 FAdvancedStructureAssetsActions::GetCategories()
{
    return EAssetTypeCategories::Blueprint;
}

FText FAdvancedStructureAssetsActions::GetName() const
{
    return LOCTEXT("Structure", "Structure");
}

UClass* FAdvancedStructureAssetsActions::GetSupportedClass() const
{
    return UUserDefinedStruct::StaticClass();
}

FColor FAdvancedStructureAssetsActions::GetTypeColor() const
{
    return FColor::Cyan;
}

#undef LOCTEXT_NAMESPACE