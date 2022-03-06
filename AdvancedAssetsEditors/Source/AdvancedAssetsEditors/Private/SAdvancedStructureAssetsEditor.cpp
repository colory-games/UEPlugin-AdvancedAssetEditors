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

FText FAdvancedStructurePropertyLayout::GetTooltipText() const
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
        auto Desc = FStructureEditorUtils::GetVarDesc(UserDefinedStruct);
        auto PropertyDesc = Desc.FindByPredicate(FStructureEditorUtils::FFindByGuidHelper<FStructVariableDescription>(Guid));
        return FText::FromString(PropertyDesc->ToolTip);
    }
    return FText();
}

void FAdvancedStructurePropertyLayout::OnTooltipTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit)
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
        FStructureEditorUtils::ChangeVariableTooltip(UserDefinedStruct, Guid, NewText.ToString());
    }
}

FText FAdvancedStructurePropertyLayout::OnGetNameText() const
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
        return FText::FromString(FStructureEditorUtils::GetVariableFriendlyName(UserDefinedStruct, Guid));
    }
    return FText::GetEmpty();
}

void FAdvancedStructurePropertyLayout::OnNameTextCommitted(const FText& NewText, ETextCommit::Type InText)
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
        FStructureEditorUtils::RenameVariable(UserDefinedStruct, Guid, NewText.ToString());
    }
}

FEdGraphPinType FAdvancedStructurePropertyLayout::OnGetPinInfo() const
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
        FStructVariableDescription* Desc = FStructureEditorUtils::GetVarDesc(UserDefinedStruct).FindByPredicate(FStructureEditorUtils::FFindByGuidHelper<FStructVariableDescription>(Guid));
        if (Desc != nullptr)
        {
            return Desc->ToPinType();
        }
    }
    return FEdGraphPinType();
}

void FAdvancedStructurePropertyLayout::OnPrePinInfoChanged(const FEdGraphPinType& PinType)
{
}

void FAdvancedStructurePropertyLayout::OnPinInfoChanged(const FEdGraphPinType& PinType)
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
        FStructureEditorUtils::ChangeVariableType(UserDefinedStruct, Guid, PinType);
    }
}

FReply FAdvancedStructurePropertyLayout::OnMoveUp()
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        if (!(PositionFlags & EPropertyPositionFlag::First))
        {
            auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
            FStructureEditorUtils::MoveVariable(UserDefinedStruct, Guid, FStructureEditorUtils::MD_Up);
        }
    }
    return FReply::Handled();
}

FReply FAdvancedStructurePropertyLayout::OnMoveDown()
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        if (!(PositionFlags & EPropertyPositionFlag::Last))
        {
            auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
            FStructureEditorUtils::MoveVariable(UserDefinedStruct, Guid, FStructureEditorUtils::MD_Down);
        }
    }
    return FReply::Handled();
}

void FAdvancedStructurePropertyLayout::OnRemoveProperty()
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
        FStructureEditorUtils::RemoveVariable(UserDefinedStruct, Guid);
    }
}

bool FAdvancedStructurePropertyLayout::IsRemoveButtonEnabled()
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
        return FStructureEditorUtils::GetVarDesc(UserDefinedStruct).Num() > 1;
    }
    return false;
}


#include "KismetCompilerModule.h"
#include "Kismet2/CompilerResultsLog.h"



