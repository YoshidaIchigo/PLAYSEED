#include "ThunderPlatformTestSubsystem.h"
#include "ThunderMovingPlatform.h"
#include "WindseedCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UThunderPlatformTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderMovingPlatform* InPlatform)
{
    Platform = InPlatform;
    Phase = 0;
    PhaseTime = 0;
    RecoveryBaseline = Player->RecoveryCount;
    Error.Reset();
    bRestartVerified = false;
    bSnapshotSaved = false;
    if (auto* PC = Cast<APlayerController>(Player->GetController()))
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseWheelAxis, IE_Axis, -6));
}

bool UThunderPlatformTestSubsystem::IsRiding(AWindseedCharacter* Player) const
{
    return Platform.IsValid() && Player->GetCharacterMovement()->IsMovingOnGround() && Player->GetMovementBaseObject() == Platform->GetSurface();
}

void UThunderPlatformTestSubsystem::StartRide(AWindseedCharacter* Player)
{
    Player->StopJumping();
    Player->GetCharacterMovement()->StopMovementImmediately();
    RideStartX = Platform->GetActorLocation().X;
    RideRelativeStart = Player->GetActorLocation() - Platform->GetActorLocation();
    Phase = 2;
    PhaseTime = 0;
}

bool UThunderPlatformTestSubsystem::MoveToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds, bool bJump)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    auto* Movement = Player->GetCharacterMovement();
    if (Offset.Size2D() < 60 && FMath::Abs(Offset.Z) < 50 && Movement->IsMovingOnGround())
    {
        Player->StopJumping();
        Movement->StopMovementImmediately();
        return true;
    }
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (bJump && Offset.Z > 50 && Offset.Size2D() < 480 && Movement->IsMovingOnGround()) Player->Jump();
    else if (Player->GetVelocity().Z < 0) Player->StopJumping();
    if (auto* PC = Cast<APlayerController>(Player->GetController()))
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-23, Offset.Rotation().Yaw, 0), DeltaSeconds, 2));
    return false;
}

bool UThunderPlatformTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    if (!Platform.IsValid()) { Error = TEXT("Moving platform is missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 20) { Error = FString::Printf(TEXT("Moving platform phase %d timed out at %s"), Phase, *Player->GetActorLocation().ToString()); return false; }
    if (Phase != 3 && Player->RecoveryCount != RecoveryBaseline) { Error = TEXT("Unexpected fall recovery during moving-platform test"); return false; }
    auto* PC = Cast<APlayerController>(Player->GetController());
    if (Phase == 0)
    {
        if (MoveToward(Player, FVector(600, 400, 82), DeltaSeconds, false)) { Phase = 1; PhaseTime = 0; }
    }
    else if (Phase == 1)
    {
        if (IsRiding(Player)) StartRide(Player);
        else MoveToward(Player, Platform->GetSurfaceTop() + FVector(0, 0, Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), DeltaSeconds, true);
    }
    else if (Phase == 2)
    {
        // 入力で追いかけず、床の移動だけで相対位置を保って運ばれることを確認します。
        const FVector Relative = Player->GetActorLocation() - Platform->GetActorLocation();
        if (!IsRiding(Player) || FVector::Dist2D(Relative, RideRelativeStart) > 100)
        { Error = TEXT("CharacterMovement did not carry the standing player"); return false; }
        const float Distance = FMath::Abs(Platform->GetActorLocation().X - RideStartX);
        if (Distance > 120 && !bRestartVerified && PC)
        {
            UE_LOG(LogTemp, Display, TEXT("PLATFORM TEST: carried %.1f cm without movement input; pressing R while riding."), Distance);
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R, IE_Pressed, 1));
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R, IE_Released, 0));
            Phase = 3; PhaseTime = 0;
        }
        else if (Distance > 120 && bRestartVerified && Platform->GetActorLocation().X > 740)
        { Phase = 4; PhaseTime = 0; }
    }
    else if (Phase == 3 && PhaseTime > .3f)
    {
        if (Player->RecoveryCount != RecoveryBaseline + 1 || FVector::Dist2D(Player->GetActorLocation(), FVector(-600, -600, 100)) > 5 ||
            FMath::Abs(Platform->GetActorLocation().X - 600) > 70)
        { Error = TEXT("R did not reset the platform and recover without movement-base drag"); return false; }
        RecoveryBaseline = Player->RecoveryCount;
        bRestartVerified = true;
        Phase = 0; PhaseTime = 0;
        UE_LOG(LogTemp, Display, TEXT("PLATFORM TEST: R recovery and platform reset passed; boarding again."));
    }
    else if (Phase == 4)
    {
        if (MoveToward(Player, FVector(1250, 800, 302), DeltaSeconds, true))
        {
            if (IsRiding(Player)) { Error = TEXT("Player never left the moving platform"); return false; }
            Phase = 5; PhaseTime = 0;
        }
    }
    else if (Phase == 5)
    {
        if (PC) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, 200, 0), DeltaSeconds, 3));
        if (PhaseTime > 1 && !bSnapshotSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderMovingPlatform.png"), false, false);
            bSnapshotSaved = true;
            UE_LOG(LogTemp, Display, TEXT("PLATFORM TEST PASS: ordinary boarding, carry, R recovery, reboarding and jump to endpoint passed."));
        }
        return PhaseTime > 2;
    }
    return false;
}
