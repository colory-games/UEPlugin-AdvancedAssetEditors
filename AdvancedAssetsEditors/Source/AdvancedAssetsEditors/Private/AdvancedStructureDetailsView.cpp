#include "AdvancedStructureDetailsView.h"

#include "Kismet2/BlueprintEditorUtils.h"
#include "DetailLayoutBuilder.h"
#include "Common.h"
#include "DetailCategoryBuilder.h"


#define LOCTEXT_NAMESPACE "AdvancedAssetEditors"


void FAdvancedStructureDefaultsDetailCustomization::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
    // ref: /Engine/Source/Editor/Kismet/Private/UserDefinedStructureEditor.cpp

    if (!DefaultsNotification.IsValid())
    {
        return;
    }

    TSharedPtr<FStructOnScope> StructData = DefaultsNotification->GetStructData();

    if (DefaultsNotification->IsPropertyChangeComplete())
    {
        UStruct* OwnerStruct = PropertyChangedEvent.MemberProperty->GetOwnerStruct();

        if (ensure(OwnerStruct == UserDefinedStruct))
        {
            const FProperty* DirectProperty = PropertyChangedEvent.MemberProperty;
            while (DirectProperty && !DirectProperty->GetOwner<const UUserDefinedStruct>())
            {
                DirectProperty = DirectProperty->GetOwner<const FProperty>();
            }
            ensure(nullptr != DirectProperty);

            if (DirectProperty)
            {
                FString DefaultValueString;
                bool bDefaultValueSet = false;
                if (StructData.IsValid() && StructData->IsValid())
                {
                    bDefaultValueSet = FBlueprintEditorUtils::PropertyValueToString(DirectProperty, StructData->GetStructMemory(), DefaultValueString, OwnerStruct);
                }

                const FGuid Guid = FStructureEditorUtils::GetGuidForProperty(DirectProperty);
                if (bDefaultValueSet && Guid.IsValid())
                {
                    FStructureEditorUtils::ChangeVariableDefaultValue(UserDefinedStruct, Guid, DefaultValueString);
                }
            }
        }
    }
}

void FAdvancedStructureDefaultsDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
    const IDetailsView* View = DetailLayout.GetDetailsView();
    IDetailCategoryBuilder& StructureCategory = DetailLayout.EditCategory("Default Values", FText::GetEmpty(), ECategoryPriority::Important);

    const TArray<TWeakObjectPtr<UObject>> Objects = DetailLayout.GetDetailsView()->GetSelectedObjects();

    auto K2Schema = GetDefault<UEdGraphSchema_K2>();

    RegisteredEnumItemsList.Empty();

    for (auto Object : Objects)
    {
        if (!Object.IsValid()) {
            continue;
        }

        TSharedPtr<FStructOnScope> StructData = DefaultsNotification->GetStructData();

        View->OnFinishedChangingProperties().AddSP(this, &FAdvancedStructureDefaultsDetailCustomization::OnFinishedChangingProperties);

        for (TFieldIterator<FProperty> It(UserDefinedStruct); It; ++It)
        {
            FProperty* Property = *It;
            
            EEditableMenu Editable = GetEditableMenu(Property);

            IDetailPropertyRow* Row = StructureCategory.AddExternalStructureProperty(StructData, (*It)->GetFName());
            if (Row)
            {
                switch (Editable)
                {
                case EEditableMenu::EditAnywhere:
                case EEditableMenu::EditDefaultsOnly:
                    Row->Visibility(EVisibility::Visible);
                    Row->IsEnabled(true);
                    break;
                case EEditableMenu::EditInstanceOnly:
                case EEditableMenu::VisibleAnywhere:
                case EEditableMenu::VisibleDefaultsOnly:
                    Row->Visibility(EVisibility::Visible);
                    Row->IsEnabled(false);
                    break;
                case EEditableMenu::VisibleInstanceOnly:
                case EEditableMenu::NoAccess:
                    Row->Visibility(EVisibility::Hidden);
                    Row->IsEnabled(false);
                    break;
                }
            }
        }
    }
}

#undef LOCTEXT_NAMESPACE
