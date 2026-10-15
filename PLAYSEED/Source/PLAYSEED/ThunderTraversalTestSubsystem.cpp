#include "ThunderTraversalTestSubsystem.h"
#include "WindseedCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UThunderTraversalTestSubsystem::BeginTerraceRoute(AWindseedCharacter* Player)
{
    Route = {FVector(-650, 350, 82), FVector(-1150, 350, 182), FVector(-1550, 700, 182),
        FVector(-1650, 1200, 282), FVector(-1450, 1650, 282), FVector(-900, 1650, 382)};
    Waypoint = 0;
    JumpCount = 0;
    LegTime = 0;
    FinishTime = 0;
    Error.Reset();
    bScreenshotRequested = false;
    RecoveryBaseline = Player->RecoveryCount;
    if (auto* PC = Cast<APlayerController>(Player->GetController()))
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseWheelAxis, IE_Axis, -6));
}

bool UThunderTraversalTestSubsystem::UpdateTerraceRoute(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (HasFailed()) return false;
    if (Player->RecoveryCount != RecoveryBaseline)
    { Error = TEXT("Unexpected fall recovery during terrace traversal"); return false; }
    auto* PC = Cast<APlayerController>(Player->GetController());
    if (Waypoint == Route.Num())
    {
        if (JumpCount < 3) { Error = TEXT("Terraces were not reached with at least three jumps"); return false; }
        FinishTime += DeltaSeconds;
        if (PC) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-25, 180, 0), DeltaSeconds, 3));
        if (FinishTime > 1 && !bScreenshotRequested)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderTerraces.png"), false, false);
            bScreenshotRequested = true;
            UE_LOG(LogTemp, Display, TEXT("TERRACE TEST PASS: all six waypoints, %d ordinary jumps, no recovery."), JumpCount);
        }
        return FinishTime > 2;
    }
    LegTime += DeltaSeconds;
    if (LegTime > 15)
    {
        Error = FString::Printf(TEXT("Terrace waypoint %d timed out at %s"), Waypoint, *Player->GetActorLocation().ToString());
        return false;
    }
    const FVector Offset = Route[Waypoint] - Player->GetActorLocation();
    auto* Movement = Player->GetCharacterMovement();
    if (Offset.Size2D() < 55 && FMath::Abs(Offset.Z) < 65 && Movement->IsMovingOnGround())
    {
        ++Waypoint;
        LegTime = 0;
        Player->StopJumping();
        Movement->StopMovementImmediately();
        UE_LOG(LogTemp, Display, TEXT("TERRACE TEST: reached waypoint %d."), Waypoint);
        return false;
    }
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (Offset.Z > 50 && Offset.Size2D() < 480 && Movement->IsMovingOnGround())
    { Player->Jump(); ++JumpCount; }
    else if (Player->GetVelocity().Z < 0) Player->StopJumping();
    if (PC) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-23, Offset.Rotation().Yaw, 0), DeltaSeconds, 2));
    return false;
}
