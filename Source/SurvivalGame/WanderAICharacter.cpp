#include "WanderAICharacter.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "WanderAIController.h"

AWanderAICharacter::AWanderAICharacter()
{
	// Use a built-in sphere so this sample remains visible without adding an external model.
	UStaticMeshComponent* Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WanderVisual"));
	Visual->SetupAttachment(GetRootComponent());
	Visual->SetRelativeLocation(FVector(0.0f, 0.0f, -35.0f));
	Visual->SetRelativeScale3D(FVector(0.45f));
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Visual->SetStaticMesh(SphereMesh.Object);
	}

	AIControllerClass = AWanderAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}
