#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerStatsWidget.generated.h"

class UProgressBar;
class UTextBlock;

/** Native HUD widget that displays the character's three survival stats. */
UCLASS(BlueprintType, Blueprintable)
class SURVIVALGAME_API UPlayerStatsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Updates each bar and its readable current-value label. Values use a 0-100 scale. */
	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void UpdateBars(float Health, float Hunger, float Stamina);

protected:
	virtual void NativeOnInitialized() override;

private:
	// These widgets are built in C++ so the HUD works without hand-wiring bindings.
	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> HungerBar;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> StaminaBar;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HealthLabel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HungerLabel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StaminaLabel;
};
