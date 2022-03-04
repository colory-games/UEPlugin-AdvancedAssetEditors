#include "SAdvancedStructureAssetsEditor.h"

#include "Kismet2/BlueprintEditorUtils.h"
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
    const FArguments& InArgs, UUserDefinedStruct* InCustomStructAsset,
    const TSharedRef<ISlateStyle>& InStyle)
{
    // Ref: FUserDefinedStructureLayout::GenerateChildContent (Source: /Engine/Source/Editor/Kismet/Private/UserDefinedStructureEditor.cpp)

    UserDefinedStruct = InCustomStructAsset;

    FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FOnGetDetailCustomizationInstance OnGetPropertiesViewCustomizationInstance = FOnGetDetailCustomizationInstance::CreateStatic(&FAdvancedStructurePropertiesDetailCustomization::MakeInstance);
    FDetailsViewArgs OnGetPropertiesViewCustomizationInstanceArgs(false, false, false, FDetailsViewArgs::HideNameArea, true, nullptr, false, FName(TEXT("UUserDefinedStruct")));
    PropertiesView = EditModule.CreateDetailView(OnGetPropertiesViewCustomizationInstanceArgs);
    PropertiesView->RegisterInstancedCustomPropertyLayout(UObject::StaticClass(), OnGetPropertiesViewCustomizationInstance);
    PropertiesView->SetObject(UserDefinedStruct);

    DefaultsNotification = MakeShareable(new FAdvancedStructureDefaultsNotification(UserDefinedStruct));
    DefaultsNotification->Initialize();

    FOnGetDetailCustomizationInstance OnGetDefaultsViewCustomizationInstance = FOnGetDetailCustomizationInstance::CreateStatic(&FAdvancedStructureDefaultsDetailCustomization::MakeInstance,
                                                                                                                               UserDefinedStruct, DefaultsNotification);
    FDetailsViewArgs OnGetDefaultsViewCustomizationInstanceArgs(false, false, false, FDetailsViewArgs::HideNameArea, true, nullptr, false, FName(TEXT("UUserDefinedStruct")));
    DefaultsView = EditModule.CreateDetailView(OnGetDefaultsViewCustomizationInstanceArgs);
    DefaultsView->RegisterInstancedCustomPropertyLayout(UObject::StaticClass(), OnGetDefaultsViewCustomizationInstance);
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

FEdGraphPinType FAdvancedStructurePropertiesDetailCustomization::OnGetPinInfo(TWeakObjectPtr<UObject> Object, FGuid Guid) const
{
    if (Object.IsValid())
    {
        auto UserDefinedStruct = Cast<UUserDefinedStruct>(Object);
        if (UserDefinedStruct != nullptr)
        {
            FStructVariableDescription* Desc = FStructureEditorUtils::GetVarDesc(UserDefinedStruct).FindByPredicate(FStructureEditorUtils::FFindByGuidHelper<FStructVariableDescription>(Guid));
            if (Desc != nullptr)
            {
                return Desc->ToPinType();
            }
        }
    }
    return FEdGraphPinType();
}

void FAdvancedStructurePropertiesDetailCustomization::OnPrePinInfoChanged(const FEdGraphPinType& PinType)
{
}

void FAdvancedStructurePropertiesDetailCustomization::OnPinInfoChanged(const FEdGraphPinType& PinType, TWeakObjectPtr<UObject> Object, FGuid Guid)
{
    if (Object.IsValid())
    {
        auto UserDefinedStruct = Cast<UUserDefinedStruct>(Object);
        if (UserDefinedStruct != nullptr)
        {
            FStructureEditorUtils::ChangeVariableType(UserDefinedStruct, Guid, PinType);
        }
    }
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

        UUserDefinedStruct* UserDefinedStruct = Cast<UUserDefinedStruct>(Object);
        for (TFieldIterator<FProperty> It(UserDefinedStruct); It; ++It)
        {
            FProperty* Property = *It;
            uint64 Flags = Property->GetPropertyFlags();
            FGuid Guid = FStructureEditorUtils::GetGuidForProperty(Property);

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
                            .TargetPinType(this, &FAdvancedStructurePropertiesDetailCustomization::OnGetPinInfo, Object, Guid)
                            .OnPinTypePreChanged(this, &FAdvancedStructurePropertiesDetailCustomization::OnPrePinInfoChanged)
                            .OnPinTypeChanged(this, &FAdvancedStructurePropertiesDetailCustomization::OnPinInfoChanged, Object, Guid)
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

void FAdvancedStructureDefaultsNotification::Initialize()
{
    StructData = MakeShareable(new FStructOnScope(UserDefinedStruct));
    UserDefinedStruct->InitializeDefaultValue(StructData->GetStructMemory());
    StructData->SetPackage(UserDefinedStruct->GetOutermost());
}

UUserDefinedStruct* FAdvancedStructureDefaultsNotification::GetUserDefinedStruct()
{
    return UserDefinedStruct;
}

TSharedPtr<class SWidget> FAdvancedStructureDefaultsNotification::GetView()
{
    return DetailsView;
}

void FAdvancedStructureDefaultsNotification::SetView(TSharedPtr<IDetailsView> View)
{
    DetailsView = View;
}

void FAdvancedStructureDefaultsNotification::PreChange(const UUserDefinedStruct* InUserDefinedStruct, FStructureEditorUtils::EStructureEditorChangeInfo Info)
{
    if (Info != FStructureEditorUtils::DefaultValueChanged)
    {
        StructData->Destroy();
        DetailsView->SetObject(nullptr);
        DetailsView->OnFinishedChangingProperties().Clear();
    }
}

void FAdvancedStructureDefaultsNotification::PostChange(const UUserDefinedStruct* InUserDefinedStruct, FStructureEditorUtils::EStructureEditorChangeInfo Info)
{
    if (Info != FStructureEditorUtils::DefaultValueChanged)
    {
        StructData->Initialize(UserDefinedStruct);
        DetailsView->SetObject(UserDefinedStruct, true);
    }
    UserDefinedStruct->InitializeDefaultValue(StructData->GetStructMemory());
}

void FAdvancedStructureDefaultsNotification::NotifyPreChange(FProperty* PropertyAboutToChange)
{
    ++PropertyChangeRecursionGuard;
}

void FAdvancedStructureDefaultsNotification::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged)
{
    --PropertyChangeRecursionGuard;
}

bool FAdvancedStructureDefaultsNotification::IsPropertyChangeComplete()
{
    return PropertyChangeRecursionGuard == 0;
}

TSharedPtr<FStructOnScope> FAdvancedStructureDefaultsNotification::GetStructData() const
{
    return StructData;
}

void FAdvancedStructureDefaultsDetailCustomization::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
    // ref: /Engine/Source/Editor/Kismet/Private/UserDefinedStructureEditor.cpp

    if (!DefaultsNotification.IsValid())
    {
        return;
    }

    TSharedPtr<FStructOnScope> StructData = DefaultsNotification->GetStructData();

    if (DefaultsNotification->IsPropertyChangeComplete())
    {
        UStruct* OwnerStruct = PropertyChangedEvent.MemberProperty->GetOwnerStruct();

        if (ensure(OwnerStruct == UserDefinedStruct))
        {
            const FProperty* DirectProperty = PropertyChangedEvent.MemberProperty;
            while (DirectProperty && !DirectProperty->GetOwner<const UUserDefinedStruct>())
            {
                DirectProperty = DirectProperty->GetOwner<const FProperty>();
            }
            ensure(nullptr != DirectProperty);

            if (DirectProperty)
            {
                FString DefaultValueString;
                bool bDefaultValueSet = false;
                if (StructData.IsValid() && StructData->IsValid())
                {
                    bDefaultValueSet = FBlueprintEditorUtils::PropertyValueToString(DirectProperty, StructData->GetStructMemory(), DefaultValueString, OwnerStruct);
                }

                const FGuid Guid = FStructureEditorUtils::GetGuidForProperty(DirectProperty);
                if (bDefaultValueSet && Guid.IsValid())
                {
                    FStructureEditorUtils::ChangeVariableDefaultValue(UserDefinedStruct, Guid, DefaultValueString);
                }
            }
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

        TSharedPtr<FStructOnScope> StructData = DefaultsNotification->GetStructData();

        View->OnFinishedChangingProperties().AddSP(this, &FAdvancedStructureDefaultsDetailCustomization::OnFinishedChangingProperties);

        for (TFieldIterator<FProperty> It(UserDefinedStruct); It; ++It)
        {
            FProperty* Property = *It;

            StructureCategory.AddExternalStructureProperty(StructData, (*It)->GetFName());
        }
    }
}

#undef LOCTEXT_NAMESPACE
