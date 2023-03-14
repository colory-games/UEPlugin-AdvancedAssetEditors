#include "AdvancedStructurePropertiesView.h"

#include "DragAndDrop/DecoratedDragDropOp.h"
#include "IDetailDragDropHandler.h"
#include "UserDefinedStructure/UserDefinedStructEditorData.h"
#include "KismetCompilerModule.h"
#include "Kismet2/CompilerResultsLog.h"
#include "DetailLayoutBuilder.h"
#include "SPinTypeSelector.h"
#include "DetailWidgetRow.h"
#include "PropertyCustomizationHelpers.h"
#include "IDetailChildrenBuilder.h"

#define LOCTEXT_NAMESPACE "AdvancedAssetEditors"

class FAdvancedStructureEdtiorPropertyDragDropOp : public FDecoratedDragDropOp
{
    TWeakPtr<FAdvancedStructurePropertiesDetailCustomization> DetailCustomization;
    FGuid Guid;
    FString PropertyFriendlyName;
public:
    DRAG_DROP_OPERATOR_TYPE(FAdvancedStructureEdtiorPropertyDragDropOp, FDecoratedDragDropOp);

    FAdvancedStructureEdtiorPropertyDragDropOp(TWeakPtr<FAdvancedStructurePropertiesDetailCustomization> DetailCustomization, const FGuid& Guid)
        : DetailCustomization(DetailCustomization), Guid(Guid)
    {
        MouseCursor = EMouseCursor::GrabHandClosed;
        if (TSharedPtr<FAdvancedStructurePropertiesDetailCustomization> StructureDetailsSP = DetailCustomization.Pin())
        {
            PropertyFriendlyName = FStructureEditorUtils::GetVariableFriendlyName(StructureDetailsSP->GetUserDefinedStruct(), Guid);
        }
    }

    void Init()
    {
        SetValidTarget(false);
        SetupDefaults();
        Construct();
    }

    void SetValidTarget(bool IsValidTarget)
    {
        FFormatNamedArguments Args;
        Args.Add(TEXT("StructVariableName"), FText::FromString(PropertyFriendlyName));

        if (IsValidTarget)
        {
            CurrentHoverText = FText::Format(LOCTEXT("MoveVariableHere", "Move '{StructVariableName}' Here"), Args);
            CurrentIconBrush = FAppStyle::Get().GetBrush("Graph.ConnectorFeedback.OK");
        }
        else
        {
            CurrentHoverText = FText::Format(LOCTEXT("CannotMoveVariableHere", "Cannot Move '{StructVariableName}' Here"), Args);
            CurrentIconBrush = FAppStyle::Get().GetBrush("Graph.ConnectorFeedback.Error");
        }
    }

    const TWeakPtr<FAdvancedStructurePropertiesDetailCustomization>& GetStructureDetails() const
    {
        return DetailCustomization;
    }

    const FGuid& GetFieldGuid() const
    {
        return Guid;
    }
};

class FAdvancedStructureEdtiorPropertyDragDropHandler : public IDetailDragDropHandler
{
    TWeakPtr<FAdvancedStructurePropertiesDetailCustomization> DetailCustomization;
    FGuid Guid;

public:
    FAdvancedStructureEdtiorPropertyDragDropHandler(TWeakPtr<FAdvancedStructurePropertiesDetailCustomization> DetailCustomization, const FGuid& Guid)
        : DetailCustomization(DetailCustomization), Guid(Guid)
    {
    }

    virtual TSharedPtr<FDragDropOperation> CreateDragDropOperation() const override
    {
        TSharedPtr<FAdvancedStructureEdtiorPropertyDragDropOp> DragOp = MakeShared<FAdvancedStructureEdtiorPropertyDragDropOp>(DetailCustomization, Guid);
        DragOp->Init();
        return DragOp;
    }

