#include "ThunderClimbTestSubsystem.h"
#include "PlayseedClimbTree.h"
#include "PlayseedClimbComponent.h"
#include "WindseedCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace
{
void ClimbKey(APlayerController* PC, FKey Key, bool bDown) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, bDown ? IE_Pressed : IE_Released, bDown ? 1 : 0)); }
void ClimbTap(APlayerController* PC, FKey Key) { ClimbKey(PC, Key, true); ClimbKey(PC, Key, false); }
}

void UThunderClimbTestSubsystem::BeginTest(AWindseedCharacter* Player, APlayseedClimbTree* InTree)
{
    Tree = InTree;
    Error.Reset();
    Phase = 0;
    PhaseTime = 0;
    RecoveryBaseline = Player->RecoveryCount;
    NormalGravity = Player->GetCharacterMovement()->GravityScale;
    bJumpVerified = false;
    bJumpAwayVerified = false;
    bTopSaved = false;
    bRecoverySaved = false;
    if (auto* PC = Cast<APlayerController>(Player->GetController())) ClimbTap(PC, EKeys::F);
}

bool UThunderClimbTestSubsystem::WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    if (Offset.Size2D() < 25 && Player->GetCharacterMovement()->IsMovingOnGround())
    { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (auto* PC = Cast<APlayerController>(Player->GetController())) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-18, Offset.Rotation().Yaw, 0), DeltaSeconds, 2));
    return false;
}

