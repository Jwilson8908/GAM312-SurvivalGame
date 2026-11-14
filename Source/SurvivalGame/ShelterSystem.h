#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "ShelterSystem.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UTextBlock;
class UButton;

// Independent placement actor: an explicit box collider stays separate from the visual mesh.
UCLASS()
class SURVIVALGAME_API AShelterPiece : public AActor
{
    GENERATED_BODY()
public:
    AShelterPiece();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) UStaticMeshComponent* Mesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) UBoxComponent* Collision;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 Kind = 0;
    void Configure(int32 InKind, bool bPreview);
    void SetPreviewValid(bool bValid);
};

// The existing Blueprint character remains responsible for movement and harvesting.
// This actor reads/writes that character's Wood and Stone properties, avoiding a second inventory.
UCLASS()
class SURVIVALGAME_API AShelterSystem : public AActor
{
    GENERATED_BODY()
public:
    AShelterSystem();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    UFUNCTION(BlueprintCallable) void Craft(int32 Kind);
    UFUNCTION(BlueprintCallable) void SelectPiece(int32 Kind);
    UFUNCTION(BlueprintCallable) void CancelPlacement();
    UFUNCTION(BlueprintCallable) void ToggleMenu();
    // Runs isolated inventory and collision checks in the editor without changing the level.
    UFUNCTION(BlueprintCallable, Category="Verification") FString RunVerification();
    int32 Resource(FName Name) const;
    bool ChangeResource(FName Name, int32 Delta);
    bool CanCraft(int32 Kind) const;
    static int32 WoodCost(int32 Kind);
    static int32 StoneCost(int32 Kind);
    static FString PieceName(int32 Kind);
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory") TArray<int32> Supplies;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Building") bool bMenuOpen = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Building") bool bPlacementValid = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Building") FString Status;
    UPROPERTY() AShelterPiece* Preview;
    UPROPERTY() class UShelterWidget* Menu;
    UPROPERTY() APlayerController* PC;
private:
    void UpdatePreview();
    void Place();
    int32 SelectedKind = -1;
    float Yaw = 0;
    float HeightOffset = 0;
};

UCLASS()
class SURVIVALGAME_API UShelterButton : public UButton
{
    GENERATED_BODY()
public:
    UShelterButton() { InitIsFocusable(false); }
};

// A real UMG widget assembled in C++, with separate craft and place buttons like the tutorials.
UCLASS()
class SURVIVALGAME_API UShelterWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY() AShelterSystem* System;
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeTick(const FGeometry& Geometry, float DeltaSeconds) override;
    UFUNCTION() void CraftWall();
    UFUNCTION() void CraftFloor();
    UFUNCTION() void CraftCeiling();
    UFUNCTION() void PlaceWall();
    UFUNCTION() void PlaceFloor();
    UFUNCTION() void PlaceCeiling();
private:
    UPROPERTY() UTextBlock* Inventory;
    UPROPERTY() UTextBlock* Message;
    UPROPERTY() TArray<UButton*> CraftButtons;
    UPROPERTY() TArray<UButton*> PlaceButtons;
};
