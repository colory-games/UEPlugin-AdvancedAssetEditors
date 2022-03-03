#include "AdvancedStrcutureAssetsEditorToolkit.h"

#include "EditorReimportHandler.h"
#include "SAdvancedStructureAssetsEditor.h"

#define LOCTEXT_NAMESPACE "FAdvancedAssetsEditorsModule"

static const FName APP_ID("AdvancedStrcutureAssetsEditor");
static const FName TAB_ID("Advanced Structure Assets Editor");

TSharedRef<SDockTab> FAdvancedStrcutureAssetsEditorToolkit::HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier)
{
    TSharedPtr<SWidget> TabWidget = SNullWidget::NullWidget;

    if (TabIdentifier == TAB_ID)
    {
        TabWidget = SNew(SAdvancedStructureAssetsEditor, UserDefinedStruct, Style);
    }

    return SNew(SDockTab)
        .TabRole(ETabRole::PanelTab)
        [
            TabWidget.ToSharedRef()
        ];
}

FAdvancedStrcutureAssetsEditorToolkit::FAdvancedStrcutureAssetsEditorToolkit(const TSharedRef<ISlateStyle>& InStyle)
    : UserDefinedStruct(nullptr), Style(InStyle)
{
}

FAdvancedStrcutureAssetsEditorToolkit::~FAdvancedStrcutureAssetsEditorToolkit()
{
    FReimportManager::Instance()->OnPreReimport().RemoveAll(this);
    FReimportManager::Instance()->OnPostReimport().RemoveAll(this);
}

void FAdvancedStrcutureAssetsEditorToolkit::Initialize(UUserDefinedStruct* InCustomStructAsset,
                                                       const EToolkitMode::Type InMode,
                                                       const TSharedPtr<IToolkitHost>& InToolkitHost)
{
    UserDefinedStruct = InCustomStructAsset;

    // TODO: Support Undo/Redo

    const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("Standalone_AdvancedStrcutureAssetsEditor")
        ->AddArea
        (
            FTabManager::NewPrimaryArea()
                ->SetOrientation(Orient_Horizontal)
                ->Split
                (
                    FTabManager::NewSplitter()
                        ->SetOrientation(Orient_Vertical)
                        ->SetSizeCoefficient(0.66f)
                        ->Split
                        (
                            FTabManager::NewStack()
                                ->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
                                ->SetHideTabWell(true)
                                ->SetSizeCoefficient(0.1f)
                        )
                        ->Split
                        (
                            FTabManager::NewStack()
                                ->AddTab(TAB_ID, ETabState::OpenedTab)
                                ->SetHideTabWell(true)
                                ->SetSizeCoefficient(0.9f)
                        )
                )
        );

    FAssetEditorToolkit::InitAssetEditor(
        InMode,
        InToolkitHost,
        APP_ID,
        Layout,
        true,
        true,
        UserDefinedStruct
    );

    RegenerateMenusAndToolbars();
}

FName FAdvancedStrcutureAssetsEditorToolkit::GetToolkitFName() const
{
    return FName("AdvancedStrcutureAssetsEditor");
}

FText FAdvancedStrcutureAssetsEditorToolkit::GetBaseToolkitName() const
{
    return LOCTEXT("AppLabel", "Advanced Structure Assets Editor");
}

FLinearColor FAdvancedStrcutureAssetsEditorToolkit::GetWorldCentricTabColorScale() const
{
    return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}

FString FAdvancedStrcutureAssetsEditorToolkit::GetWorldCentricTabPrefix() const
{
    return LOCTEXT("WorldCentricTabPrefix", "Advanced Structure ").ToString();
}

FString FAdvancedStrcutureAssetsEditorToolkit::GetDocumentationLink() const
{
    return FString(TEXT("https://github.com/colory-games/Advanced-Assets-Editors"));
}

void FAdvancedStrcutureAssetsEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
    WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(
        LOCTEXT("WorkspaceMenu_AdvancedStrcutureAssetsEditor", "Advanced Strcuture Assets Editor")
    );

    FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

    InTabManager->RegisterTabSpawner(TAB_ID, FOnSpawnTab::CreateSP(this, &FAdvancedStrcutureAssetsEditorToolkit::HandleTabManagerSpawnTab, TAB_ID))
        .SetDisplayName(LOCTEXT("AdvancedStructureAssetsEditorTabName", "Advanced Strcuture Assets Editor"))
        .SetGroup(WorkspaceMenuCategory.ToSharedRef())
        .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));
}

void FAdvancedStrcutureAssetsEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
    FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

    InTabManager->UnregisterTabSpawner(TAB_ID);
}

#undef LOCTEXT_NAMESPACE