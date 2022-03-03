// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "IDetailCustomization.h"
#include "Engine/UserDefinedStruct.h"


class SAdvancedStructureAssetsEditor : public SCompoundWidget
{
    UUserDefinedStruct* UserDefinedStruct;
    TSharedPtr<FStructOnScope> StructData;

    TSharedPtr<IDetailsView> PropertiesView;
    TSharedPtr<IDetailsView> DefaultsView;

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
    bool IsPropertyChangeComplete();

    typedef TArray<TSharedPtr<uint8>> EnumItems;
    typedef TArray<TSharedPtr<EnumItems>> EnumItemsList;
    EnumItemsList RegisteredEnumItemsList;

    TSharedPtr<FStructOnScope> StructData;
    UUserDefinedStruct* UserDefinedStruct;
    int32 PropertyChangeRecursionGuard;

public:
    static TSharedRef<IDetailCustomization> MakeInstance(TSharedPtr<FStructOnScope> InStructData, UUserDefinedStruct* UserDefinedStruct)
    {
        return MakeShareable(new FAdvancedStructureDefaultsDetailCustomization(InStructData, UserDefinedStruct));
    }

    FAdvancedStructureDefaultsDetailCustomization(TSharedPtr<FStructOnScope> InStructData, UUserDefinedStruct* UserDefinedStruct)
        : StructData(InStructData), UserDefinedStruct(UserDefinedStruct), PropertyChangeRecursionGuard(0)
    {
    }

    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
};