void FAdvancedStructurePropertyLayout::OnEditableChanged(TSharedPtr<uint8> Type, ESelectInfo::Type SelectionType)
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        const FScopedTransaction Transaction(LOCTEXT("ChangeVariableOnBPInstance", "Change variable editable on BP instance"));

        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();

        FStructureEditorUtils::ModifyStructData(UserDefinedStruct);


        FProperty* Property = FStructureEditorUtils::GetPropertyByGuid(UserDefinedStruct, Guid);
        SetEditableMenu(Property, (EEditableMenu)(*Type));

        return;

        for (TFieldIterator<FProperty> It(UserDefinedStruct); It; ++It)
        {
            FProperty* P = *It;
            EEditableMenu Editable = GetEditableMenu(P);
            FString S = GetEditableMenuString(Editable);
            UE_LOG(LogTemp, Error, TEXT("on editable changed = %s"), *S);
        }

        // UserDefinedStructureCompilerUtils::CreateVariables => forcely configure CPF_ flags with using FStructureEditorUtils::GetVarDesc

  
        //UserDefinedStruct->EditorData.
        //FStructureEditorUtils::ChangeEditableOnBPInstance()

        {
            TGuardValue<FStructureEditorUtils::EStructureEditorChangeInfo> ActiveChangeGuard(FStructureEditorUtils::FStructEditorManager::ActiveChange, FStructureEditorUtils::EStructureEditorChangeInfo::Unknown);
            UserDefinedStruct->Status = EUserDefinedStructureStatus::UDSS_Dirty;
            IKismetCompilerInterface& Compiler = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
            FCompilerResultsLog Results;
            Compiler.CompileStructure(UserDefinedStruct, Results);
            UserDefinedStruct->MarkPackageDirty();
        }

        for (TFieldIterator<FProperty> It(UserDefinedStruct); It; ++It)
        {
            FProperty* P = *It;
            EEditableMenu Editable = GetEditableMenu(P);
            FString S = GetEditableMenuString(Editable);
            UE_LOG(LogTemp, Error, TEXT("on editable changed after = %s"), *S);
        }
    }
}

TSharedRef<SWidget> FAdvancedStructurePropertyLayout::OnEditableWidgetGenerated(TSharedPtr<uint8> Type)
{
    FString Name = GetEditableMenuString((EEditableMenu)(*Type));

    return SNew(STextBlock).Text(FText::AsCultureInvariant(Name));
}

FText FAdvancedStructurePropertyLayout::GetEditableText() const
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
        FProperty* Property = FStructureEditorUtils::GetPropertyByGuid(UserDefinedStruct, Guid);
        EEditableMenu Menu = GetEditableMenu(Property);
        FString Name = GetEditableMenuString(Menu);

        return FText::AsCultureInvariant(Name);
    }

    return FText::GetEmpty();
}

void FAdvancedStructurePropertyLayout::OnChanged()
{
    OnGenerateChildren.ExecuteIfBound();
}

EVisibility FAdvancedStructurePropertyLayout::GetErrorIconVisibility()
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
        auto Desc = FStructureEditorUtils::GetVarDesc(UserDefinedStruct).FindByPredicate(FStructureEditorUtils::FFindByGuidHelper<FStructVariableDescription>(Guid));
        if (Desc && Desc->bInvalidMember)
        {
            return EVisibility::Visible;
        }
    }

    return EVisibility::Collapsed;
}

void FAdvancedStructurePropertyLayout::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
    ChildrenBuilder.AddCustomRow(LOCTEXT("Tooltip", "Tooltip"))
    .NameContent()
    [
        SNew(STextBlock)
        .Text(LOCTEXT("Tooltip", "Tooltip"))
        .Font(IDetailLayoutBuilder::GetDetailFont())
    ]
    .ValueContent()
    [
        SNew(SEditableTextBox)
        .Text(this, &FAdvancedStructurePropertyLayout::GetTooltipText)
        .OnTextCommitted(this, &FAdvancedStructurePropertyLayout::OnTooltipTextCommitted)
        .Font(IDetailLayoutBuilder::GetDetailFont())
    ];

    RegisteredEnumItemsList.Empty();
    TSharedPtr<EnumItems> Items = MakeShareable(new EnumItems());
    for (int Index = 0; Index < EEditableMenu::EEditableMenu_MAX; ++Index)
    {
        Items->Add(MakeShareable(new uint8(Index)));
    };
    RegisteredEnumItemsList.Add(Items);

    EEditableMenu CurrentIndex = EEditableMenu::NoAccess;
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
        FProperty* Property = FStructureEditorUtils::GetPropertyByGuid(UserDefinedStruct, Guid);
        CurrentIndex = GetEditableMenu(Property);
    }

    ChildrenBuilder.AddCustomRow(LOCTEXT("Tooltip", "Tooltip"))
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
        .OnSelectionChanged(this, &FAdvancedStructurePropertyLayout::OnEditableChanged)
        .OnGenerateWidget(this, &FAdvancedStructurePropertyLayout::OnEditableWidgetGenerated)
        [
            SNew(STextBlock)
            .Text(this, &FAdvancedStructurePropertyLayout::GetEditableText)
            .Font(IDetailLayoutBuilder::GetDetailFont())
        ]
    ];
}

