#include "Common.h"

#include "Engine/UserDefinedStruct.h"
#include "Kismet2/StructureEditorUtils.h"

#define LOCTEXT_NAMESPACE "AdvancedAssetEditors"


static TArray<FString> EditableMenuStrings = {
    "EditAnywhere",
    "EditDefaultsOnly",
    "EditInstanceOnly",
    "VisibleAnywhere",
    "VisibleDefaultsOnly",
    "VisibleInstanceOnly",
    "NoAccess",
};

void SetEditableMenu(FProperty* Property, EEditableMenu Editable)
{
    EPropertyFlags ClearFlags = EPropertyFlags::CPF_Edit | EPropertyFlags::CPF_EditConst |
        EPropertyFlags::CPF_DisableEditOnInstance | EPropertyFlags::CPF_DisableEditOnTemplate;
    Property->ClearPropertyFlags(ClearFlags);

    if (Editable == EEditableMenu::NoAccess)
    {
        return;
    }

    EPropertyFlags NewFlags = CPF_None;
    switch (Editable)
    {
    case EEditableMenu::EditAnywhere:
        NewFlags = EPropertyFlags::CPF_Edit;
        break;
    case EEditableMenu::EditDefaultsOnly:
        NewFlags = EPropertyFlags::CPF_Edit | EPropertyFlags::CPF_DisableEditOnInstance;
        break;
    case EEditableMenu::EditInstanceOnly:
        NewFlags = EPropertyFlags::CPF_Edit | EPropertyFlags::CPF_DisableEditOnTemplate;
        break;
    case EEditableMenu::VisibleAnywhere:
        NewFlags = EPropertyFlags::CPF_Edit | EPropertyFlags::CPF_EditConst;
        break;
    case EEditableMenu::VisibleDefaultsOnly:
        NewFlags = EPropertyFlags::CPF_Edit | EPropertyFlags::CPF_EditConst | EPropertyFlags::CPF_DisableEditOnInstance;
        break;
    case EEditableMenu::VisibleInstanceOnly:
        NewFlags = EPropertyFlags::CPF_Edit | EPropertyFlags::CPF_EditConst | EPropertyFlags::CPF_DisableEditOnTemplate;
        break;
    }
    Property->SetPropertyFlags(NewFlags);
}

EEditableMenu GetEditableMenu(const FProperty* Property)
{
    EEditableMenu CurrentIndex = EEditableMenu::NoAccess;
    EPropertyFlags Flags = Property->GetPropertyFlags();

    if (Flags & EPropertyFlags::CPF_Edit)
    {
        if (Flags & EPropertyFlags::CPF_EditConst)
        {
            if (Flags & EPropertyFlags::CPF_DisableEditOnInstance)
            {
                CurrentIndex = EEditableMenu::VisibleDefaultsOnly;
            }
            else if (Flags & EPropertyFlags::CPF_DisableEditOnTemplate)
            {
                CurrentIndex = EEditableMenu::VisibleInstanceOnly;
            }
            else
            {
                CurrentIndex = EEditableMenu::VisibleAnywhere;
            }
        }
        else
        {
            if (Flags & EPropertyFlags::CPF_DisableEditOnInstance)
            {
                CurrentIndex = EEditableMenu::EditDefaultsOnly;
            }
            else if (Flags & EPropertyFlags::CPF_DisableEditOnTemplate)
            {
                CurrentIndex = EEditableMenu::EditInstanceOnly;
            }
            else
            {
                CurrentIndex = EEditableMenu::EditAnywhere;
            }
        }
    }

    return CurrentIndex;
}

FString GetEditableMenuString(const EEditableMenu EditableMenu)
{
    return EditableMenuStrings[EditableMenu];
}

FAdvancedStructureData GetAdvancedStructureData(const UUserDefinedStruct* UserDefinedStruct)
{
    FAdvancedStructureData Data;

    // IKismetCompilerInterface::CompileStructure clear the flags of EEditableMenu.
    // We will restore the flags by storing them manually.
    for (TFieldIterator<FProperty> It(UserDefinedStruct); It; ++It)
    {
        FProperty* Property = *It;
        EEditableMenu Editable = GetEditableMenu(Property);
        FGuid Guid = FStructureEditorUtils::GetGuidForProperty(Property);
        Data.PropertyAccessRights.Add(Guid, Editable);
    }

    return Data;
}

void RestoreAdvancedStructureData(const FAdvancedStructureData& Data, UUserDefinedStruct* UserDefinedStruct)
{
    // Restore the flags of EEditableMenu.
    for (TFieldIterator<FProperty> It(UserDefinedStruct); It; ++It)
    {
        FProperty* Property = *It;
        FGuid Guid = FStructureEditorUtils::GetGuidForProperty(Property);
        if (Data.PropertyAccessRights.Find(Guid) != nullptr)
        {
            EEditableMenu Editable = Data.PropertyAccessRights[Guid];
            SetEditableMenu(Property, Editable);
        }
    }
}

#undef LOCTEXT_NAMESPACE
