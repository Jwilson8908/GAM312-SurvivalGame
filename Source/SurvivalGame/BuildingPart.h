#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "BuildingPart.generated.h"

UCLASS()
class SURVIVALGAME_API ABuildingPart : public AActor
{
    GENERATED_BODY()

public:
    ABuildingPart();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
    USceneComponent* PivotComponent;

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
};
