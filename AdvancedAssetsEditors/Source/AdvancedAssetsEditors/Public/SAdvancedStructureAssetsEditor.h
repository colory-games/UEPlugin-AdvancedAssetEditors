// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "IDetailCustomization.h"


class SAdvancedStructureAssetsEditor : public SCompoundWidget
{
    UScriptStruct* ScriptStruct;

    TSharedPtr<IDetailsView> PropertiesView;
    TSharedPtr<IDetailsView> DefaultsView;

public:
    SLATE_BEGIN_ARGS(SAdvancedStructureAssetsEditor) { }
    SLATE_END_ARGS()

    virtual ~SAdvancedStructureAssetsEditor();

    void Construct(const FArguments& InArgs, UScriptStruct* InCustomStructAsset,
                   const TSharedRef<ISlateStyle>& InStyle);
};


class FAdvancedStructurePropertiesDetailCustomization : public IDetailCustomization
{
    void OnEditableChanged(TSharedPtr<uint8> Type, ESelectInfo::Type SelectionType, TWeakObjectPtr<UObject> Object, FProperty* Property);
    TSharedRef<SWidget> OnEditableWidgetGenerated(TSharedPtr<uint8> Type);
    FText GetEditableText(FProperty* Property) const;

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

    typedef TArray<TSharedPtr<uint8>> EnumItems;
    typedef TArray<TSharedPtr<EnumItems>> EnumItemsList;
    EnumItemsList RegisteredEnumItemsList;

public:
    static TSharedRef<IDetailCustomization> MakeInstance()
    {
        return MakeShareable(new FAdvancedStructureDefaultsDetailCustomization);
    }

    FAdvancedStructureDefaultsDetailCustomization()
    {
    }

    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
};