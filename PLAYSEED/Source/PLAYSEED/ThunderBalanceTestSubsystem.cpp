#include "ThunderBalanceTestSubsystem.h"
#include "ThunderTiltingBridge.h"
#include "WindseedCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UThunderBalanceTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderTiltingBridge* InBridge)
{
    Bridge = InBridge;
    Phase = 0;
    PhaseTime = 0;
    RecoveryBaseline = Player->RecoveryCount;
    Error.Reset();
    bRestartVerified = false;
    bSnapshotSaved = false;
    if (auto* PC = Cast<APlayerController>(Player->GetController()))
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseWheelAxis, IE_Axis, -6));
}

bool UThunderBalanceTestSubsystem::MoveToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    auto* Movement = Player->GetCharacterMovement();
    if (Offset.Size2D() < 55 && FMath::Abs(Offset.Z) < 55 && Movement->IsMovingOnGround())
    {
        Player->StopJumping();
        Movement->StopMovementImmediately();
        return true;
    }
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (Offset.Z > 50 && Offset.Size2D() < 480 && Movement->IsMovingOnGround()) Player->Jump();
    else if (Player->GetVelocity().Z < 0) Player->StopJumping();
    if (auto* PC = Cast<APlayerController>(Player->GetController()))
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, Offset.Rotation().Yaw, 0), DeltaSeconds, 2));
    return false;
}

bool UThunderBalanceTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    if (!Bridge.IsValid()) { Error = TEXT("Tilting bridge is missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 20) { Error = FString::Printf(TEXT("Balance bridge phase %d timed out at %s"), Phase, *Player->GetActorLocation().ToString()); return false; }
    if (Phase != 4 && Player->RecoveryCount != RecoveryBaseline) { Error = TEXT("Unexpected recovery while crossing balance bridge"); return false; }
    auto* PC = Cast<APlayerController>(Player->GetController());
    const bool bRiding = Player->GetMovementBaseObject() == Bridge->GetSurface();
    auto Advance = [this](int32 Next) { Phase = Next; PhaseTime = 0; };
    if (Phase == 0)
    {
        if (MoveToward(Player, FVector(-850, -1750, 82), DeltaSeconds)) Advance(1);
    }
    else if (Phase == 1)
    {
        if (MoveToward(Player, FVector(-850, -2100, 262), DeltaSeconds)) Advance(2);
    }
    else if (Phase == 2)
    {
        if (MoveToward(Player, Bridge->GetDeckPoint(-400) + FVector(0, 0, 82), DeltaSeconds) && bRiding) Advance(3);
    }
    else if (Phase == 3 && PhaseTime > 1)
    {
        if (!bRiding || Bridge->GetActorRotation().Pitch < 3) { Error = TEXT("Left-side weight did not tilt the bridge"); return false; }
        UE_LOG(LogTemp, Display, TEXT("BALANCE TEST: left side pitch %.2f degrees."), Bridge->GetActorRotation().Pitch);
        if (!bRestartVerified && PC)
        {
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R, IE_Pressed, 1));
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R, IE_Released, 0));
            Advance(4);
        }
        else Advance(5);
    }
    else if (Phase == 4 && PhaseTime > .3f)
    {
        if (Player->RecoveryCount != RecoveryBaseline + 1 || FVector::Dist2D(Player->GetActorLocation(), FVector(-600, -600, 100)) > 5 || FMath::Abs(Bridge->GetActorRotation().Pitch) > .1f)
        { Error = TEXT("R did not reset the bridge and recover without dragging"); return false; }
        RecoveryBaseline = Player->RecoveryCount;
        bRestartVerified = true;
        UE_LOG(LogTemp, Display, TEXT("BALANCE TEST: R reset passed; crossing again."));
        Advance(0);
    }
    else if (Phase == 5)
    {
        if (MoveToward(Player, Bridge->GetDeckPoint(400) + FVector(0, 0, 82), DeltaSeconds) && bRiding) Advance(6);
    }
    else if (Phase == 6 && PhaseTime > 1)
    {
        if (!bRiding || Bridge->GetActorRotation().Pitch > -3) { Error = TEXT("Right-side weight did not tilt the bridge"); return false; }
        UE_LOG(LogTemp, Display, TEXT("BALANCE TEST: right side pitch %.2f degrees."), Bridge->GetActorRotation().Pitch);
        Advance(7);
    }
    else if (Phase == 7)
    {
        if (MoveToward(Player, FVector(850, -2100, 262), DeltaSeconds)) Advance(8);
    }
    else if (Phase == 8)
    {
        if (PC) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-15, 180, 0), DeltaSeconds, 3));
        if (PhaseTime > 1 && !bSnapshotSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderTiltingBridge.png"), false, false);
            bSnapshotSaved = true;
            UE_LOG(LogTemp, Display, TEXT("BALANCE TEST PASS: ordinary boarding, left/right tilt, R reset, reboarding and crossing passed."));
        }
        return PhaseTime > 2;
    }
    return false;
}
