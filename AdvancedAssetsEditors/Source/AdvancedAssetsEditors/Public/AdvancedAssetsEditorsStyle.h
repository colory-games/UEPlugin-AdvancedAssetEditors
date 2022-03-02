#pragma once

#include "Styling/SlateStyle.h"


class FAdvancedAssetsEditorsStyle
{
    static TSharedPtr<FSlateStyleSet> StyleInstance;

public:

    static void Initialize();

    static void Shutdown();

    static void ReloadTextures();

    static const TSharedPtr<FSlateStyleSet> Get();

    static FName GetStyleSetName();
};
