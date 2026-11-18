#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "WanderAIController.generated.h"

/** Simple AI controller that repeatedly chooses reachable points around its pawn. */
UCLASS(Blueprintable)
class SURVIVALGAME_API AWanderAIController : public AAIController
{
	GENERATED_BODY()

public:
	AWanderAIController();

	/** Starts the random-point wander loop; safe to call from a Blueprint event graph. */
	UFUNCTION(BlueprintCallable, Category = "AI|Wander")
	void StartWandering();

	/** Stops movement and clears the pending wander timer. */
	UFUNCTION(BlueprintCallable, Category = "AI|Wander")
	void StopWandering();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	/** Maximum distance from the pawn used when finding a random reachable destination. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Wander", meta = (ClampMin = "100.0"))
	float WanderRadius = 1200.0f;

	/** The controller waits for a random interval after each move, matching the module example. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Wander", meta = (ClampMin = "0.0"))
	float MinimumPause = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Wander", meta = (ClampMin = "0.0"))
	float MaximumPause = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Wander", meta = (ClampMin = "1.0"))
	float AcceptanceRadius = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Wander")
	bool bStartWanderingOnPossess = true;

private:
	void MoveToRandomReachablePoint();
	void ScheduleNextWander();
	void ContinueWandering();

	FTimerHandle WanderTimerHandle;
	bool bIsWandering = false;
};
