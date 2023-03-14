#include "AdvancedStructureAssetSubsystem.h"

#include "BlueprintEditorModule.h"

#include "AdvancedStructureAssetsEditorToolkit.h"
#include "AdvancedStructureEditor.h"

void UAdvancedStructureAssetSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
}

void UAdvancedStructureAssetSubsystem::Deinitialize()
{
}

void UAdvancedStructureAssetSubsystem::OpenAdvancedEditor(TArray<UObject*> ObjectsToEdit)
{
	// TODO: Handle more than 2 objects at once.
	if (ObjectsToEdit.Num() == 1)
	{
		UObject* Object = ObjectsToEdit[0];
		if (OpenedEditorInstances.Contains(Object))
		{
			OpenedEditorInstances[Object]->GetInstanceInterface()->FocusWindow(Object);
			return;
		}

		UUserDefinedStruct* UserDefinedStruct = Cast<UUserDefinedStruct>(Object);
		if (UserDefinedStruct == nullptr)
		{
			return;
		}

		UAdvancedStructureEditor* AdvancedStructureEditor = NewObject<UAdvancedStructureEditor>(this);
		AdvancedStructureEditor->Initialize(ObjectsToEdit);

		for (auto& O : ObjectsToEdit)
		{
			OpenedEditorInstances.Add(O, AdvancedStructureEditor);
		}
	}
}

void UAdvancedStructureAssetSubsystem::NotifyAdvancedEditorClosed(TArray<UObject*> ObjectsAreEditing)
{
	for (auto Object : ObjectsAreEditing)
	{
		OpenedEditorInstances.Remove(Object);
	}
}
