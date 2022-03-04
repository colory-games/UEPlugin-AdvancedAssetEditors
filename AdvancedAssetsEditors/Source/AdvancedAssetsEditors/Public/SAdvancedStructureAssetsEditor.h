// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet2/StructureEditorUtils.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "IDetailCustomization.h"
#include "Engine/UserDefinedStruct.h"
#include "Misc/NotifyHook.h"


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


class FAdvancedStructurePropertiesDetailCustomization : public IDetailCustomization
{
    void OnEditableChanged(TSharedPtr<uint8> Type, ESelectInfo::Type SelectionType, TWeakObjectPtr<UObject> Object, FProperty* Property);
    TSharedRef<SWidget> OnEditableWidgetGenerated(TSharedPtr<uint8> Type);
    FText GetEditableText(FProperty* Property) const;
    FEdGraphPinType OnGetPinInfo(TWeakObjectPtr<UObject> Object, FGuid Guid) const;
    void OnPrePinInfoChanged(const FEdGraphPinType& PinType);
    void OnPinInfoChanged(const FEdGraphPinType& PinType, TWeakObjectPtr<UObject> Object, FGuid Guid);

    typedef TArray<TSharedPtr<uint8>> EnumItems;
    typedef TArray<TSharedPtr<EnumItems>> EnumItemsList;
    EnumItemsList RegisteredEnumItemsList;

public:
    static TSharedRef<IDetailCustomization> MakeInstance()
    {
        return MakeShareable(new FAdvancedStructurePropertiesDetailCustomization);
    }

    FAdvancedStructurePropertiesDetailCustomization()
    {
    }

    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
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