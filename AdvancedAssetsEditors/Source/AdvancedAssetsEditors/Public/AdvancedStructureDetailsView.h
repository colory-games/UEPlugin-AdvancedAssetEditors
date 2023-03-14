// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "IDetailCustomization.h"
#include "Engine/UserDefinedStruct.h"

#include "AdvancedStructureNotification.h"


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