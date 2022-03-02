#include "Templates/SharedPointer.h"

#include "AdvancedAssetsEditorsStyle.h"
#include "AdvancedStructureAssetsActions.h"

#define LOCTEXT_NAMESPACE "FAdvancedAssetsEditorsModule"


class FAdvancedAssetsEditorsModule : public IModuleInterface
{
    TSharedPtr<ISlateStyle> Style;
    TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;

public:
    virtual void StartupModule() override
    {
        FAdvancedAssetsEditorsStyle::Initialize();
        FAdvancedAssetsEditorsStyle::ReloadTextures();

        // Register Actions
        IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
        TSharedRef<IAssetTypeActions> Actions = MakeShareable(
            new FAdvancedStructureAssetsActions(FAdvancedAssetsEditorsStyle::Get().ToSharedRef()));
        AssetTools.RegisterAssetTypeActions(Actions);
        RegisteredAssetTypeActions.Add(Actions);
    }

    virtual void ShutdownModule() override
    {
        // Unregister Actions
        FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools");
        if (AssetToolsModule != nullptr)
        {
            IAssetTools& AssetTools = AssetToolsModule->Get();
            for (auto Actions : RegisteredAssetTypeActions)
            {
                AssetTools.UnregisterAssetTypeActions(Actions);
            }
        }
        RegisteredAssetTypeActions.Empty();

        FAdvancedAssetsEditorsStyle::Shutdown();
    }

    virtual bool SupportsDynamicReloading() override
    {
        return true;
    }
};

#undef LOCTEXT_NAMESPACE


IMPLEMENT_MODULE(FAdvancedAssetsEditorsModule, AdvancedAssetsEditors);

