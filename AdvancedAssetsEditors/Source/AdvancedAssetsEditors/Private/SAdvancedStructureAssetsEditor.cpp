#include "SAdvancedStructureAssetsEditor.h"

#include "Kismet2/StructureEditorUtils.h"
#include "UserDefinedStructure/UserDefinedStructEditorData.h"
#include "DetailLayoutBuilder.h"
#include "SPinTypeSelector.h"
#include "PropertyCustomizationHelpers.h"


#include "DetailCategoryBuilder.h"
#include "IDetailGroup.h"

#define LOCTEXT_NAMESPACE "SCustomStructAssetEditor"


SAdvancedStructureAssetsEditor::~SAdvancedStructureAssetsEditor()
{
    FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
}

void SAdvancedStructureAssetsEditor::Construct(
    const FArguments& InArgs, UScriptStruct* InCustomStructAsset,
    const TSharedRef<ISlateStyle>& InStyle)
{
    // Ref: FUserDefinedStructureLayout::GenerateChildContent (Source: /Engine/Source/Editor/Kismet/Private/UserDefinedStructureEditor.cpp)

    ScriptStruct = InCustomStructAsset;

    FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FOnGetDetailCustomizationInstance OnGetPropertiesViewCustomizationInstance = FOnGetDetailCustomizationInstance::CreateStatic(&FAdvancedStructurePropertiesDetailCustomization::MakeInstance);
    FDetailsViewArgs OnGetPropertiesViewCustomizationInstanceArgs(false, false, false, FDetailsViewArgs::HideNameArea, true, nullptr, false, FName(TEXT("UScriptStruct")));
    PropertiesView = EditModule.CreateDetailView(OnGetPropertiesViewCustomizationInstanceArgs);
    PropertiesView->RegisterInstancedCustomPropertyLayout(UObject::StaticClass(), OnGetPropertiesViewCustomizationInstance);
    PropertiesView->SetObject(ScriptStruct);

    FOnGetDetailCustomizationInstance OnGetDefaultsViewCustomizationInstance = FOnGetDetailCustomizationInstance::CreateStatic(&FAdvancedStructureDefaultsDetailCustomization::MakeInstance);
    FDetailsViewArgs OnGetDefaultsViewCustomizationInstanceArgs(false, false, false, FDetailsViewArgs::HideNameArea, true, nullptr, false, FName(TEXT("UScriptStruct")));
    DefaultsView = EditModule.CreateDetailView(OnGetDefaultsViewCustomizationInstanceArgs);
    DefaultsView->RegisterInstancedCustomPropertyLayout(UObject::StaticClass(), OnGetDefaultsViewCustomizationInstance);
    DefaultsView->SetObject(ScriptStruct);

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

enum EEditableMenu
{
    EditAnywhere,
    EditDefaultsOnly,
    EditInstanceOnly,
    VisibleAnywhere,
    VisibleDefaultsOnly,
    VisibleInstanceOnly,
    NoAccess,
    EEditableMenu_MAX
};

static TArray<FString> EditableMenuStrings = {
    "EditAnywhere",
    "EditDefaultsOnly",
    "EditInstanceOnly",
    "VisibleAnywhere",
    "VisibleDefaultsOnly",
    "VisibleInstanceOnly",
    "NoAccess",
};

void SetEditableMenu(FProperty* Property, EEditableMenu Editable)
{
    EPropertyFlags ClearFlags = EPropertyFlags::CPF_Edit | EPropertyFlags::CPF_EditConst |
                                EPropertyFlags::CPF_DisableEditOnInstance | EPropertyFlags::CPF_DisableEditOnTemplate;
    Property->ClearPropertyFlags(ClearFlags);

    EPropertyFlags NewFlags = CPF_None;
    switch (Editable)
    {
    case EEditableMenu::EditAnywhere:
        NewFlags = EPropertyFlags::CPF_Edit;
        break;
    case EEditableMenu::EditDefaultsOnly:
        NewFlags = EPropertyFlags::CPF_Edit | EPropertyFlags::CPF_DisableEditOnInstance;
        break;
    case EEditableMenu::EditInstanceOnly:
        NewFlags = EPropertyFlags::CPF_Edit | EPropertyFlags::CPF_DisableEditOnTemplate;
        break;
    case EEditableMenu::VisibleAnywhere:
        NewFlags = EPropertyFlags::CPF_Edit | EPropertyFlags::CPF_EditConst;
        break;
    case EEditableMenu::VisibleDefaultsOnly:
        NewFlags = EPropertyFlags::CPF_Edit | EPropertyFlags::CPF_EditConst | EPropertyFlags::CPF_DisableEditOnInstance;
        break;
    case EEditableMenu::VisibleInstanceOnly:
        NewFlags = EPropertyFlags::CPF_Edit | EPropertyFlags::CPF_EditConst | EPropertyFlags::CPF_DisableEditOnTemplate;
        break;
    }
    Property->SetPropertyFlags(NewFlags);
}

EEditableMenu GetEditableMenu(const FProperty* Property)
{
    EEditableMenu CurrentIndex = EEditableMenu::NoAccess;
    EPropertyFlags Flags = Property->GetPropertyFlags();

    if (Flags & EPropertyFlags::CPF_Edit)
    {
        if (Flags & EPropertyFlags::CPF_EditConst)
        {
            if (Flags & EPropertyFlags::CPF_DisableEditOnInstance)
            {
                CurrentIndex = EEditableMenu::VisibleDefaultsOnly;
            }
            else if (Flags & EPropertyFlags::CPF_DisableEditOnTemplate)
            {
                CurrentIndex = EEditableMenu::VisibleInstanceOnly;
            }
            else
            {
                CurrentIndex = EEditableMenu::VisibleAnywhere;
            }
        }
        else
        {
            if (Flags & EPropertyFlags::CPF_DisableEditOnInstance)
            {
                CurrentIndex = EEditableMenu::EditDefaultsOnly;
            }
            else if (Flags & EPropertyFlags::CPF_DisableEditOnTemplate)
            {
                CurrentIndex = EEditableMenu::EditInstanceOnly;
            }
            else
            {
                CurrentIndex = EEditableMenu::EditAnywhere;
            }
        }
    }

    return CurrentIndex;
}

FString GetEditableMenuString(const EEditableMenu EditableMenu)
{
    return EditableMenuStrings[EditableMenu];
}

void FAdvancedStructurePropertiesDetailCustomization::OnEditableChanged(TSharedPtr<uint8> Type, ESelectInfo::Type SelectionType, TWeakObjectPtr<UObject> Object, FProperty* Property)
{
    if (!Object.IsValid())
    {
        return;
    }

    SetEditableMenu(Property, (EEditableMenu)(*Type));
    Object->MarkPackageDirty();
}

TSharedRef<SWidget> FAdvancedStructurePropertiesDetailCustomization::OnEditableWidgetGenerated(TSharedPtr<uint8> Type)
{
    FString Name = GetEditableMenuString((EEditableMenu)(*Type));

    return SNew(STextBlock).Text(FText::AsCultureInvariant(Name));
}

FText FAdvancedStructurePropertiesDetailCustomization::GetEditableText(FProperty* Property) const
{
    EEditableMenu Menu = GetEditableMenu(Property);
    FString Name = GetEditableMenuString(Menu);

    return FText::AsCultureInvariant(Name);
}

void FAdvancedStructurePropertiesDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
    // ref: https://github.com/johnfredcee/UBrowse/blob/master/Source/UBrowse/Private/SUBrowser.cpp

    const IDetailsView* View = DetailLayout.GetDetailsView();
    IDetailCategoryBuilder& StructureCategory = DetailLayout.EditCategory("Structure", FText::GetEmpty(), ECategoryPriority::Important);

    const TArray<TWeakObjectPtr<UObject>> Objects = DetailLayout.GetDetailsView()->GetSelectedObjects();

    auto K2Schema = GetDefault<UEdGraphSchema_K2>();

    RegisteredEnumItemsList.Empty();

    for (auto Object : Objects)
    {
        if (!Object.IsValid()) {
            continue;
        }

        UScriptStruct* ScriptStruct = Cast<UScriptStruct>(Object);
        for (TFieldIterator<FProperty> It(ScriptStruct); It; ++It)
        {
            FProperty* Property = *It;
            uint64 Flags = Property->GetPropertyFlags();

            IDetailGroup& PropertyGroup = StructureCategory.AddGroup(FName(Property->GetAuthoredName()), FText::AsCultureInvariant(Property->GetAuthoredName()));
            PropertyGroup.HeaderRow()
                .NameContent()
                [
                    SNew(SEditableTextBox)
                    .Text(FText::AsCultureInvariant(Property->GetAuthoredName()))
                    .Font(IDetailLayoutBuilder::GetDetailFont())
                ]
                .ValueContent()
                [
                    SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        [
                            SNew(SPinTypeSelector, FGetPinTypeTree::CreateUObject(K2Schema, &UEdGraphSchema_K2::GetVariableTypeTree))
                            .Schema(K2Schema)
                            .TypeTreeFilter(ETypeTreeFilter::None)
                            .Font(IDetailLayoutBuilder::GetDetailFont())
                        ]
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SButton)
                            [
                                SNew(SImage)
                                .Image(FEditorStyle::GetBrush("BlueprintEditor.Details.ArgUpButton"))
                            ]
                        ]
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SButton)
                            [
                                SNew(SImage)
                                .Image(FEditorStyle::GetBrush("BlueprintEditor.Details.ArgDownButton"))
                            ]
                        ]
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                ];

            IDetailGroup& PropertyGeneralGroup = PropertyGroup.AddGroup(FName("General"), LOCTEXT("General", "General"));
            PropertyGeneralGroup.HeaderRow()
                .NameContent()
                [
                    SNew(STextBlock)
                    .Text(FText::AsCultureInvariant("Tooltip"))
                    .Font(IDetailLayoutBuilder::GetDetailFont())
                ]
                .ValueContent()
                [
                    SNew(SEditableTextBox)
                    .Text(FText::AsCultureInvariant(Property->GetMetaData(TEXT("ToolTip")).Replace(TEXT("\n"), TEXT(" "))))
                    .Font(IDetailLayoutBuilder::GetDetailFont())
                ];

            TSharedPtr<EnumItems> Items = MakeShareable(new EnumItems());
            for (int Index = 0; Index < EEditableMenu::EEditableMenu_MAX; ++Index)
            {
                Items->Add(MakeShareable(new uint8(Index)));
            };
            RegisteredEnumItemsList.Add(Items);

            EEditableMenu CurrentIndex = GetEditableMenu(Property);

            PropertyGeneralGroup.HeaderRow()
                .NameContent()
                [
                    SNew(STextBlock)
                    .Text(FText::AsCultureInvariant("Editable"))
                    .Font(IDetailLayoutBuilder::GetDetailFont())
                ]
                .ValueContent()
                [
                    SNew(SComboBox<TSharedPtr<uint8>>)
                    .OptionsSource(&(*Items))
                    .InitiallySelectedItem((*Items)[CurrentIndex])
                    .OnSelectionChanged(this, &FAdvancedStructurePropertiesDetailCustomization::OnEditableChanged, Object, Property)
                    .OnGenerateWidget(this, &FAdvancedStructurePropertiesDetailCustomization::OnEditableWidgetGenerated)
                    [
                        SNew(STextBlock)
                        .Text(this, &FAdvancedStructurePropertiesDetailCustomization::GetEditableText, Property)
                        .Font(IDetailLayoutBuilder::GetDetailFont())
                    ]
                ];

        }
    }
}

void FAdvancedStructureDefaultsDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
    const IDetailsView* View = DetailLayout.GetDetailsView();
    IDetailCategoryBuilder& StructureCategory = DetailLayout.EditCategory("Default Values", FText::GetEmpty(), ECategoryPriority::Important);

    const TArray<TWeakObjectPtr<UObject>> Objects = DetailLayout.GetDetailsView()->GetSelectedObjects();

    auto K2Schema = GetDefault<UEdGraphSchema_K2>();

    RegisteredEnumItemsList.Empty();

    for (auto Object : Objects)
    {
        if (!Object.IsValid()) {
            continue;
        }

        UScriptStruct* ScriptStruct = Cast<UScriptStruct>(Object);
        for (TFieldIterator<FProperty> It(ScriptStruct); It; ++It)
        {
            FProperty* Property = *It;

            IDetailGroup& PropertyGroup = StructureCategory.AddGroup(FName(Property->GetAuthoredName()), FText::AsCultureInvariant(Property->GetAuthoredName()));
            PropertyGroup.HeaderRow()
                .NameContent()
                [
                    SNew(STextBlock)
                    .Text(FText::AsCultureInvariant(Property->GetAuthoredName()))
                    .Font(IDetailLayoutBuilder::GetDetailFont())
                ]
                .ValueContent()
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    [
                        SNew(SPinTypeSelector, FGetPinTypeTree::CreateUObject(K2Schema, &UEdGraphSchema_K2::GetVariableTypeTree))
                        .Schema(K2Schema)
                        .TypeTreeFilter(ETypeTreeFilter::None)
                        .Font(IDetailLayoutBuilder::GetDetailFont())
                    ]
                ];
        }
    }
}

#undef LOCTEXT_NAMESPACE
