// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AssetTypeActions_Base.h"

#include "IDetailCustomization.h"

class FAdvancedStructureActions : public FAssetTypeActions_Base
{
public:
	FAdvancedStructureActions(const TSharedRef<ISlateStyle>& InStyle);

	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

	virtual uint32 GetCategories() override;

	virtual FText GetName() const override;

	virtual UClass* GetSupportedClass() const override;

	virtual FColor GetTypeColor() const override;

	TSharedRef<ISlateStyle> Style;
};

class FAdvancedStructureCustomization : public IDetailCustomization
{
public:
	virtual ~FAdvancedStructureCustomization();

	virtual void CustomizeDetails(IDetailLayoutBuilder& Layout) override;

	static TSharedRef<IDetailCustomization> MakeInstance();
};
