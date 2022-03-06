// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet2/StructureEditorUtils.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "IDetailCustomization.h"
#include "Engine/UserDefinedStruct.h"
#include "Misc/NotifyHook.h"
#include "IDetailChildrenBuilder.h"


class FAdvancedStructureDefaultsNotification : public FStructureEditorUtils::INotifyOnStructChanged, public TSharedFromThis<FAdvancedStructureDefaultsNotification>, public FNotifyHook
{
    TSharedPtr<FStructOnScope> StructData;
    TSharedPtr<IDetailsView> DetailsView;
    UUserDefinedStruct* UserDefinedStruct;
    int32 PropertyChangeRecursionGuard = 0;

public:
    FAdvancedStructureDefaultsNotification(UUserDefinedStruct* EditedStruct) : UserDefinedStruct(EditedStruct), PropertyChangeRecursionGuard(0)
    {
    }

    virtual ~FAdvancedStructureDefaultsNotification()
    {
    }

    void Initialize();
    UUserDefinedStruct* GetUserDefinedStruct();
    TSharedPtr<SWidget> GetView();
    void SetView(TSharedPtr<IDetailsView> View);
    virtual void PreChange(const UUserDefinedStruct* UserDefinedStruct, FStructureEditorUtils::EStructureEditorChangeInfo Info) override;
    virtual void PostChange(const UUserDefinedStruct* UserDefinedStruct, FStructureEditorUtils::EStructureEditorChangeInfo Info) override;
    virtual void NotifyPreChange(FProperty* PropertyAboutToChange) override;
    virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;
    bool IsPropertyChangeComplete();
    TSharedPtr<FStructOnScope> GetStructData() const;
};


class SAdvancedStructureAssetsEditor : public SCompoundWidget
{
    UUserDefinedStruct* UserDefinedStruct;

    TSharedPtr<IDetailsView> PropertiesView;
    TSharedPtr<IDetailsView> DefaultsView;
    TSharedPtr<FAdvancedStructureDefaultsNotification> DefaultsNotification;

public:
    SLATE_BEGIN_ARGS(SAdvancedStructureAssetsEditor) { }
    SLATE_END_ARGS()

    virtual ~SAdvancedStructureAssetsEditor();

    void Construct(const FArguments& InArgs, UUserDefinedStruct* InUserDefinedStruct,
                   const TSharedRef<ISlateStyle>& InStyle);
};


enum EPropertyPositionFlag
{
    First = 0x01,
    Last = 0x02,
};

typedef TArray<TSharedPtr<uint8>> EnumItems;
typedef TArray<TSharedPtr<EnumItems>> EnumItemsList;

class FAdvancedStructurePropertiesDetailCustomization;
class FAdvancedStructureStructureLayout;

class FAdvancedStructurePropertyLayout : public IDetailCustomNodeBuilder, public TSharedFromThis<FAdvancedStructurePropertyLayout>
{
    TWeakPtr<FAdvancedStructurePropertiesDetailCustomization> DetailCustomization;
    TWeakPtr<FAdvancedStructureStructureLayout> StructureLayout;
    FGuid Guid;
    FSimpleDelegate OnGenerateChildren;
    uint32 PositionFlags;
    EnumItemsList RegisteredEnumItemsList;

    FText GetTooltipText() const;
    void OnTooltipTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit);

    FText OnGetNameText() const;
    void OnNameTextCommitted(const FText& NewText, ETextCommit::Type InText);

    FEdGraphPinType OnGetPinInfo() const;
    void OnPrePinInfoChanged(const FEdGraphPinType& PinType);
    void OnPinInfoChanged(const FEdGraphPinType& PinType);

    FReply OnMoveUp();
    FReply OnMoveDown();

    void OnRemoveProperty();
    bool IsRemoveButtonEnabled();

    void OnEditableChanged(TSharedPtr<uint8> Type, ESelectInfo::Type SelectionType);
    TSharedRef<SWidget> OnEditableWidgetGenerated(TSharedPtr<uint8> Type);
    FText GetEditableText() const;

    EVisibility GetErrorIconVisibility();

