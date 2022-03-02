// Fill out your copyright notice in the Description page of Project Settings.

#include "AdvancedStructureActions.h"

#include "AdvancedStructureToolkit.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "FAdvancedStructureActions"


FAdvancedStructureActions::FAdvancedStructureActions(const TSharedRef<ISlateStyle>& InStyle) : Style(InStyle)
{
}

void FAdvancedStructureActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
    EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

    for (auto It = InObjects.CreateConstIterator(); It; ++It)
    {
        auto ScriptStruct = Cast<UScriptStruct>(*It);
        if (ScriptStruct != nullptr)
        {
            TSharedRef<FAdvancedStructureEditorToolkit> EditorToolkit = MakeShareable(new FAdvancedStructureEditorToolkit(Style));
            EditorToolkit->Initialize(ScriptStruct, Mode, EditWithinLevelEditor);
        }
    }
}

uint32 FAdvancedStructureActions::GetCategories()
{
    return EAssetTypeCategories::Misc;
}

FText FAdvancedStructureActions::GetName() const
{
    return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AdvancedStructure", "Advanced Structure");
}

UClass* FAdvancedStructureActions::GetSupportedClass() const
{
    return UUserDefinedStruct::StaticClass();
}

FColor FAdvancedStructureActions::GetTypeColor() const
{
    return FColor::White;
}

FAdvancedStructureCustomization::~FAdvancedStructureCustomization()
{
}

void FAdvancedStructureCustomization::CustomizeDetails(IDetailLayoutBuilder& Layout)
{
    TArray<TWeakObjectPtr<UObject>> ObjectsToCustomize;
    Layout.GetObjectsBeingCustomized(ObjectsToCustomize);

    IDetailCategoryBuilder& Category = Layout.EditCategory("Advanced Structure");
    IDetailGroup& PropertyGroup = Category.AddGroup("Advanced Structure", FText::AsCultureInvariant("Advanced Structure"));
}

TSharedRef<IDetailCustomization> FAdvancedStructureCustomization::MakeInstance()
{
    return MakeShareable(new FAdvancedStructureCustomization);
}

#undef LOCTEXT_NAMESPACE