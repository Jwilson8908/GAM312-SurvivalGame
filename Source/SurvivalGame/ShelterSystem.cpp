#include "ShelterSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UnrealType.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "InputCoreTypes.h"

namespace
{
    // Dimensions in centimetres; every tile spans 400 cm and each wall is 300 cm high.
    FVector Dimensions(int32 Kind) { return Kind == 0 ? FVector(20,400,300) : FVector(400,400,20); }
    FLinearColor WoodTint() { return FLinearColor(0.40f,0.20f,0.07f); }
}

AShelterPiece::AShelterPiece()
{
    // Collision remains available even while Unreal compiles the visual mesh asynchronously.
    Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("SimpleCollision"));
    SetRootComponent(Collision);
    Collision->SetMobility(EComponentMobility::Movable);
    Collision->SetCollisionProfileName(TEXT("BlockAll"));
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShelterMesh"));
    Mesh->SetupAttachment(Collision);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Mesh->SetStaticMesh(Cube.Object);
    Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AShelterPiece::Configure(int32 InKind, bool bPreview)
{
    Kind = InKind;
    Mesh->SetWorldScale3D(Dimensions(Kind) / 100.f);
    Collision->SetBoxExtent(Dimensions(Kind)*0.5f);
    Collision->SetCollisionEnabled(bPreview ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
    Mesh->SetCastShadow(!bPreview);
    if (UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/ModuleThree/M_Shelter.M_Shelter")))
    {
        Mesh->SetMaterial(0,Material);
        if (UMaterialInstanceDynamic* MID = Mesh->CreateAndSetMaterialInstanceDynamic(0))
            MID->SetVectorParameterValue(TEXT("Tint"), bPreview ? FLinearColor::Green : WoodTint());
    }
}

void AShelterPiece::SetPreviewValid(bool bValid)
{
    if (auto* MID = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(0)))
        MID->SetVectorParameterValue(TEXT("Tint"), bValid ? FLinearColor(0.06f,0.8f,0.35f) : FLinearColor(1.f,0.07f,0.04f));
}

AShelterSystem::AShelterSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    Supplies.Init(0,3);
    Status = TEXT("F: crafting menu | E: harvest | Build a shelter from collected resources");
}
void AShelterSystem::BeginPlay()
{
    Super::BeginPlay();
    PC = UGameplayStatics::GetPlayerController(this,0);
    if (!PC) return;
    Menu = CreateWidget<UShelterWidget>(PC,UShelterWidget::StaticClass());
    Menu->System = this;
    Menu->AddToViewport(20);
    Menu->SetVisibility(ESlateVisibility::Collapsed);
}
int32 AShelterSystem::WoodCost(int32 Kind) { const int32 Costs[] = {30,15,50}; return Kind>=0 && Kind<3 ? Costs[Kind] : MAX_int32; }
int32 AShelterSystem::StoneCost(int32 Kind) { const int32 Costs[] = {15,5,30}; return Kind>=0 && Kind<3 ? Costs[Kind] : MAX_int32; }
FString AShelterSystem::PieceName(int32 Kind) { return Kind==0 ? TEXT("Wall") : Kind==1 ? TEXT("Floor") : TEXT("Ceiling"); }
int32 AShelterSystem::Resource(FName Name) const
{
    // Reflection bridges the existing Blueprint inventory without renaming its variables.
    APawn* Pawn = PC ? PC->GetPawn() : nullptr;
    if (!Pawn) return 0;
    if (FIntProperty* P = FindFProperty<FIntProperty>(Pawn->GetClass(),Name))
        return P->GetPropertyValue_InContainer(Pawn);
    return 0;
}
bool AShelterSystem::ChangeResource(FName Name, int32 Delta)
{
    APawn* Pawn = PC ? PC->GetPawn() : nullptr;
    if (!Pawn) return false;
    if (FIntProperty* P = FindFProperty<FIntProperty>(Pawn->GetClass(),Name))
    {
        const int32 NewValue = P->GetPropertyValue_InContainer(Pawn)+Delta;
        if (NewValue<0) return false;
        P->SetPropertyValue_InContainer(Pawn,NewValue);
        return true;
    }
    return false;
}
bool AShelterSystem::CanCraft(int32 Kind) const
{
    return Kind>=0 && Kind<3 && Resource(TEXT("Wood"))>=WoodCost(Kind) && Resource(TEXT("Stone"))>=StoneCost(Kind);
}
void AShelterSystem::Craft(int32 Kind)
{
    if (!CanCraft(Kind)) { Status=TEXT("Not enough resources. Collect more wood and stone."); return; }
    // Only charge when both requirements are met. Roll back wood if the second write fails.
    if (!ChangeResource(TEXT("Wood"),-WoodCost(Kind))) return;
    if (!ChangeResource(TEXT("Stone"),-StoneCost(Kind))) { ChangeResource(TEXT("Wood"),WoodCost(Kind)); return; }
    ++Supplies[Kind];
    Status=FString::Printf(TEXT("Crafted %s. Select PLACE to preview it."),*PieceName(Kind));
}
void AShelterSystem::ToggleMenu()
{
    if (!PC || !Menu) return;
    bMenuOpen=!bMenuOpen;
    Menu->SetVisibility(bMenuOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    PC->bShowMouseCursor=bMenuOpen;
    PC->SetIgnoreLookInput(bMenuOpen);
    PC->SetIgnoreMoveInput(bMenuOpen);
    if (bMenuOpen) { FInputModeGameAndUI Mode; Mode.SetHideCursorDuringCapture(false); PC->SetInputMode(Mode); }
    else { PC->SetInputMode(FInputModeGameOnly()); }
}
void AShelterSystem::SelectPiece(int32 Kind)
{
    if (!Supplies.IsValidIndex(Kind) || Supplies[Kind]<1) { Status=TEXT("Craft this item first."); return; }
    CancelPlacement();
    SelectedKind=Kind; HeightOffset=0; Yaw=0;
    Preview=GetWorld()->SpawnActor<AShelterPiece>();
    if (!Preview) { SelectedKind=-1; return; }
    Preview->Configure(Kind,true);
    if (bMenuOpen) ToggleMenu();
    Status=TEXT("Left click: place | R: rotate 90 degrees | Page Up/Down: height | Right click: cancel");
    UpdatePreview();
}
void AShelterSystem::CancelPlacement()
{
    // Previewing does not spend a crafted item, so cancellation never loses inventory.
    if (IsValid(Preview)) Preview->Destroy();
    Preview=nullptr; SelectedKind=-1; bPlacementValid=false;
}
void AShelterSystem::UpdatePreview()
{
    if (!PC || !IsValid(Preview)) return;
    FVector Start; FRotator View; PC->GetPlayerViewPoint(Start,View);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(ShelterPlacement),false);
    Params.AddIgnoredActor(PC->GetPawn()); Params.AddIgnoredActor(Preview);
    FHitResult Hit;
    // A camera visibility trace finds the real surface; previews never collide with this trace.
    const bool bHit=GetWorld()->LineTraceSingleByChannel(Hit,Start,Start+View.Vector()*1200.f,ECC_Visibility,Params);
    FVector Position=bHit ? Hit.ImpactPoint : Start+View.Vector()*600.f;
    const FVector Size=Dimensions(SelectedKind);
    if (bHit)
    {
        const float Grid = SelectedKind==0 ? 200.f : 400.f;
        Position.X=FMath::GridSnap(Position.X,Grid);
        Position.Y=FMath::GridSnap(Position.Y,Grid);
        // Place horizontal tiles above top surfaces or below the underside of a ceiling.
        Position.Z += (Hit.ImpactNormal.Z < -0.5f ? -1.f : 1.f)*Size.Z*0.5f;
        if (SelectedKind==2)
        {
            const auto* Support=Cast<AShelterPiece>(Hit.GetActor());
            if (!Support || Support->Kind==1) Position.Z+=300.f;
            else if (Support->Kind==0) Position.Z=Support->GetActorLocation().Z+160.f;
        }
    }
    Position.Z+=HeightOffset;
    Preview->SetActorLocationAndRotation(Position,FRotator(0,Yaw,0));
    // Allow flush faces and the 10 cm corner joint of perpendicular walls.
    // The committed meshes retain their full simple collision dimensions.
    const FVector Margin=SelectedKind==0 ? FVector(1.f,11.f,1.f) : FVector(1.f);
    const FVector Extent=Size*0.5f-Margin;
    FCollisionObjectQueryParams Objects; Objects.AddObjectTypesToQuery(ECC_WorldStatic); Objects.AddObjectTypesToQuery(ECC_WorldDynamic); Objects.AddObjectTypesToQuery(ECC_Pawn);
    bPlacementValid=bHit && !GetWorld()->OverlapAnyTestByObjectType(Position,FRotator(0,Yaw,0).Quaternion(),Objects,FCollisionShape::MakeBox(Extent),Params);
    // Reject construction on resource nodes, which would float after the node is harvested.
    if (bHit && Hit.GetActor() && Hit.GetActor()->GetClass()->GetName().Contains(TEXT("BP_Resource"))) bPlacementValid=false;
    Preview->SetPreviewValid(bPlacementValid);
}
void AShelterSystem::Place()
{
    UpdatePreview();
    if (!IsValid(Preview) || !bPlacementValid || !Supplies.IsValidIndex(SelectedKind) || Supplies[SelectedKind]<1)
    { Status=TEXT("Invalid placement. Aim at an open surface; red means blocked."); return; }
    // Commit the preview, enable simple collision, and consume exactly one crafted item.
    Preview->Configure(SelectedKind,false);
    --Supplies[SelectedKind];
    Status=FString::Printf(TEXT("Placed %s. Press F to craft or place another."),*PieceName(SelectedKind));
    Preview=nullptr; SelectedKind=-1; bPlacementValid=false;
}
void AShelterSystem::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!PC) return;
    if (PC->WasInputKeyJustPressed(EKeys::F)) ToggleMenu();
    if (bMenuOpen) return;
    if (IsValid(Preview))
    {
        if (PC->WasInputKeyJustPressed(EKeys::R)) Yaw+=90.f;
        if (PC->WasInputKeyJustPressed(EKeys::PageUp)) HeightOffset+=20.f;
        if (PC->WasInputKeyJustPressed(EKeys::PageDown)) HeightOffset-=20.f;
        UpdatePreview();
        if (PC->WasInputKeyJustPressed(EKeys::RightMouseButton)) CancelPlacement();
        else if (PC->WasInputKeyJustPressed(EKeys::LeftMouseButton)) Place();
    }
}
void AShelterSystem::EndPlay(const EEndPlayReason::Type Reason)
{
    CancelPlacement();
    if (Menu) Menu->RemoveFromParent();
    if (PC && bMenuOpen) { PC->bShowMouseCursor=false; PC->SetIgnoreLookInput(false); PC->SetIgnoreMoveInput(false); PC->SetInputMode(FInputModeGameOnly()); }
    Super::EndPlay(Reason);
}

