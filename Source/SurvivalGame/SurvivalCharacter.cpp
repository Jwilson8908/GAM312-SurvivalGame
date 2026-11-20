// Copyright notice belongs in the project settings.

#include "SurvivalCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputCoreTypes.h"

ASurvivalCharacter::ASurvivalCharacter()
{
	// Keep the view at a natural standing eye height and let mouse/controller
	// look input rotate the camera with the controller.
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(GetCapsuleComponent());
	PlayerCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	PlayerCamera->bUsePawnControlRotation = true;

	// The controller owns yaw so the existing Enhanced Input look mapping works.
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void ASurvivalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Add native Space-bar jump without replacing the Blueprint's Enhanced Input
	// mappings for movement, look, sprint, and interaction.
	PlayerInputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindKey(EKeys::SpaceBar, IE_Released, this, &ACharacter::StopJumping);
}