bool UThunderClimbTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    if (!Tree.IsValid()) { Error = TEXT("Climb tree is missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 20) { Error = FString::Printf(TEXT("Climb phase %d timed out at %s"), Phase, *Player->GetActorLocation().ToString()); return false; }
    auto* Climb = Player->GetClimb();
    auto* Movement = Player->GetCharacterMovement();
    auto* PC = Cast<APlayerController>(Player->GetController());
    if (!PC) { Error = TEXT("Climb controller is missing"); return false; }
    const FVector Approach = Tree->GetActorLocation() + FVector(130, 0, 82);
    auto Advance = [this](int32 Next) { Phase = Next; PhaseTime = 0; };
    if (Phase == 0 && PhaseTime > .3f)
    {
        if (Climb->IsClimbing()) { Error = TEXT("F grabbed an out-of-range tree"); return false; }
        Advance(1);
    }
    else if (Phase == 1 || Phase == 8)
    {
        if (WalkToward(Player, Approach, DeltaSeconds))
        {
            ClimbTap(PC, Phase == 1 ? EKeys::F : EKeys::Gamepad_FaceButton_Top);
            Advance(Phase == 1 ? 2 : 9);
        }
    }
    else if ((Phase == 2 || Phase == 9) && PhaseTime > .2f)
    {
        if (!Climb->IsClimbing() || Movement->MovementMode != MOVE_Flying) { Error = TEXT("F/Y did not begin the climb"); return false; }
        ClimbKey(PC, EKeys::W, true);
        Advance(Phase == 2 ? 3 : 10);
    }
    else if (Phase == 3 && Player->GetActorLocation().Z > Tree->GetActorLocation().Z + 360)
    {
        HeightBeforeDescent = Player->GetActorLocation().Z;
        ClimbKey(PC, EKeys::W, false);
        ClimbKey(PC, EKeys::S, true);
        Advance(4);
    }
    else if (Phase == 4 && Player->GetActorLocation().Z < HeightBeforeDescent - 140)
    {
        if (!Climb->IsClimbing()) { Error = TEXT("S descent lost the tree too early"); return false; }
        ClimbKey(PC, EKeys::S, false);
        ClimbKey(PC, EKeys::W, true);
        Advance(5);
    }
    else if (Phase == 5 && !Climb->IsClimbing())
    {
        ClimbKey(PC, EKeys::W, false);
        if (Movement->IsMovingOnGround())
        {
            const float Feet = Player->GetActorLocation().Z - Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
            const auto* Base = Cast<UPrimitiveComponent>(Player->GetMovementBaseObject());
            if (FMath::Abs(Feet - Tree->GetTopZ()) > 10 || !Base || Base->GetOwner() != Tree.Get())
            { Error = TEXT("Climb did not land on the crown platform with collision restored"); return false; }
            UE_LOG(LogTemp, Display, TEXT("CLIMB TEST: W ascent, S descent and physical crown landing at %.1f cm passed."), Feet);
            Advance(6);
        }
    }
    else if (Phase == 6)
    {
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-15, 135, 0), DeltaSeconds, 3));
        if (PhaseTime > .7f && !bTopSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderTreeTop.png"), false, false);
            bTopSaved = true;
        }
        if (PhaseTime > 1.2f) { ClimbKey(PC, EKeys::SpaceBar, true); Advance(7); }
    }
    else if (Phase == 7)
    {
        if (PhaseTime > .15f) ClimbKey(PC, EKeys::SpaceBar, false);
        if (Movement->IsFalling()) Player->AddMovementInput(FVector::ForwardVector);
        if (PhaseTime > .2f && !bJumpVerified)
        {
            if (Climb->IsClimbing() || !Movement->IsFalling() || Player->GetVelocity().Z <= 0) { Error = TEXT("Space did not jump from the crown"); return false; }
            bJumpVerified = true;
        }
        if (bJumpVerified && Movement->IsMovingOnGround())
        {
            if (FVector::Dist2D(Player->GetActorLocation(), Tree->GetActorLocation()) < 250 || !FMath::IsNearlyEqual(Movement->GravityScale, NormalGravity))
            { Error = TEXT("Tree jump did not return to ordinary movement"); return false; }
            Advance(8);
        }
    }
    else if (Phase == 10 && Player->GetActorLocation().Z > Tree->GetActorLocation().Z + 300)
    {
        ClimbKey(PC, EKeys::W, false);
        if (bJumpAwayVerified) { ClimbTap(PC, EKeys::R); Advance(11); }
        else { ClimbTap(PC, EKeys::SpaceBar); Advance(15); }
    }
    else if (Phase == 11 && PhaseTime > .3f)
    {
        if (Climb->IsClimbing() || Movement->MovementMode == MOVE_Flying || Player->RecoveryCount != RecoveryBaseline + 1 || FVector::Dist2D(Player->GetActorLocation(), FVector(-600, -600, 100)) > 5)
        { Error = TEXT("R did not release the climb and restore the start"); return false; }
        Advance(12);
    }
    else if (Phase == 12)
    {
        if (WalkToward(Player, Approach, DeltaSeconds)) Advance(13);
    }
    else if (Phase == 13)
    {
        WalkToward(Player, Tree->GetActorLocation() + FVector(0, 0, 82), DeltaSeconds);
        if (PhaseTime > 1)
        {
            const double Radius = FVector::Dist2D(Player->GetActorLocation(), Tree->GetActorLocation());
            Player->GetCharacterMovement()->StopMovementImmediately();
            if (Radius < 85 || Radius > 105 || Climb->IsClimbing()) { Error = TEXT("Tree trunk collision remained ignored after R"); return false; }
            Advance(14);
        }
    }
    else if (Phase == 14)
    {
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-15, -150, 0), DeltaSeconds, 3));
        if (PhaseTime > 1 && !bRecoverySaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderClimbRecovery.png"), false, false);
            bRecoverySaved = true;
            UE_LOG(LogTemp, Display, TEXT("CLIMB TEST PASS: range, F/Y grab, W/S climb, crown landing, Space jump, R release and normal trunk collision passed."));
        }
        return PhaseTime > 2;
    }
    else if (Phase == 15)
    {
        if (PhaseTime > .15f && !bJumpAwayVerified)
        {
            if (Climb->IsClimbing() || !Movement->IsFalling() || Player->GetVelocity().X < 200 || Player->GetVelocity().Z <= 0)
            { Error = TEXT("Space did not leap away from the held trunk"); return false; }
            bJumpAwayVerified = true;
        }
        if (bJumpAwayVerified && Movement->IsMovingOnGround()) Advance(8);
    }
    return false;
}