TSharedRef<SWidget> UShelterWidget::RebuildWidget()
{
    if (!WidgetTree) WidgetTree=NewObject<UWidgetTree>(this,TEXT("WidgetTree"));
    auto* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>(); WidgetTree->RootWidget=Canvas;
    auto* Border=WidgetTree->ConstructWidget<UBorder>();
    Border->SetBrushColor(FLinearColor(0.018f,0.03f,0.045f,0.98f)); Border->SetPadding(FMargin(30));
    auto* Slot=Canvas->AddChildToCanvas(Border); Slot->SetAnchors(FAnchors(0.5f,0.5f)); Slot->SetAlignment(FVector2D(0.5f)); Slot->SetSize(FVector2D(880,560));
    auto* Column=WidgetTree->ConstructWidget<UVerticalBox>(); Border->SetContent(Column);
    auto AddText=[&](const FString& Value,int32 Size,FLinearColor Color)
    {
        auto* Text=WidgetTree->ConstructWidget<UTextBlock>(); Text->SetText(FText::FromString(Value));
        FSlateFontInfo Font=Text->GetFont(); Font.Size=Size; Text->SetFont(Font); Text->SetColorAndOpacity(FSlateColor(Color));
        auto* S=Column->AddChildToVerticalBox(Text); S->SetPadding(FMargin(0,0,0,16)); return Text;
    };
    AddText(TEXT("SHELTER / CRAFTING"),30,FLinearColor(0.2f,0.9f,0.65f));
    Inventory=AddText(TEXT("Inventory"),21,FLinearColor::White);
    AddText(TEXT("RECIPES AND RESOURCE COSTS"),15,FLinearColor(0.55f,0.65f,0.7f));
    for (int32 Kind=0;Kind<3;++Kind)
    {
        auto* Row=WidgetTree->ConstructWidget<UHorizontalBox>();
        Column->AddChildToVerticalBox(Row)->SetPadding(FMargin(0,5));
        auto* Label=WidgetTree->ConstructWidget<UTextBlock>();
        Label->SetText(FText::FromString(FString::Printf(TEXT("%-10s     %d Wood + %d Stone"),*AShelterSystem::PieceName(Kind),AShelterSystem::WoodCost(Kind),AShelterSystem::StoneCost(Kind))));
        auto* L=Row->AddChildToHorizontalBox(Label); L->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); L->SetVerticalAlignment(VAlign_Center);
        for (int32 Action=0;Action<2;++Action)
        {
            auto* Button=WidgetTree->ConstructWidget<UShelterButton>();
            auto* Text=WidgetTree->ConstructWidget<UTextBlock>(); Text->SetText(FText::FromString(Action==0 ? TEXT("CRAFT") : TEXT("PLACE"))); Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.02f,0.03f,0.04f)));
            Button->SetContent(Text); Button->SetBackgroundColor(Action==0 ? FLinearColor(0.2f,0.9f,0.65f) : FLinearColor(0.65f,0.8f,0.95f));
            Row->AddChildToHorizontalBox(Button)->SetPadding(FMargin(8,6));
            if (Action==0) CraftButtons.Add(Button); else PlaceButtons.Add(Button);
        }
    }
    CraftButtons[0]->OnClicked.AddDynamic(this,&UShelterWidget::CraftWall);
    CraftButtons[1]->OnClicked.AddDynamic(this,&UShelterWidget::CraftFloor);
    CraftButtons[2]->OnClicked.AddDynamic(this,&UShelterWidget::CraftCeiling);
    PlaceButtons[0]->OnClicked.AddDynamic(this,&UShelterWidget::PlaceWall);
    PlaceButtons[1]->OnClicked.AddDynamic(this,&UShelterWidget::PlaceFloor);
    PlaceButtons[2]->OnClicked.AddDynamic(this,&UShelterWidget::PlaceCeiling);
    Message=AddText(TEXT(""),16,FLinearColor(1.f,0.8f,0.4f)); Message->SetAutoWrapText(true);
    AddText(TEXT("F  close menu    |    R  rotate    |    Left click  place\nPage Up / Down  adjust height    |    Right click  cancel"),16,FLinearColor(0.6f,0.7f,0.75f));
    return Super::RebuildWidget();
}
void UShelterWidget::NativeTick(const FGeometry& Geometry,float DeltaSeconds)
{
    Super::NativeTick(Geometry,DeltaSeconds);
    if (!System || !Inventory || !Message) return;
    // Read the same values harvesting changes, so collecting and spending appear immediately.
    Inventory->SetText(FText::FromString(FString::Printf(TEXT("WOOD %d    STONE %d\nREADY TO PLACE:  Wall %d   Floor %d   Ceiling %d"),System->Resource(TEXT("Wood")),System->Resource(TEXT("Stone")),System->Supplies[0],System->Supplies[1],System->Supplies[2])));
    Message->SetText(FText::FromString(System->Status));
    for (int32 K=0;K<3;++K) { CraftButtons[K]->SetIsEnabled(System->CanCraft(K)); PlaceButtons[K]->SetIsEnabled(System->Supplies[K]>0); }
}
void UShelterWidget::CraftWall(){ if(System) System->Craft(0); }
void UShelterWidget::CraftFloor(){ if(System) System->Craft(1); }
void UShelterWidget::CraftCeiling(){ if(System) System->Craft(2); }
void UShelterWidget::PlaceWall(){ if(System) System->SelectPiece(0); }
void UShelterWidget::PlaceFloor(){ if(System) System->SelectPiece(1); }
void UShelterWidget::PlaceCeiling(){ if(System) System->SelectPiece(2); }

