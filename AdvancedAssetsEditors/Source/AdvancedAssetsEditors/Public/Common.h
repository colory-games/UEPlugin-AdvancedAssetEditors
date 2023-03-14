// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/UserDefinedStruct.h"


enum EPropertyPositionFlag
{
    First = 0x01,
    Last = 0x02,
};

enum EEditableMenu
{
    EditAnywhere,
    EditDefaultsOnly,
    EditInstanceOnly,
    VisibleAnywhere,
    VisibleDefaultsOnly,
    VisibleInstanceOnly,
    NoAccess,
    EEditableMenu_MAX
};

struct FAdvancedStructureData
{
    TMap<FGuid, EEditableMenu> PropertyAccessRights;
};

typedef TArray<TSharedPtr<uint8>> EnumItems;
typedef TArray<TSharedPtr<EnumItems>> EnumItemsList;

void SetEditableMenu(FProperty* Property, EEditableMenu Editable);
EEditableMenu GetEditableMenu(const FProperty* Property);
FString GetEditableMenuString(const EEditableMenu EditableMenu);
FAdvancedStructureData GetAdvancedStructureData(const UUserDefinedStruct* UserDefinedStruct);
void RestoreAdvancedStructureData(const FAdvancedStructureData& Data, UUserDefinedStruct* UserDefinedStruct);
