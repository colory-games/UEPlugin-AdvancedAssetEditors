// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Toolkits/AssetEditorToolkit.h"

#include "CustomStructAsset/CustomStructAssetBase.h"


class FCustomStructAssetEditorToolkit : public FAssetEditorToolkit
{
    UCustomStructAssetBase* CustomStructAsset;
    TSharedRef<ISlateStyle> Style;

    TSharedRef<SDockTab> HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier);

public:
    FCustomStructAssetEditorToolkit(const TSharedRef<ISlateStyle>& InStyle);

    virtual ~FCustomStructAssetEditorToolkit();

    void Initialize(UCustomStructAssetBase* InCustomStructAsset,
                    const EToolkitMode::Type InMode,
                    const TSharedPtr<IToolkitHost>& InToolkitHost);

    virtual FName GetToolkitFName() const override;

    virtual FText GetBaseToolkitName() const override;

    virtual FLinearColor GetWorldCentricTabColorScale() const override;

    virtual FString GetWorldCentricTabPrefix() const override;

    virtual FString GetDocumentationLink() const override;

    virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

    virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
};
