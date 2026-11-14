#include "BuildingPart.h"

ABuildingPart::ABuildingPart()
{
    PrimaryActorTick.bCanEverTick = true;

    PivotComponent = CreateDefaultSubobject<USceneComponent>(
        TEXT("PivotComponent")
    );
    RootComponent = PivotComponent;
}

void ABuildingPart::BeginPlay()
{
    Super::BeginPlay();
}

void ABuildingPart::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
