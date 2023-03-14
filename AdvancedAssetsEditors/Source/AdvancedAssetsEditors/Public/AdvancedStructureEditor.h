#pragma once

#include "Tools/UAssetEditor.h"

#include "AdvancedStructureEditor.generated.h"

UCLASS()
class ADVANCEDASSETSEDITORS_API UAdvancedStructureEditor : public UAssetEditor
{
	GENERATED_BODY()

public:
	void Initialize(const TArray<UObject*>& InObjects);

	IAssetEditorInstance* GetInstanceInterface();

	void GetObjectsToEdit(TArray<UObject*>& OutObjects) override;
	virtual TSharedPtr<FBaseAssetToolkit> CreateToolkit() override;

	UPROPERTY()
	TArray<UObject*> ObjectsToEdit;
};
