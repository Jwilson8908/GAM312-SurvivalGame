#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PlayerStatsWorldSubsystem.generated.h"

class APlayerController;
class UPlayerStatsWidget;

/** Creates the HUD and mirrors Health, Hunger, and Stamina from the local player's pawn. */
UCLASS()
class SURVIVALGAME_API UPlayerStatsWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

private:
	void EnsureHudExists();
	void RefreshHudValues();

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> LocalPlayerController;

	UPROPERTY(Transient)
	TObjectPtr<UPlayerStatsWidget> StatsWidget;
};
