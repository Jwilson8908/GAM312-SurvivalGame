#include "PlayerStatsWidget.h"

#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Blueprint/WidgetTree.h"

namespace
{
	constexpr float StatMaximum = 100.0f;

	UProgressBar* AddStatRow(UWidgetTree* WidgetTree, UVerticalBox* Parent,
		const TCHAR* Name, const TCHAR* InitialText, const FLinearColor& FillColor,
		TObjectPtr<UTextBlock>& OutLabel)
	{
		UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),
			FName(*FString::Printf(TEXT("%sLabel"), Name)));
		Label->SetText(FText::FromString(InitialText));
		Label->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		UVerticalBoxSlot* LabelSlot = Parent->AddChildToVerticalBox(Label);
		LabelSlot->SetPadding(FMargin(0.0f, 3.0f, 0.0f, 2.0f));

		UProgressBar* Bar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), FName(Name));
		Bar->SetPercent(1.0f);
		Bar->SetFillColorAndOpacity(FillColor);
		UVerticalBoxSlot* BarSlot = Parent->AddChildToVerticalBox(Bar);
		BarSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));

		OutLabel = Label;
		return Bar;
	}

	void SetStat(UProgressBar* Bar, UTextBlock* Label, const TCHAR* StatName, float Value)
	{
		const float ClampedValue = FMath::Clamp(Value, 0.0f, StatMaximum);
		if (Bar)
		{
			Bar->SetPercent(ClampedValue / StatMaximum);
		}
		if (Label)
		{
			Label->SetText(FText::FromString(FString::Printf(TEXT("%s  %.0f / %.0f"),
				StatName, ClampedValue, StatMaximum)));
		}
	}
}

void UPlayerStatsWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!WidgetTree)
	{
		return;
	}

	// Build a compact lower-left HUD panel at runtime, avoiding a required Widget Blueprint asset.
	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("StatsPanel"));
	Panel->SetPadding(FMargin(14.0f));
	Panel->SetBrushColor(FLinearColor(0.015f, 0.02f, 0.025f, 0.82f));
	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(Panel);
	PanelSlot->SetAnchors(FAnchors(0.0f, 1.0f));
	PanelSlot->SetAlignment(FVector2D(0.0f, 1.0f));
	PanelSlot->SetPosition(FVector2D(24.0f, -24.0f));
	PanelSlot->SetAutoSize(true);

	UVerticalBox* Rows = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StatRows"));
	Panel->SetContent(Rows);

	HealthBar = AddStatRow(WidgetTree, Rows, TEXT("HealthBar"), TEXT("Health  100 / 100"),
		FLinearColor(0.85f, 0.12f, 0.12f), HealthLabel);
	HungerBar = AddStatRow(WidgetTree, Rows, TEXT("HungerBar"), TEXT("Hunger  100 / 100"),
		FLinearColor(0.72f, 0.43f, 0.12f), HungerLabel);
	StaminaBar = AddStatRow(WidgetTree, Rows, TEXT("StaminaBar"), TEXT("Stamina  100 / 100"),
		FLinearColor(0.16f, 0.68f, 0.24f), StaminaLabel);
}

void UPlayerStatsWidget::UpdateBars(float Health, float Hunger, float Stamina)
{
	SetStat(HealthBar, HealthLabel, TEXT("Health"), Health);
	SetStat(HungerBar, HungerLabel, TEXT("Hunger"), Hunger);
	SetStat(StaminaBar, StaminaLabel, TEXT("Stamina"), Stamina);
}
