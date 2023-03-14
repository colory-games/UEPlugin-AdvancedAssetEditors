// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AssetTypeActions_Base.h"

#include "IDetailCustomization.h"

class FAdvancedStructureAssetsActions : public FAssetTypeActions_Base
{
public:
    FAdvancedStructureAssetsActions();

    virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

    bool HasActions(const TArray<UObject*>& InObjects) const;

    void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder);

    virtual uint32 GetCategories() override;

    virtual FText GetName() const override;

    virtual UClass* GetSupportedClass() const override;

    virtual FColor GetTypeColor() const override;
};
