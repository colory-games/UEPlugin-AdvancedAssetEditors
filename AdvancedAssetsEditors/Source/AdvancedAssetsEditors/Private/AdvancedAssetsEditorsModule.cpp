#include "Templates/SharedPointer.h"

#include "AdvancedStructureAssetsActions.h"


class FAdvancedAssetsEditorsModule : public IModuleInterface
{
    TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;

public:
    virtual void StartupModule() override
    {
        // Register Actions
        IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
        TSharedRef<IAssetTypeActions> Actions = MakeShareable(new FAdvancedStructureAssetsActions());
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
    }

    virtual bool SupportsDynamicReloading() override
    {
        return true;
    }
};


IMPLEMENT_MODULE(FAdvancedAssetsEditorsModule, AdvancedAssetsEditors);