void FAdvancedStructurePropertyLayout::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
    auto K2Schema = GetDefault<UEdGraphSchema_K2>();
    const float PinInfoUIWidth = 240.0f;

    TSharedPtr<SImage> ErrorIcon;

    NodeRow
    .NameContent()
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        .HAlign(HAlign_Left)
        .VAlign(VAlign_Center)
        [
            SAssignNew(ErrorIcon, SImage)
            .Image(FEditorStyle::GetBrush("Icons.Error"))
        ]
        + SHorizontalBox::Slot()
        .FillWidth(1)
        .VAlign(VAlign_Center)
        [
            SNew(SEditableTextBox)
            .Text(this, &FAdvancedStructurePropertyLayout::OnGetNameText)
            .OnTextCommitted(this, &FAdvancedStructurePropertyLayout::OnNameTextCommitted)
            .Font(IDetailLayoutBuilder::GetDetailFont())
        ]
    ]
    .ValueContent()
    .MaxDesiredWidth(PinInfoUIWidth)
    .MinDesiredWidth(PinInfoUIWidth)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .Padding(0.0f, 0.0f, 4.0f, 0.0f)
        [
            SNew(SPinTypeSelector, FGetPinTypeTree::CreateUObject(K2Schema, &UEdGraphSchema_K2::GetVariableTypeTree))
            .TargetPinType(this, &FAdvancedStructurePropertyLayout::OnGetPinInfo)
            .OnPinTypePreChanged(this, &FAdvancedStructurePropertyLayout::OnPrePinInfoChanged)
            .OnPinTypeChanged(this, &FAdvancedStructurePropertyLayout::OnPinInfoChanged)
            .Schema(K2Schema)
            .TypeTreeFilter(ETypeTreeFilter::None)
            .Font(IDetailLayoutBuilder::GetDetailFont())
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .HAlign(HAlign_Right)
        .VAlign(VAlign_Center)
        [
            SNew(SButton)
            .OnClicked(this, &FAdvancedStructurePropertyLayout::OnMoveUp)
            .IsEnabled(!(EPropertyPositionFlag::First & PositionFlags))
            [
                SNew(SImage)
                .Image(FEditorStyle::GetBrush("BlueprintEditor.Details.ArgUpButton"))
            ]
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .HAlign(HAlign_Right)
        .VAlign(VAlign_Center)
        [
            SNew(SButton)
            .OnClicked(this, &FAdvancedStructurePropertyLayout::OnMoveDown)
            .IsEnabled(!(EPropertyPositionFlag::Last & PositionFlags))
            [
                SNew(SImage)
                .Image(FEditorStyle::GetBrush("BlueprintEditor.Details.ArgDownButton"))
            ]
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .HAlign(HAlign_Right)
        .VAlign(VAlign_Center)
        [
            PropertyCustomizationHelpers::MakeClearButton(
                FSimpleDelegate::CreateSP(this, &FAdvancedStructurePropertyLayout::OnRemoveProperty),
                LOCTEXT("RemoveVariable", "Remove member variable"),
                TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateSP(this, &FAdvancedStructurePropertyLayout::IsRemoveButtonEnabled))
            )
        ]
    ];

    if (ErrorIcon.IsValid())
    {
        ErrorIcon->SetVisibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FAdvancedStructurePropertyLayout::GetErrorIconVisibility)));
    }
}

FName FAdvancedStructurePropertyLayout::GetName() const
{
    return FName(*Guid.ToString());
}

