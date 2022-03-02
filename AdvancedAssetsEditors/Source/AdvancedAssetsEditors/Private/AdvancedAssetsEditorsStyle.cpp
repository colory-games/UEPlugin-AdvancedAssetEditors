#include "AdvancedAssetsEditorsStyle.h"

#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FAdvancedAssetsEditorsModule"

TSharedPtr<FSlateStyleSet> FAdvancedAssetsEditorsStyle::StyleInstance = nullptr;


void FAdvancedAssetsEditorsStyle::Initialize()
{
    if (!StyleInstance.IsValid())
    {
        StyleInstance = MakeShareable(new FSlateStyleSet(GetStyleSetName()));

        TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("AdvancedAssetsEditors");
        FString BaseDir = Plugin->GetBaseDir();
        StyleInstance->SetContentRoot(BaseDir / TEXT("Content"));

        FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
    }
}

void FAdvancedAssetsEditorsStyle::Shutdown()
{
    FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
    check(StyleInstance.IsUnique());
    StyleInstance.Reset();
}

void FAdvancedAssetsEditorsStyle::ReloadTextures()
{
    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
    }
}

const TSharedPtr<FSlateStyleSet> FAdvancedAssetsEditorsStyle::Get()
{
    return StyleInstance;
}

FName FAdvancedAssetsEditorsStyle::GetStyleSetName()
{
    static FName StyleSetName(TEXT("AdvancedAssetsEditorsStyle"));

    return StyleSetName;
}

#undef LOCTEXT_NAMESPACE
