#pragma once

#include "EditorSubsystem.h"

#include "Engine/UserDefinedStruct.h"

#include "AdvancedStructureEditor.h"

#include "AdvancedStructureAssetSubsystem.generated.h"

UCLASS()
class ADVANCEDASSETSEDITORS_API UAdvancedStructureAssetSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void OpenAdvancedEditor(TArray<UObject*> ObjectsToEdit);
	void NotifyAdvancedEditorClosed(TArray<UObject*> ObjectsAreEditing);

	TMap<UObject*, UAdvancedStructureEditor*> OpenedEditorInstances;

	TSharedPtr<IToolkitHost> ToolKitHostRef;
};