public:
    FAdvancedStructurePropertyLayout(
        TWeakPtr<FAdvancedStructurePropertiesDetailCustomization> InDetailCustomization, TWeakPtr<FAdvancedStructureStructureLayout> InStructureLayout,
        FGuid InGuid, uint32 InPositionFlags) :
        DetailCustomization(InDetailCustomization),
        StructureLayout(InStructureLayout),
        Guid(InGuid),
        PositionFlags(InPositionFlags) {}

    void OnChanged();
    virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override
    {
        OnGenerateChildren = InOnRegenerateChildren;
    }
    virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
    virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
    virtual void Tick(float DeltaTime) override {}
    virtual bool RequiresTick() const override { return false; }
    virtual FName GetName() const override;
    virtual bool InitiallyCollapsed() const override { return true; }
};

class FAdvancedStructureStructureLayout : public IDetailCustomNodeBuilder, public TSharedFromThis<FAdvancedStructureStructureLayout>
{
    TWeakPtr<FAdvancedStructurePropertiesDetailCustomization> DetailCustomization;
    FSimpleDelegate OnGenerateChildren;
    FEdGraphPinType InitialPinType;

    const FSlateBrush* GetStatusImage() const;
    FText GetStatusTooltipText() const;

    FReply AddNewProperty();
    FText GetTooltipText() const;
    void OnTooltipTextCommited(const FText& NewText, ETextCommit::Type InTextCommit);

public:
    FAdvancedStructureStructureLayout(TWeakPtr<FAdvancedStructurePropertiesDetailCustomization> InDetailCutomization) :
        DetailCustomization(InDetailCutomization),
        InitialPinType(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType()) {}

    void OnChanged();
    virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override
    {
        OnGenerateChildren = InOnRegenerateChildren;
    }
    virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
    virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override {}
    virtual void Tick(float DeltaTime) override {}
    virtual bool RequiresTick() const override { return false; }
    virtual FName GetName() const override;
    virtual bool InitiallyCollapsed() const override { return false; }
};

class FAdvancedStructurePropertiesDetailCustomization : public IDetailCustomization, FStructureEditorUtils::INotifyOnStructChanged
{
    TWeakObjectPtr<UUserDefinedStruct> UserDefinedStruct;
    TSharedPtr<FAdvancedStructureStructureLayout> StructureLayout;

public:
    static TSharedRef<IDetailCustomization> MakeInstance()
    {
        return MakeShareable(new FAdvancedStructurePropertiesDetailCustomization);
    }

    FAdvancedStructurePropertiesDetailCustomization()
    {
    }

    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

    virtual void PreChange(const UUserDefinedStruct* InUserDefinedStruct, FStructureEditorUtils::EStructureEditorChangeInfo Info) override;
    virtual void PostChange(const UUserDefinedStruct* InUserDefinedStruct, FStructureEditorUtils::EStructureEditorChangeInfo Info) override;
    UUserDefinedStruct* GetUserDefinedStruct()
    {
        return UserDefinedStruct.Get();
    }
};


class FAdvancedStructureDefaultsDetailCustomization : public IDetailCustomization
{
    void OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent);

    typedef TArray<TSharedPtr<uint8>> EnumItems;
    typedef TArray<TSharedPtr<EnumItems>> EnumItemsList;
    EnumItemsList RegisteredEnumItemsList;

    UUserDefinedStruct* UserDefinedStruct;
    TSharedPtr<FAdvancedStructureDefaultsNotification> DefaultsNotification;

public:
    static TSharedRef<IDetailCustomization> MakeInstance(
        UUserDefinedStruct* UserDefinedStruct,
        TSharedPtr<FAdvancedStructureDefaultsNotification> Notification)
    {
        return MakeShareable(new FAdvancedStructureDefaultsDetailCustomization(UserDefinedStruct, Notification));
    }

    FAdvancedStructureDefaultsDetailCustomization(
        UUserDefinedStruct* UserDefinedStruct,
        TSharedPtr<FAdvancedStructureDefaultsNotification> Notification)
        : UserDefinedStruct(UserDefinedStruct), DefaultsNotification(Notification)
    {
    }

    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
};