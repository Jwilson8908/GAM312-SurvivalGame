#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WanderAICharacter.generated.h"

/** Placeable sample AI pawn; add a Nav Mesh Bounds Volume for it to wander. */
UCLASS(Blueprintable)
class SURVIVALGAME_API AWanderAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AWanderAICharacter();
};
