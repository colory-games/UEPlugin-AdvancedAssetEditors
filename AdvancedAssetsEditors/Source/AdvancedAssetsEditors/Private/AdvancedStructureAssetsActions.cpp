// Fill out your copyright notice in the Description page of Project Settings.

#include "AdvancedStructureAssetsActions.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Engine/UserDefinedStruct.h"

#include "../Public/AdvancedStrcutureAssetsEditorToolkit.h"

#define LOCTEXT_NAMESPACE "FAdvancedAssetsEditorsModule"


FAdvancedStructureAssetsActions::FAdvancedStructureAssetsActions(const TSharedRef<ISlateStyle>& InStyle) : Style(InStyle)
{
}

void FAdvancedStructureAssetsActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
    EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

    for (auto It = InObjects.CreateConstIterator(); It; ++It)
    {
        auto ScriptStruct = Cast<UScriptStruct>(*It);
        if (ScriptStruct != nullptr)
        {
            TSharedRef<FAdvancedStrcutureAssetsEditorToolkit> EditorToolkit = MakeShareable(new FAdvancedStrcutureAssetsEditorToolkit(Style));
            EditorToolkit->Initialize(ScriptStruct, Mode, EditWithinLevelEditor);
        }
    }
}

uint32 FAdvancedStructureAssetsActions::GetCategories()
{
    return EAssetTypeCategories::Misc;
}

FText FAdvancedStructureAssetsActions::GetName() const
{
    return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AdvancedStructure", "Advanced Structure");
}

UClass* FAdvancedStructureAssetsActions::GetSupportedClass() const
{
    return UUserDefinedStruct::StaticClass();
}

FColor FAdvancedStructureAssetsActions::GetTypeColor() const
{
    return FColor::White;
}

#undef LOCTEXT_NAMESPACE