    virtual TOptional<EItemDropZone> CanAcceptDrop(const FDragDropEvent& DragDropSource, EItemDropZone DropZone) const override
    {
        const TSharedPtr<FAdvancedStructureEdtiorPropertyDragDropOp> DragOp = DragDropSource.GetOperationAs<FAdvancedStructureEdtiorPropertyDragDropOp>();
        if (!DragOp.IsValid())
        {
            return TOptional<EItemDropZone>();
        }

        const TSharedPtr<FAdvancedStructurePropertiesDetailCustomization> OtherStructureDetailsSP = DragOp->GetStructureDetails().Pin();
        const TSharedPtr<FAdvancedStructurePropertiesDetailCustomization> MyStructureDetailsSP = DetailCustomization.Pin();
        if (!OtherStructureDetailsSP.IsValid() || !MyStructureDetailsSP.IsValid() || OtherStructureDetailsSP->GetUserDefinedStruct() != MyStructureDetailsSP->GetUserDefinedStruct())
        {
            DragOp->SetValidTarget(false);
            return TOptional<EItemDropZone>();
        }

        const EItemDropZone OverrideZone = (DropZone == EItemDropZone::BelowItem) ? EItemDropZone::BelowItem : EItemDropZone::AboveItem;
        const FStructureEditorUtils::EMovePosition MovePosition = (OverrideZone == EItemDropZone::BelowItem) ? FStructureEditorUtils::PositionBelow : FStructureEditorUtils::PositionAbove;
        if (!FStructureEditorUtils::CanMoveVariable(MyStructureDetailsSP->GetUserDefinedStruct(), DragOp->GetFieldGuid(), Guid, MovePosition))
        {
            DragOp->SetValidTarget(false);
            return TOptional<EItemDropZone>();
        }

        DragOp->SetValidTarget(true);
        return OverrideZone;
    }


    virtual bool AcceptDrop(const FDragDropEvent& DragDropSource, EItemDropZone DropZone) const override
    {
        const TSharedPtr<FAdvancedStructureEdtiorPropertyDragDropOp> DragOp = DragDropSource.GetOperationAs<FAdvancedStructureEdtiorPropertyDragDropOp>();
        if (!DragOp.IsValid())
        {
            return false;
        }

        // Struct must match between drag source and drop target
        const TSharedPtr<FAdvancedStructurePropertiesDetailCustomization> OtherStructureDetailsSP = DragOp->GetStructureDetails().Pin();
        const TSharedPtr<FAdvancedStructurePropertiesDetailCustomization> MyStructureDetailsSP = DetailCustomization.Pin();
        if (!OtherStructureDetailsSP.IsValid() || !MyStructureDetailsSP.IsValid() || OtherStructureDetailsSP->GetUserDefinedStruct() != MyStructureDetailsSP->GetUserDefinedStruct())
        {
            return false;
        }

        const FStructureEditorUtils::EMovePosition MovePosition = (DropZone == EItemDropZone::BelowItem) ? FStructureEditorUtils::PositionBelow : FStructureEditorUtils::PositionAbove;

        UUserDefinedStruct* UserDefinedStruct = MyStructureDetailsSP->GetUserDefinedStruct();
        FAdvancedStructureData Data = GetAdvancedStructureData(UserDefinedStruct);
        bool Result = FStructureEditorUtils::MoveVariable(UserDefinedStruct, DragOp->GetFieldGuid(), Guid, MovePosition);
        if (!Result)
        {
            return false;
        }
        RestoreAdvancedStructureData(Data, UserDefinedStruct);

        // Notify the editor about the upate.
        MyStructureDetailsSP->DefaultsNotification->PostChange(UserDefinedStruct, FStructureEditorUtils::EStructureEditorChangeInfo::Unknown);
        UserDefinedStruct->MarkPackageDirty();
        FStructureEditorUtils::BroadcastPostChange(UserDefinedStruct);

        return true;
    }
};

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

        FAdvancedStructureData Data = GetAdvancedStructureData(UserDefinedStruct);
        FStructureEditorUtils::RenameVariable(UserDefinedStruct, Guid, NewText.ToString());
        RestoreAdvancedStructureData(Data, UserDefinedStruct);

        // Notify the editor about the upate.
        DetailCustomizationSP->DefaultsNotification->PostChange(UserDefinedStruct, FStructureEditorUtils::EStructureEditorChangeInfo::Unknown);
        UserDefinedStruct->MarkPackageDirty();
        FStructureEditorUtils::BroadcastPostChange(UserDefinedStruct);
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

        FAdvancedStructureData Data = GetAdvancedStructureData(UserDefinedStruct);
        FStructureEditorUtils::ChangeVariableType(UserDefinedStruct, Guid, PinType);
        RestoreAdvancedStructureData(Data, UserDefinedStruct);

        // Notify the editor about the upate.
        DetailCustomizationSP->DefaultsNotification->PostChange(UserDefinedStruct, FStructureEditorUtils::EStructureEditorChangeInfo::Unknown);
        UserDefinedStruct->MarkPackageDirty();
        FStructureEditorUtils::BroadcastPostChange(UserDefinedStruct);
    }
}

