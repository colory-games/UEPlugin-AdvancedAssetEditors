#include "SAdvancedStructureAssetsEditor.h"

#include "AdvancedStructurePropertiesView.h"

#define LOCTEXT_NAMESPACE "AdvancedAssetEditors"


SAdvancedStructureAssetsEditor::~SAdvancedStructureAssetsEditor()
{
    FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
}

void SAdvancedStructureAssetsEditor::Construct(
    const FArguments& InArgs, UUserDefinedStruct* InCustomStructAsset)
{
    // Ref: FUserDefinedStructureLayout::GenerateChildContent (Source: /Engine/Source/Editor/Kismet/Private/UserDefinedStructureEditor.cpp)

    UserDefinedStruct = InCustomStructAsset;

    DefaultsNotification = MakeShareable(new FAdvancedStructureDefaultsNotification(UserDefinedStruct));
    DefaultsNotification->Initialize();

    FPropertyEditorModule& PropertyEditorModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

    // Property View
    FDetailsViewArgs PropertyViewArgs;
    PropertyViewArgs.bAllowSearch = false;
    PropertyViewArgs.bHideSelectionTip = true;
    PropertyViewArgs.bShowOptions = false;
    PropertyViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    PropertiesView = PropertyEditorModule.CreateDetailView(PropertyViewArgs);
    PropertiesView->RegisterInstancedCustomPropertyLayout(UObject::StaticClass(),
        FOnGetDetailCustomizationInstance::CreateStatic(&FAdvancedStructurePropertiesDetailCustomization::MakeInstance, DefaultsNotification));
    PropertiesView->SetObject(UserDefinedStruct);

    // Defaults View
    FDetailsViewArgs DefaultsViewArgs;
    DefaultsViewArgs.bAllowSearch = false;
    DefaultsViewArgs.bHideSelectionTip = true;
    DefaultsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    DefaultsView = PropertyEditorModule.CreateDetailView(DefaultsViewArgs);
    DefaultsView->RegisterInstancedCustomPropertyLayout(UObject::StaticClass(),
        FOnGetDetailCustomizationInstance::CreateStatic(&FAdvancedStructureDefaultsDetailCustomization::MakeInstance, UserDefinedStruct, DefaultsNotification));
    DefaultsView->SetObject(UserDefinedStruct);
    DefaultsNotification->SetView(DefaultsView);

    ChildSlot
    [
        SNew(SSplitter)
        .Orientation(EOrientation::Orient_Vertical)
        + SSplitter::Slot()
        [
            SNew(SBorder)
            [
                PropertiesView.ToSharedRef()
            ]
        ]
        + SSplitter::Slot()
        [
            SNew(SBorder)
            [
                DefaultsView.ToSharedRef()
            ]
        ]
    ];
}

#undef LOCTEXT_NAMESPACE
