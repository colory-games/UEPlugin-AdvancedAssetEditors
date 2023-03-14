#include "AdvancedStructureNotification.h"

#include "Common.h"

#define LOCTEXT_NAMESPACE "AdvancedAssetEditors"


void FAdvancedStructureDefaultsNotification::Initialize()
{
    StructData = MakeShareable(new FStructOnScope(UserDefinedStruct));
    UserDefinedStruct->InitializeDefaultValue(StructData->GetStructMemory());
    StructData->SetPackage(UserDefinedStruct->GetOutermost());
}

UUserDefinedStruct* FAdvancedStructureDefaultsNotification::GetUserDefinedStruct()
{
    return UserDefinedStruct;
}

TSharedPtr<class SWidget> FAdvancedStructureDefaultsNotification::GetView()
{
    return DetailsView;
}

void FAdvancedStructureDefaultsNotification::SetView(TSharedPtr<IDetailsView> View)
{
    DetailsView = View;
}

void FAdvancedStructureDefaultsNotification::PreChange(const UUserDefinedStruct* InUserDefinedStruct, FStructureEditorUtils::EStructureEditorChangeInfo Info)
{
    for (TFieldIterator<FProperty> It(InUserDefinedStruct); It; ++It)
    {
        FProperty* Property = *It;

        EEditableMenu Editable = GetEditableMenu(Property);
        FString S = GetEditableMenuString(Editable);
        UE_LOG(LogTemp, Error, TEXT("pre change = %s"), *S);
    }

    if (Info != FStructureEditorUtils::DefaultValueChanged)
    {
        StructData->Destroy();
        DetailsView->SetObject(nullptr);
        DetailsView->OnFinishedChangingProperties().Clear();
    }
}

void FAdvancedStructureDefaultsNotification::PostChange(const UUserDefinedStruct* InUserDefinedStruct, FStructureEditorUtils::EStructureEditorChangeInfo Info)
{
    for (TFieldIterator<FProperty> It(InUserDefinedStruct); It; ++It)
    {
        FProperty* Property = *It;

        EEditableMenu Editable = GetEditableMenu(Property);
        FString S = GetEditableMenuString(Editable);
        UE_LOG(LogTemp, Error, TEXT("post change = %s"), *S);
    }

    if (Info != FStructureEditorUtils::DefaultValueChanged)
    {
        StructData->Initialize(UserDefinedStruct);
        DetailsView->SetObject(UserDefinedStruct, true);
    }
    UserDefinedStruct->InitializeDefaultValue(StructData->GetStructMemory());
}

void FAdvancedStructureDefaultsNotification::NotifyPreChange(FProperty* PropertyAboutToChange)
{
    ++PropertyChangeRecursionGuard;
}

void FAdvancedStructureDefaultsNotification::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged)
{
    --PropertyChangeRecursionGuard;
}

bool FAdvancedStructureDefaultsNotification::IsPropertyChangeComplete()
{
    return PropertyChangeRecursionGuard == 0;
}

TSharedPtr<FStructOnScope> FAdvancedStructureDefaultsNotification::GetStructData() const
{
    return StructData;
}


#undef LOCTEXT_NAMESPACE
