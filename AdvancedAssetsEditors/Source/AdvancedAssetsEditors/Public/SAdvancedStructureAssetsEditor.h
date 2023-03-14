// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "IDetailCustomization.h"

#include "AdvancedStructureNotification.h"


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

    void Construct(const FArguments& InArgs, UUserDefinedStruct* InUserDefinedStruct);
};

class FAdvancedStructurePropertiesDetailCustomization;


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