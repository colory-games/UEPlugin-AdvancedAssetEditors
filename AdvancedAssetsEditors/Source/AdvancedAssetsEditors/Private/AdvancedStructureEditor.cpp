#include "AdvancedStructureEditor.h"

#include "AdvancedStructureAssetSubsystem.h"
#include "AdvancedStructureAssetsEditorToolkit.h"

void UAdvancedStructureEditor::Initialize(const TArray<UObject*>& InObjects)
{
	UAdvancedStructureAssetSubsystem* Subsystem = GEditor->GetEditorSubsystem<UAdvancedStructureAssetSubsystem>();

	ObjectsToEdit = InObjects;

	UAssetEditor::Initialize();
}

IAssetEditorInstance* UAdvancedStructureEditor::GetInstanceInterface()
{
	return ToolkitInstance;
}

void UAdvancedStructureEditor::GetObjectsToEdit(TArray<UObject*>& OutObjects)
{
	OutObjects.Append(ObjectsToEdit);
}

TSharedPtr<FBaseAssetToolkit> UAdvancedStructureEditor::CreateToolkit()
{
	TSharedPtr<FAdvancedStrcutureAssetsEditorToolkit> Toolkit = MakeShared<FAdvancedStrcutureAssetsEditorToolkit>(this);

	UUserDefinedStruct* UserDefinedStruct = Cast<UUserDefinedStruct>(ObjectsToEdit[0]);
	Toolkit->Initialize(UserDefinedStruct);

	return Toolkit;
}