FString AShelterSystem::RunVerification()
{
    // Commandlets load editor maps without trace collision. Use an isolated world with it enabled.
    const auto Init=UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true).EnableTraceCollision(true).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false).RequiresHitProxies(false);
    UWorld* TestWorld=UWorld::CreateWorld(EWorldType::Editor,false,NAME_None,nullptr,true,ERHIFeatureLevel::Num,&Init);
    if(!TestWorld) return TEXT("Failures: 1\nFAIL: Could not create isolated verification world\n");
    FString Report;
    int32 Failures=0;
    auto Check=[&](const TCHAR* Name,bool Passed) { Report+=FString::Printf(TEXT("%s: %s\n"),Passed ? TEXT("PASS") : TEXT("FAIL"),Name); if(!Passed) ++Failures; };
    // All test actors are transient and destroyed before returning; real player state is untouched.
    FActorSpawnParameters Params; Params.ObjectFlags|=RF_Transient;
    auto* TestPC=TestWorld->SpawnActor<APlayerController>(APlayerController::StaticClass(),FTransform::Identity,Params);
    UClass* PawnClass=LoadClass<APawn>(nullptr,TEXT("/Game/Player/BP_SurvivalCharacter.BP_SurvivalCharacter_C"));
    auto* Pawn=PawnClass ? TestWorld->SpawnActor<APawn>(PawnClass,FTransform(FVector(10000,10000,200)),Params) : nullptr;
    auto* Test=TestWorld->SpawnActor<AShelterSystem>(AShelterSystem::StaticClass(),FTransform::Identity,Params);
    Check(TEXT("Test actors and original Blueprint character load"),TestPC && Pawn && Test);
    if(TestPC && Pawn && Test)
    {
        TestPC->Possess(Pawn); Test->PC=TestPC;
        Check(TEXT("Original Wood and Stone integer properties exist"),FindFProperty<FIntProperty>(PawnClass,TEXT("Wood")) && FindFProperty<FIntProperty>(PawnClass,TEXT("Stone")));
        Check(TEXT("Inventory starts empty"),Test->Resource(TEXT("Wood"))==0 && Test->Resource(TEXT("Stone"))==0);
        Test->Craft(0);
        Check(TEXT("Crafting without materials is rejected"),Test->Supplies[0]==0);
        Check(TEXT("Resource additions update the shared inventory"),Test->ChangeResource(TEXT("Wood"),95) && Test->ChangeResource(TEXT("Stone"),50) && Test->Resource(TEXT("Wood"))==95);
        for(int32 K=0;K<3;++K) Test->Craft(K);
        Check(TEXT("All three recipes consume exact total costs (95 wood, 50 stone)"),Test->Resource(TEXT("Wood"))==0 && Test->Resource(TEXT("Stone"))==0);
        Check(TEXT("Each crafted item increments its own supply"),Test->Supplies==TArray<int32>({1,1,1}));
        Test->ChangeResource(TEXT("Wood"),30); Test->Craft(0);
        Check(TEXT("Insufficient stone never spends wood"),Test->Resource(TEXT("Wood"))==30 && Test->Supplies[0]==1);
        Test->SelectPiece(0);
        Check(TEXT("Selecting an item spawns a wall preview"),IsValid(Test->Preview) && Test->Preview->Kind==0);
        Check(TEXT("Preview has no collision"),Test->Preview && Test->Preview->Collision->GetCollisionEnabled()==ECollisionEnabled::NoCollision);
        if(Test->Preview)
        {
            Test->Preview->Configure(0,false);
            Check(TEXT("Placed geometry enables collision"),Test->Preview->Collision->GetCollisionEnabled()==ECollisionEnabled::QueryAndPhysics);
            Check(TEXT("Placed geometry blocks player pawns"),Test->Preview->Collision->GetCollisionResponseToChannel(ECC_Pawn)==ECR_Block);
            Check(TEXT("Wall dimensions are 20 x 400 x 300 cm"),Test->Preview->Mesh->GetComponentScale().Equals(FVector(0.2f,4.f,3.f)));
        }
        Test->CancelPlacement();
        Check(TEXT("Cancel removes preview without spending a supply"),!Test->Preview && Test->Supplies[0]==1);
        Test->SelectPiece(1); Check(TEXT("Floor selection spawns a floor"),Test->Preview && Test->Preview->Kind==1); Test->CancelPlacement();
        Test->SelectPiece(2); Check(TEXT("Ceiling selection spawns a ceiling"),Test->Preview && Test->Preview->Kind==2); Test->CancelPlacement();
        Test->Craft(-1); Test->SelectPiece(99);
        Check(TEXT("Invalid item indexes are safely rejected"),!Test->Preview && Test->Supplies==TArray<int32>({1,1,1}));

        // Exercise the actual trace and commit path on a transient platform, far from the level.
        auto* Ground=TestWorld->SpawnActor<AShelterPiece>(AShelterPiece::StaticClass(),FTransform(FVector(40000,40000,-10)),Params);
        auto* Camera=TestWorld->SpawnActor<AShelterPiece>(AShelterPiece::StaticClass(),FTransform(FRotator(-45,0,0),FVector(40000,40000,500)),Params);
        if(Ground && Camera)
        {
            Ground->Configure(1,false); Ground->Mesh->SetWorldScale3D(FVector(20,20,0.2f)); Ground->Collision->SetBoxExtent(FVector(1000,1000,10));
            Camera->Configure(1,true); Camera->SetActorHiddenInGame(true);
            // An editor-only controller has no camera manager; its own transform is the view.
            TestPC->SetInitialLocationAndRotation(Camera->GetActorLocation(),Camera->GetActorRotation());
            Test->SelectPiece(1); Test->UpdatePreview();
            if(!Test->bPlacementValid)
            {
                FVector ViewLocation; FRotator ViewRotation; TestPC->GetPlayerViewPoint(ViewLocation,ViewRotation);
                FHitResult DebugHit; FCollisionQueryParams DebugParams; DebugParams.AddIgnoredActor(Pawn); DebugParams.AddIgnoredActor(Camera); DebugParams.AddIgnoredActor(Test->Preview);
                const bool Found=TestWorld->LineTraceSingleByChannel(DebugHit,ViewLocation,ViewLocation+ViewRotation.Vector()*1200.f,ECC_Visibility,DebugParams);
                Report+=FString::Printf(TEXT("TRACE DETAIL: View %s, Rotation %s, Hit %d %s, Preview %s\n"),*ViewLocation.ToString(),*ViewRotation.ToString(),Found,*GetNameSafe(DebugHit.GetActor()),Test->Preview ? *Test->Preview->GetActorLocation().ToString() : TEXT("none"));
            }
            Check(TEXT("Visibility trace produces a valid floor placement"),Test->bPlacementValid);
            AShelterPiece* PlacedFloor=Test->Preview;
            Test->Place();
            const bool bCommitted=Test->Preview==nullptr && Test->Supplies[1]==0;
            Check(TEXT("Confirming placement commits the actor and consumes one supply"),bCommitted);
            if(bCommitted && PlacedFloor)
            {
                Check(TEXT("Committed floor blocks pawns"),PlacedFloor->Collision->GetCollisionResponseToChannel(ECC_Pawn)==ECR_Block && PlacedFloor->Collision->GetCollisionEnabled()==ECollisionEnabled::QueryAndPhysics);
                Test->Supplies[1]=1; Test->SelectPiece(1);
                Test->HeightOffset=-20; Test->UpdatePreview();
                Check(TEXT("Overlap validation rejects geometry embedded in its support"),!Test->bPlacementValid);
                Test->Place();
                Check(TEXT("Rejected placement preserves the crafted supply"),Test->Supplies[1]==1 && Test->Preview!=nullptr);
                Test->CancelPlacement();
                Camera->SetActorRotation(FRotator(45,0,0)); TestPC->SetControlRotation(Camera->GetActorRotation()); Test->SelectPiece(0); Test->UpdatePreview();
                Check(TEXT("A trace with no surface rejects placement"),!Test->bPlacementValid);
                Test->CancelPlacement();
                Camera->SetActorRotation(FRotator(-45,0,0)); TestPC->SetControlRotation(Camera->GetActorRotation()); Test->SelectPiece(2); Test->UpdatePreview();
                Check(TEXT("Ceiling preview clears the floor by one wall height"),Test->Preview && Test->Preview->GetActorLocation().Z>300.f && Test->bPlacementValid);
                Test->CancelPlacement();
                PlacedFloor->Destroy();
            }
            else Test->CancelPlacement();
        }
        else Check(TEXT("Transient trace-test geometry spawns"),false);
        if(Ground) Ground->Destroy(); if(Camera) Camera->Destroy();
    }
    if(Test) Test->Destroy(); if(Pawn) Pawn->Destroy(); if(TestPC) TestPC->Destroy();
    TestWorld->DestroyWorld(false);
    return FString::Printf(TEXT("Failures: %d\n"),Failures)+Report;
}
