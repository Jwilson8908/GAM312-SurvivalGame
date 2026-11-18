#include "PlayerStatsWorldSubsystem.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "PlayerStatsWidget.h"
#include "UObject/UnrealType.h"

namespace
{
	float ReadStatValue(const UObject* Object, const FName PropertyName)
	{
		if (!Object)
		{
			return 100.0f;
		}

		const FProperty* Property = Object->GetClass()->FindPropertyByName(PropertyName);
		const FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property);
		if (!NumericProperty)
		{
			return 100.0f;
		}

		// Blueprint stat variables may be floats or integers, so read either numeric type safely.
		if (NumericProperty->IsFloatingPoint())
		{
			const void* ValueAddress = NumericProperty->ContainerPtrToValuePtr<void>(Object);
			return static_cast<float>(NumericProperty->GetFloatingPointPropertyValue(ValueAddress));
		}
		return static_cast<float>(NumericProperty->GetSignedIntPropertyValue_InContainer(Object));
	}
}

void UPlayerStatsWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	EnsureHudExists();
}

void UPlayerStatsWorldSubsystem::Tick(float DeltaTime)
{
	EnsureHudExists();
	RefreshHudValues();
}

TStatId UPlayerStatsWorldSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UPlayerStatsWorldSubsystem, STATGROUP_Tickables);
}

void UPlayerStatsWorldSubsystem::EnsureHudExists()
{
	UWorld* World = GetWorld();
	if (!World || (World->WorldType != EWorldType::Game && World->WorldType != EWorldType::PIE))
	{
		return;
	}

	if (!IsValid(LocalPlayerController))
	{
		LocalPlayerController = World->GetFirstPlayerController();
	}
	if (!IsValid(LocalPlayerController) || !LocalPlayerController->IsLocalController())
	{
		return;
	}

	if (!IsValid(StatsWidget))
	{
		// Retry each tick until the local controller exists, then create the HUD only once.
		StatsWidget = CreateWidget<UPlayerStatsWidget>(LocalPlayerController, UPlayerStatsWidget::StaticClass());
		if (StatsWidget)
		{
			StatsWidget->AddToViewport(10);
		}
	}
}

void UPlayerStatsWorldSubsystem::RefreshHudValues()
{
	if (!IsValid(LocalPlayerController) || !IsValid(StatsWidget))
	{
		return;
	}

	const APawn* PlayerPawn = LocalPlayerController->GetPawn();
	if (!PlayerPawn)
	{
		return;
	}

	// These property names match the Health, Hunger, and Stamina variables in BP_SurvivalCharacter.
	StatsWidget->UpdateBars(
		ReadStatValue(PlayerPawn, TEXT("Health")),
		ReadStatValue(PlayerPawn, TEXT("Hunger")),
		ReadStatValue(PlayerPawn, TEXT("Stamina")));
}
