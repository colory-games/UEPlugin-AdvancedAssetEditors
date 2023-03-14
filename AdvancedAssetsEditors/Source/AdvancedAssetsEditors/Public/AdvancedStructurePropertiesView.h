// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "IDetailCustomNodeBuilder.h"
#include "IDetailCustomization.h"
#include "Kismet2/StructureEditorUtils.h"
#include "Engine/UserDefinedStruct.h"

#include "Common.h"
#include "AdvancedStructureNotification.h"

#include "SAdvancedStructureAssetsEditor.h"


class FAdvancedStructureStructureLayout;

class FAdvancedStructurePropertiesDetailCustomization : public IDetailCustomization, FStructureEditorUtils::INotifyOnStructChanged
{
    TWeakObjectPtr<UUserDefinedStruct> UserDefinedStruct;
    TSharedPtr<FAdvancedStructureStructureLayout> StructureLayout;

public:
    TSharedPtr<FAdvancedStructureDefaultsNotification> DefaultsNotification;

    static TSharedRef<IDetailCustomization> MakeInstance(TSharedPtr<FAdvancedStructureDefaultsNotification> Notification)
    {
        return MakeShareable(new FAdvancedStructurePropertiesDetailCustomization(Notification));
    }

    FAdvancedStructurePropertiesDetailCustomization(TSharedPtr<FAdvancedStructureDefaultsNotification> Notification) : DefaultsNotification(Notification)
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