void FAdvancedStructurePropertyLayout::OnRemoveProperty()
{
    auto DetailCustomizationSP = DetailCustomization.Pin();
    if (DetailCustomizationSP.IsValid())
    {
        auto UserDefinedStruct = DetailCustomizationSP->GetUserDefinedStruct();

        FAdvancedStructureData Data = GetAdvancedStructureData(UserDefinedStruct);
        FStructureEditorUtils::RemoveVariable(UserDefinedStruct, Guid);
        RestoreAdvancedStructureData(Data, UserDefinedStruct);

        // Notify the editor about the upate.
        DetailCustomizationSP->DefaultsNotification->PostChange(UserDefinedStruct, FStructureEditorUtils::EStructureEditorChangeInfo::Unknown);
        UserDefinedStruct->MarkPackageDirty();
        FStructureEditorUtils::BroadcastPostChange(UserDefinedStruct);
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

        FAdvancedStructureData Data = GetAdvancedStructureData(UserDefinedStruct);
        {
            TGuardValue<FStructureEditorUtils::EStructureEditorChangeInfo> ActiveChangeGuard(FStructureEditorUtils::FStructEditorManager::ActiveChange, FStructureEditorUtils::EStructureEditorChangeInfo::Unknown);
            UserDefinedStruct->Status = EUserDefinedStructureStatus::UDSS_Dirty;
            IKismetCompilerInterface& Compiler = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
            FCompilerResultsLog Results;
            Compiler.CompileStructure(UserDefinedStruct, Results);
        }
        RestoreAdvancedStructureData(Data, UserDefinedStruct);

        // Notify the editor about the upate.
        DetailCustomizationSP->DefaultsNotification->PostChange(UserDefinedStruct, FStructureEditorUtils::EStructureEditorChangeInfo::Unknown);
        UserDefinedStruct->MarkPackageDirty();
        FStructureEditorUtils::BroadcastPostChange(UserDefinedStruct);
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
            PropertyCustomizationHelpers::MakeClearButton(
                FSimpleDelegate::CreateSP(this, &FAdvancedStructurePropertyLayout::OnRemoveProperty),
                LOCTEXT("RemoveVariable", "Remove member variable"),
                TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateSP(this, &FAdvancedStructurePropertyLayout::IsRemoveButtonEnabled))
            )
        ]
    ]
    .DragDropHandler(MakeShared<FAdvancedStructureEdtiorPropertyDragDropHandler>(DetailCustomization, Guid));

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

        FAdvancedStructureData Data = GetAdvancedStructureData(UserDefinedStruct);
        FStructureEditorUtils::AddVariable(UserDefinedStruct, InitialPinType);
        RestoreAdvancedStructureData(Data, UserDefinedStruct);

        // Notify the editor about the upate.
        DetailCustomizationSP->DefaultsNotification->PostChange(UserDefinedStruct, FStructureEditorUtils::EStructureEditorChangeInfo::Unknown);
        UserDefinedStruct->MarkPackageDirty();
        FStructureEditorUtils::BroadcastPostChange(UserDefinedStruct);
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

#undef LOCTEXT_NAMESPACE
