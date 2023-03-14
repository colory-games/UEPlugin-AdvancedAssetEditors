// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Tools/BaseAssetToolkit.h"
#include "Engine/UserDefinedStruct.h"


class FAdvancedStrcutureAssetsEditorToolkit : public FBaseAssetToolkit
{
    TSharedRef<SDockTab> HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier);

    FReply OnAddNewField();
    const FSlateBrush* OnGetStructureStatus() const;
    FText OnGetStatusTooltip() const;
    void FillToolbar(FToolBarBuilder& ToolbarBuilder);

    UUserDefinedStruct* UserDefinedStruct;

public:
    FAdvancedStrcutureAssetsEditorToolkit(UAssetEditor* InOwningAssetEditor);

    void Initialize(UUserDefinedStruct* InUserDefinedStruct);

    virtual bool IsPrimaryEditor() const override
    {
        return false;
    }

    virtual ~FAdvancedStrcutureAssetsEditorToolkit();

    virtual FName GetToolkitFName() const override;

    virtual FText GetBaseToolkitName() const override;

    virtual FLinearColor GetWorldCentricTabColorScale() const override;

    virtual FString GetWorldCentricTabPrefix() const override;

    virtual FString GetDocumentationLink() const override;

    virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

    virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
};
