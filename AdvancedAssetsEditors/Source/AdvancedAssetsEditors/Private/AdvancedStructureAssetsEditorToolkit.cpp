#include "AdvancedStructureAssetsEditorToolkit.h"

#include "EditorReimportHandler.h"
#include "SAdvancedStructureAssetsEditor.h"
#include "AdvancedStructureAssetSubsystem.h"
#include "Styling/ToolBarStyle.h"
#include "Widgets/Images/SLayeredImage.h"
#include "SPositiveActionButton.h"


#define LOCTEXT_NAMESPACE "AdvancedStructureAsset"

static const FName APP_ID("AdvancedStrcutureAssetsEditor");
static const FName TAB_ID("Advanced Structure Assets Editor");

TSharedRef<SDockTab> FAdvancedStrcutureAssetsEditorToolkit::HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier)
{
    TSharedPtr<SWidget> TabWidget = SNullWidget::NullWidget;

    if (TabIdentifier == TAB_ID)
    {
        UAdvancedStructureEditor* Editor = CastChecked<UAdvancedStructureEditor>(OwningAssetEditor);
        TArray<UObject*> UserDefinedStructures = Editor->ObjectsToEdit;
        UUserDefinedStruct* UDS = CastChecked<UUserDefinedStruct>(UserDefinedStructures[0]);

        TabWidget = SNew(SAdvancedStructureAssetsEditor, UDS);
    }

    return SNew(SDockTab)
        .TabRole(ETabRole::PanelTab)
        [
            TabWidget.ToSharedRef()
        ];
}

FReply FAdvancedStrcutureAssetsEditorToolkit::OnAddNewField()
{
    if (UserDefinedStruct != nullptr)
    {
        FEdGraphPinType InitialPinType;

        FStructureEditorUtils::AddVariable(UserDefinedStruct, InitialPinType);

        //InvokeTab(MemberVariablesTabId);
    }

    return FReply::Handled();
}

void FAdvancedStrcutureAssetsEditorToolkit::Initialize(UUserDefinedStruct* InUserDefinedStruct)
{
    UserDefinedStruct = InUserDefinedStruct;

    //TSharedPtr<FExtender> Extender = MakeShared<FExtender>();
    //Extender->AddToolBarExtension("Asset", EExtensionHook::After, GetToolkitCommands(),
    //    FToolBarExtensionDelegate::CreateSP(this, &FAdvancedStrcutureAssetsEditorToolkit::FillToolbar));
    //AddToolbarExtender(Extender);
}

const FSlateBrush* FAdvancedStrcutureAssetsEditorToolkit::OnGetStructureStatus() const
{
    if (UserDefinedStruct != nullptr)
    {
        switch (UserDefinedStruct->Status.GetValue())
        {
        case EUserDefinedStructureStatus::UDSS_Error:
            return FAppStyle::Get().GetBrush("Blueprint.CompileStatus.Overlay.Error");
        case EUserDefinedStructureStatus::UDSS_UpToDate:
            return FAppStyle::Get().GetBrush("Blueprint.CompileStatus.Overlay.Good");
        default:
            return FAppStyle::Get().GetBrush("Blueprint.CompileStatus.Overlay.Unknown");
        }
    }
    return nullptr;
}

FText FAdvancedStrcutureAssetsEditorToolkit::OnGetStatusTooltip() const
{
    if (UserDefinedStruct != nullptr)
    {
        switch (UserDefinedStruct->Status.GetValue())
        {
        case EUserDefinedStructureStatus::UDSS_Error:
            return FText::FromString(UserDefinedStruct->ErrorMessage);
        default:
            return LOCTEXT("GoodToGo_Status", "Good to go");
        }
    }
    return FText::GetEmpty();
}

void FAdvancedStrcutureAssetsEditorToolkit::FillToolbar(FToolBarBuilder& ToolbarBuilder)
{
    const FToolBarStyle& ToolBarStyle = ToolbarBuilder.GetStyleSet()->GetWidgetStyle<FToolBarStyle>(ToolbarBuilder.GetStyleName());

    ToolbarBuilder.BeginSection("UserDefinedStructure");

    TSharedPtr<SLayeredImage> CompileStatusImage;
    ToolbarBuilder.AddWidget(
        SNew(SBox)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        .Padding(ToolBarStyle.ButtonPadding)
        [
            SAssignNew(CompileStatusImage, SLayeredImage)
            .Image(FAppStyle::Get().GetBrush("Blueprint.CompileStatus.Background"))
            .ToolTipText(this, &FAdvancedStrcutureAssetsEditorToolkit::OnGetStatusTooltip)
        ]);
    CompileStatusImage->AddLayer(TAttribute<const FSlateBrush*>::CreateSP(this, &FAdvancedStrcutureAssetsEditorToolkit::OnGetStructureStatus));

    ToolbarBuilder.AddWidget(
        SNew(SBox)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Fill)
        .Padding(ToolBarStyle.ButtonPadding)
        [
            SNew(SPositiveActionButton)
            .Text(LOCTEXT("AddStructVariable", "Add Variable"))
            .ToolTipText(LOCTEXT("AddStructVariableToolTip", "Adds a new member variable to the end of this structure"))
            .OnClicked(this, &FAdvancedStrcutureAssetsEditorToolkit::OnAddNewField)
        ]);

    ToolbarBuilder.EndSection();
}

FAdvancedStrcutureAssetsEditorToolkit::FAdvancedStrcutureAssetsEditorToolkit(UAssetEditor* InOwningAssetEditor) : FBaseAssetToolkit(InOwningAssetEditor)
{
    StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_AdvancedStrcutureEditor")
        ->AddArea(
            FTabManager::NewPrimaryArea()
            ->SetOrientation(Orient_Horizontal)
            ->Split(
                FTabManager::NewStack()
                ->AddTab(TAB_ID, ETabState::OpenedTab)
                ->SetHideTabWell(true)
                ->SetSizeCoefficient(1.0f)
            )
        );
}

FAdvancedStrcutureAssetsEditorToolkit::~FAdvancedStrcutureAssetsEditorToolkit()
{
    UAdvancedStructureAssetSubsystem* Subsystem = GEditor->GetEditorSubsystem<UAdvancedStructureAssetSubsystem>();
    if (Subsystem)
    {
        TArray<UObject*> ObjectsToEdit;
        OwningAssetEditor->GetObjectsToEdit(ObjectsToEdit);
        Subsystem->NotifyAdvancedEditorClosed(ObjectsToEdit);
    }
}

FName FAdvancedStrcutureAssetsEditorToolkit::GetToolkitFName() const
{
    return FName("AdvancedStrcutureEditor");
}

FText FAdvancedStrcutureAssetsEditorToolkit::GetBaseToolkitName() const
{
    return LOCTEXT("AppLabel", "Advanced Structure Editor");
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
        LOCTEXT("WorkspaceMenu_AdvancedStrcutureEditor", "Advanced Strcutur Editor")
    );

    FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

    InTabManager->RegisterTabSpawner(TAB_ID, FOnSpawnTab::CreateSP(this, &FAdvancedStrcutureAssetsEditorToolkit::HandleTabManagerSpawnTab, TAB_ID))
        .SetDisplayName(LOCTEXT("AdvancedStructureEditorTabName", "Advanced Strcuture Editor"))
        .SetGroup(WorkspaceMenuCategory.ToSharedRef())
        .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));
}

void FAdvancedStrcutureAssetsEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
    FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

    InTabManager->UnregisterTabSpawner(TAB_ID);
}

#undef LOCTEXT_NAMESPACE