const FSlateBrush* FAdvancedStructureStructureLayout::GetStatusImage() const
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
        switch (UserDefinedStruct->Status.GetValue())
        {
        case EUserDefinedStructureStatus::UDSS_Error:
            return FEditorStyle::GetBrush("Kismet.Status.Error.Small");
        case EUserDefinedStructureStatus::UDSS_UpToDate:
            return FEditorStyle::GetBrush("Kismet.Status.Good.Small");
        default:
            return FEditorStyle::GetBrush("Kismet.Status.Unknown.Small");
        }
    }
    return nullptr;
}

FText FAdvancedStructureStructureLayout::GetStatusTooltipText() const
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
        if (UserDefinedStruct->Status.GetValue() == EUserDefinedStructureStatus::UDSS_Error)
        {
            return FText::FromString(UserDefinedStruct->ErrorMessage);
        }
    }
    return FText::GetEmpty();
}

FReply FAdvancedStructureStructureLayout::AddNewProperty()
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
        FStructureEditorUtils::AddVariable(UserDefinedStruct, InitialPinType);
    }
    return FReply::Handled();
}

FText FAdvancedStructureStructureLayout::GetTooltipText() const
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
        return FText::FromString(FStructureEditorUtils::GetTooltip(UserDefinedStruct));
    }
    return FText();
}

void FAdvancedStructureStructureLayout::OnTooltipTextCommited(const FText& NewText, ETextCommit::Type InTextCommit)
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
        FStructureEditorUtils::ChangeTooltip(UserDefinedStruct, NewText.ToString());
    }
}

void FAdvancedStructureStructureLayout::OnChanged()
{
    OnGenerateChildren.ExecuteIfBound();
}

void FAdvancedStructureStructureLayout::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
    const float NameWidth = 80.0f;
    const float ContentWidth = 130.0f;

    ChildrenBuilder.AddCustomRow(FText::GetEmpty())
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .MaxWidth(NameWidth)
        .HAlign(HAlign_Left)
        .VAlign(VAlign_Center)
        [
            SNew(SImage)
            .Image(this, &FAdvancedStructureStructureLayout::GetStatusImage)
            .ToolTipText(this, &FAdvancedStructureStructureLayout::GetStatusTooltipText)
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .HAlign(HAlign_Left)
        [
            SNew(SBox)
            .WidthOverride(ContentWidth)
            [
                SNew(SButton)
                .HAlign(HAlign_Center)
                .Text(LOCTEXT("NewStructureProperty", "New Variable"))
                .OnClicked(this, &FAdvancedStructureStructureLayout::AddNewProperty)
            ]
        ]
    ];

    ChildrenBuilder.AddCustomRow(FText::GetEmpty())
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .MaxWidth(NameWidth)
        .HAlign(HAlign_Left)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("Tooltip", "Tooltip"))
            .Font(IDetailLayoutBuilder::GetDetailFont())
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .HAlign(HAlign_Left)
        [
            SNew(SBox)
            .WidthOverride(ContentWidth)
            [
                SNew(SEditableTextBox)
                .Text(this, &FAdvancedStructureStructureLayout::GetTooltipText)
                .OnTextCommitted(this, &FAdvancedStructureStructureLayout::OnTooltipTextCommited)
                .Font(IDetailLayoutBuilder::GetDetailFont())
            ]
        ]
    ];

    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
        auto VarDescs = FStructureEditorUtils::GetVarDesc(UserDefinedStruct);
        int32 PropertySize = VarDescs.Num();
        int32 PropertyIndex = -1;
        // TODO: Use FStructureEditorUtils instead.
        for (TFieldIterator<FProperty> It(UserDefinedStruct); It; ++It)
        {
            PropertyIndex++;
            uint32 PositionFlags = 0;
            PositionFlags |= (0 == PropertyIndex) ? EPropertyPositionFlag::First : 0;
            PositionFlags |= ((PropertySize - 1) == PropertyIndex) ? EPropertyPositionFlag::Last : 0;

            FProperty* Property = *It;
            FGuid Guid = FStructureEditorUtils::GetGuidForProperty(Property);

            TSharedRef<FAdvancedStructurePropertyLayout> PropLayout = MakeShareable(new FAdvancedStructurePropertyLayout(DetailCustomization, SharedThis(this), Guid, PositionFlags));

            ChildrenBuilder.AddCustomBuilder(PropLayout);
        }
    }
}

