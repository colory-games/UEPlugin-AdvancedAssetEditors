// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet2/StructureEditorUtils.h"


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
