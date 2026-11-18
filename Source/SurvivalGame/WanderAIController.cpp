#include "WanderAIController.h"

#include "NavigationSystem.h"
#include "TimerManager.h"
#include "Navigation/PathFollowingComponent.h"

AWanderAIController::AWanderAIController()
{
	bWantsPlayerState = false;
}

void AWanderAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (bStartWanderingOnPossess)
	{
		StartWandering();
	}
}

void AWanderAIController::StartWandering()
{
	if (!GetPawn() || !GetWorld())
	{
		return;
	}

	bIsWandering = true;
	GetWorld()->GetTimerManager().ClearTimer(WanderTimerHandle);
	MoveToRandomReachablePoint();
}

void AWanderAIController::StopWandering()
{
	bIsWandering = false;
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(WanderTimerHandle);
	}
	StopMovement();
}

void AWanderAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	if (bIsWandering)
	{
		ScheduleNextWander();
	}
}

void AWanderAIController::MoveToRandomReachablePoint()
{
	APawn* ControlledPawn = GetPawn();
	UWorld* World = GetWorld();
	UNavigationSystemV1* Navigation = World ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
	if (!bIsWandering || !ControlledPawn || !Navigation)
	{
		ScheduleNextWander();
		return;
	}

	FNavLocation Destination;
	const FVector Origin = ControlledPawn->GetActorLocation();
	if (Navigation->GetRandomReachablePointInRadius(Origin, WanderRadius, Destination))
	{
		// MoveToLocation uses the navigation mesh and reports completion through OnMoveCompleted.
		MoveToLocation(Destination.Location, AcceptanceRadius);
	}
	else
	{
		// Keep retrying if the pawn is outside navigable space or no point is available yet.
		ScheduleNextWander();
	}
}

void AWanderAIController::ScheduleNextWander()
{
	if (!bIsWandering || !GetWorld())
	{
		return;
	}

	const float Pause = FMath::FRandRange(FMath::Min(MinimumPause, MaximumPause),
		FMath::Max(MinimumPause, MaximumPause));
	GetWorld()->GetTimerManager().SetTimer(WanderTimerHandle, this,
		&AWanderAIController::ContinueWandering, Pause, false);
}

void AWanderAIController::ContinueWandering()
{
	MoveToRandomReachablePoint();
}