FName FAdvancedStructureStructureLayout::GetName() const
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();
        return UserDefinedStruct->GetFName();
    }

    return NAME_None;
}

void FAdvancedStructurePropertiesDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
    // ref: https://github.com/johnfredcee/UBrowse/blob/master/Source/UBrowse/Private/SUBrowser.cpp

    IDetailCategoryBuilder& StructureCategory = DetailLayout.EditCategory("Structure", FText::GetEmpty(), ECategoryPriority::Important);
    const TArray<TWeakObjectPtr<UObject>> Objects = DetailLayout.GetDetailsView()->GetSelectedObjects();
    check(Objects.Num() > 0);

    if (Objects.Num() == 1)
    {
        UserDefinedStruct = CastChecked<UUserDefinedStruct>(Objects[0].Get());
        StructureLayout = MakeShareable(new FAdvancedStructureStructureLayout(SharedThis(this)));
        StructureCategory.AddCustomBuilder(StructureLayout.ToSharedRef());
    }
}

void FAdvancedStructurePropertiesDetailCustomization::PreChange(const UUserDefinedStruct* InUserDefinedStruct, FStructureEditorUtils::EStructureEditorChangeInfo Info)
{
}

void FAdvancedStructurePropertiesDetailCustomization::PostChange(const UUserDefinedStruct* InUserDefinedStruct, FStructureEditorUtils::EStructureEditorChangeInfo Info)
{
    if (InUserDefinedStruct && (GetUserDefinedStruct() == InUserDefinedStruct))
    {
        if (StructureLayout.IsValid())
        {
            StructureLayout->OnChanged();
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
    for (TFieldIterator<FProperty> It(InUserDefinedStruct); It; ++It)
    {
        FProperty* Property = *It;

        EEditableMenu Editable = GetEditableMenu(Property);
        FString S = GetEditableMenuString(Editable);
        UE_LOG(LogTemp, Error, TEXT("pre change = %s"), *S);
    }

    if (Info != FStructureEditorUtils::DefaultValueChanged)
    {
        StructData->Destroy();
        DetailsView->SetObject(nullptr);
        DetailsView->OnFinishedChangingProperties().Clear();
    }
}

void FAdvancedStructureDefaultsNotification::PostChange(const UUserDefinedStruct* InUserDefinedStruct, FStructureEditorUtils::EStructureEditorChangeInfo Info)
{
    for (TFieldIterator<FProperty> It(InUserDefinedStruct); It; ++It)
    {
        FProperty* Property = *It;

        EEditableMenu Editable = GetEditableMenu(Property);
        FString S = GetEditableMenuString(Editable);
        UE_LOG(LogTemp, Error, TEXT("post change = %s"), *S);
    }

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
            
            EEditableMenu Editable = GetEditableMenu(Property);

            IDetailPropertyRow* Row = StructureCategory.AddExternalStructureProperty(StructData, (*It)->GetFName());
            if (Row)
            {
                switch (Editable)
                {
                case EEditableMenu::EditAnywhere:
                case EEditableMenu::EditDefaultsOnly:
                    Row->Visibility(EVisibility::Visible);
                    Row->IsEnabled(true);
                    break;
                case EEditableMenu::EditInstanceOnly:
                case EEditableMenu::VisibleAnywhere:
                case EEditableMenu::VisibleDefaultsOnly:
                    Row->Visibility(EVisibility::Visible);
                    Row->IsEnabled(false);
                    break;
                case EEditableMenu::VisibleInstanceOnly:
                case EEditableMenu::NoAccess:
                    Row->Visibility(EVisibility::Hidden);
                    Row->IsEnabled(false);
                    break;
                }
            }
        }
    }
}

#undef LOCTEXT_NAMESPACE
