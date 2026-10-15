#include "ThunderRideTestSubsystem.h"
#include "ThunderGaleleafTestSubsystem.h"
#include "ThunderMeadowStage.h"
#include "ThunderRollingBoulder.h"
#include "PlayseedGaleleafEnemy.h"
#include "PlayseedGlideBoard.h"
#include "PlayseedRideComponent.h"
#include "PlayseedActionComponent.h"
#include "PlayseedClimbComponent.h"
#include "PlayseedCombatComponent.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace
{
void RideKey(APlayerController* PC, FKey Key, bool bPressed)
{ PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, bPressed ? IE_Pressed : IE_Released, bPressed ? 1 : 0)); }
void RideTap(APlayerController* PC, FKey Key) { RideKey(PC, Key, true); RideKey(PC, Key, false); }
}

void UThunderRideTestSubsystem::SetPhase(int32 Number) { Phase = Number; PhaseTime = 0; Waypoint = 0; }

void UThunderRideTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage)
{
    Stage = InStage;
    Board.Reset(); Error.Reset(); Route.Reset();
    SetPhase(0);
    bJumpHeld = bSlopeScreenshot = bAcquireStrikeSent = false;
    HighestFeet = 0;
    const auto* Move = Player->GetCharacterMovement();
    NormalFriction = Move->GroundFriction; NormalBraking = Move->BrakingDecelerationWalking;
    NormalAcceleration = Move->MaxAcceleration; NormalAirControl = Move->AirControl;
    // 葉をテスト側から生成せず、前単位と同じ攻撃・踏みつけ・撃破を実行して手に入れます。
    GetGameInstance()->GetSubsystem<UThunderGaleleafTestSubsystem>()->BeginTest(Player, Stage->GetGaleleafEnemy());
}

bool UThunderRideTestSubsystem::NormalMovementRestored(AWindseedCharacter* Player) const
{
    const auto* Move = Player->GetCharacterMovement();
    return !Player->GetRide()->IsRiding() && !Player->GetRide()->GetBoard() && FMath::IsNearlyEqual(Move->MaxWalkSpeed, 470.f)
        && FMath::IsNearlyEqual(Move->GroundFriction, NormalFriction) && FMath::IsNearlyEqual(Move->BrakingDecelerationWalking, NormalBraking)
        && FMath::IsNearlyEqual(Move->MaxAcceleration, NormalAcceleration) && FMath::IsNearlyEqual(Move->AirControl, NormalAirControl);
}

bool UThunderRideTestSubsystem::Walk(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    if (Offset.Size2D() < 65 && FMath::Abs(Offset.Z) < 90 && Player->GetCharacterMovement()->IsMovingOnGround())
    { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D(), Offset.Size2D() < 350 ? .35f : 1.f);
    auto* PC = Cast<APlayerController>(Player->GetController());
    PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, Offset.Rotation().Yaw, 0), DeltaSeconds, 3));
    return false;
}

bool UThunderRideTestSubsystem::FollowRoute(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (Waypoint < Route.Num() && Walk(Player, Route[Waypoint], DeltaSeconds)) ++Waypoint;
    return Waypoint >= Route.Num();
}

bool UThunderRideTestSubsystem::AcquireAndMount(AWindseedCharacter* Player, float DeltaSeconds)
{
    auto* PC = Cast<APlayerController>(Player->GetController());
    auto* Enemy = Stage->GetGaleleafEnemy();
    const FVector Approach[] = {FVector(-600, -900, 82), FVector(1900, -900, 82), FVector(1900, -600, 82), FVector(2100, -300, 82)};
    if (Waypoint < 4) { if (Walk(Player, Approach[Waypoint], DeltaSeconds)) ++Waypoint; return false; }
    if (Enemy->HasGaleleaf())
    {
        const FVector Offset = Enemy->GetActorLocation() - Player->GetActorLocation();
        if (Offset.Size2D() < 155 && Player->GetCharacterMovement()->IsMovingOnGround())
        {
            Player->GetCharacterMovement()->StopMovementImmediately();
            if (!bAcquireStrikeSent) { RideTap(PC, EKeys::LeftMouseButton); bAcquireStrikeSent = true; }
        }
        else Player->AddMovementInput(Offset.GetSafeNormal2D(), Offset.Size2D() < 350 ? .35f : 1.f);
        return false;
    }
    Board = Enemy->GetDroppedGaleleaf();
    if (!Board.IsValid()) { Error = TEXT("Actual enemy strike did not leave a board"); return false; }
    if (Walk(Player, Board->GetActorLocation() + FVector(0, 0, 82), DeltaSeconds)) RideTap(PC, EKeys::Q);
    return Player->GetRide()->IsRiding();
}

bool UThunderRideTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* PC = Cast<APlayerController>(Player->GetController());
    if (!Stage.IsValid() || !PC) { Error = TEXT("Ride stage/controller missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 70) { Error = FString::Printf(TEXT("Ride phase %d waypoint %d timed out at %s"), Phase, Waypoint, *Player->GetActorLocation().ToString()); return false; }
    auto* Move = Player->GetCharacterMovement();
    auto* Ride = Player->GetRide();
    if (Phase != 8 && Phase != 9 && Player->GetVitality()->GetHealth() != 8)
    { Error = FString::Printf(TEXT("Ride phase %d unexpectedly took damage at %s"), Phase, *Player->GetActorLocation().ToString()); return false; }
    if (bJumpHeld)
    {
        JumpHold += DeltaSeconds;
        if (JumpHold > .18f) { RideKey(PC, EKeys::SpaceBar, false); bJumpHeld = false; }
    }
    if (Phase == 0)
    {
        auto* Drops = GetGameInstance()->GetSubsystem<UThunderGaleleafTestSubsystem>();
        const bool bDone = Drops->UpdateTest(Player, DeltaSeconds);
        if (!Drops->GetError().IsEmpty()) { Error = Drops->GetError(); return false; }
        if (bDone) { Board = Stage->GetGaleleafEnemy()->GetDroppedGaleleaf(); SetPhase(1); }
    }
    else if (Phase == 1)
    {
        if (!Board.IsValid()) { Error = TEXT("Verified carrier board disappeared"); return false; }
        if (Walk(Player, Board->GetActorLocation() + FVector(0, 0, 82), DeltaSeconds))
        {
            RideTap(PC, EKeys::Q);
            if (!Ride->IsRiding() || Board->GetAttachParentActor() != Player || Move->MovementMode != MOVE_Walking || Move->MaxWalkSpeed != 900)
            { Error = TEXT("Q did not mount the board using normal CharacterMovement"); return false; }
            // 乗車中のShiftと攻撃入力で速度や状態を壊さないことも確認します。
            RideTap(PC, EKeys::LeftShift); RideTap(PC, EKeys::LeftMouseButton); RideTap(PC, EKeys::E);
            if (Move->MaxWalkSpeed != 900 || Player->GetCombat()->IsAttacking()) { Error = TEXT("Ride input exclusivity failed"); return false; }
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderGaleleafRide.png"), false, false);
            SetPhase(2);
        }
    }
    else if (Phase == 2)
    {
        Player->AddMovementInput(FVector(-1, 0, 0));
        if (PhaseTime > 1.3f)
        {
            if (Player->GetVelocity().Size2D() < 700) { Error = TEXT("Riding did not accelerate beyond running speed"); return false; }
            CoastStart = Player->GetActorLocation(); SetPhase(3);
        }
    }
    else if (Phase == 3 && PhaseTime > .35f)
    {
        if (FVector::Dist2D(CoastStart, Player->GetActorLocation()) < 80 || Player->GetVelocity().Size2D() < 300)
        { Error = TEXT("Releasing movement did not preserve visible glide momentum"); return false; }
        Move->StopMovementImmediately();
        RideTap(PC, EKeys::Gamepad_DPad_Up);
        if (!NormalMovementRestored(Player) || !Board.IsValid() || Board->GetAttachParentActor()) { Error = TEXT("D-pad dismount did not restore movement and release the prop"); return false; }
        SetPhase(4);
    }
    else if (Phase == 4 && PhaseTime > .2f)
    {
        RideTap(PC, EKeys::Gamepad_DPad_Up);
        if (!Ride->IsRiding()) { Error = TEXT("Grounded board could not be mounted again"); return false; }
        Route = {FVector(1900, -600, 82), FVector(1900, -900, 82), FVector(0, -900, 82), FVector(0, 1100, 82), FVector(600, 1850, 82)};
        const auto& Feet = Stage->GetMountainFootPath();
        for (int32 Index : {0, 1, 2, 1, 0}) Route.Add(Feet[Index] + FVector(0, 0, 82));
        SetPhase(5);
    }
    else if (Phase == 5)
    {
        HighestFeet = FMath::Max(HighestFeet, float(Player->GetActorLocation().Z - 82));
        const auto* Boulder = Stage->GetRollingBoulder();
        if (!bJumpHeld && !Boulder->IsWaiting() && Move->IsMovingOnGround() && FVector::Dist2D(Boulder->GetActorLocation(), Player->GetActorLocation()) < 430 && FMath::Abs(Boulder->GetActorLocation().Z - 70 - (Player->GetActorLocation().Z - 82)) < 160)
        { RideKey(PC, EKeys::SpaceBar, true); bJumpHeld = true; JumpHold = 0; }
        if (!bSlopeScreenshot && HighestFeet > 150)
        { FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderRideSlope.png"), false, false); bSlopeScreenshot = true; }
        if (FollowRoute(Player, DeltaSeconds))
        {
            if (!Ride->IsRiding() || HighestFeet < 150 || Player->GetActorLocation().Z > 130) { Error = TEXT("Actual slope ascent and descent did not complete while riding"); return false; }
            RecoveryBeforeReset = Player->RecoveryCount;
            RideTap(PC, EKeys::R); SetPhase(6);
        }
    }
    else if (Phase == 6 && PhaseTime > .3f)
    {
        if (!NormalMovementRestored(Player) || Board.IsValid() || Player->RecoveryCount != RecoveryBeforeReset + 1 || !Stage->GetGaleleafEnemy()->HasGaleleaf())
        { Error = TEXT("R destroyed the board but left ride movement or failed to restore the carrier"); return false; }
        bAcquireStrikeSent = false; SetPhase(7);
    }
    else if (Phase == 7)
    {
        if (AcquireAndMount(Player, DeltaSeconds))
        {
            ImpactBefore = Player->GetVitality()->GetSuccessfulImpacts();
            Route = {FVector(1900, -600, 82), FVector(1900, -900, 82), FVector(0, -900, 82), FVector(0, -1200, 82)};
            SetPhase(8);
        }
    }
    else if (Phase == 8)
    {
        if (Player->GetVitality()->GetHealth() == 6)
        {
            if (!NormalMovementRestored(Player) || Player->GetVitality()->GetSuccessfulImpacts() != ImpactBefore + 1) { Error = TEXT("Real hazard impact did not cancel riding and restore movement"); return false; }
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderRideImpact.png"), false, false);
            RideTap(PC, EKeys::R); SetPhase(9);
        }
        else FollowRoute(Player, DeltaSeconds);
    }
    else if (Phase == 9 && PhaseTime > .3f)
    {
        if (!NormalMovementRestored(Player) || Player->GetVitality()->GetHealth() != 8) { Error = TEXT("R after ride damage did not restore normal play"); return false; }
        bAcquireStrikeSent = false; SetPhase(10);
    }
    else if (Phase == 10)
    {
        if (AcquireAndMount(Player, DeltaSeconds)) SetPhase(11);
    }
    else if (Phase == 11)
    {
        if (Walk(Player, FVector(2250, 300, 82), DeltaSeconds))
        { PoundBefore = Player->GetActions()->GetCompletedPounds(); RideKey(PC, EKeys::SpaceBar, true); bJumpHeld = true; JumpHold = 0; SetPhase(12); }
    }
    else if (Phase == 12 && PhaseTime > .22f) { RideTap(PC, EKeys::LeftControl); SetPhase(13); }
    else if (Phase == 13)
    {
        if (Ride->IsRiding()) { Error = TEXT("Midair ground pound did not cancel riding"); return false; }
        if (Move->IsMovingOnGround() && PhaseTime > .2f)
        {
            if (!NormalMovementRestored(Player) || Player->GetActions()->GetCompletedPounds() != PoundBefore + 1) { Error = TEXT("Ground pound landing retained glide settings"); return false; }
            SetPhase(14);
        }
    }
    else if (Phase == 14)
    {
        if (!Board.IsValid()) { Error = TEXT("Pound cancellation lost the grounded board"); return false; }
        if (Walk(Player, Board->GetActorLocation() + FVector(0, 0, 82), DeltaSeconds))
        {
            RideTap(PC, EKeys::Q);
            if (!Ride->IsRiding()) { Error = TEXT("Board could not be mounted after pound"); return false; }
            Route = {FVector(1900, -600, 82), FVector(1900, -900, 82), FVector(-1800, -900, 82), FVector(-1800, -580, 82)};
            SetPhase(15);
        }
    }
    else if (Phase == 15)
    {
        if (FollowRoute(Player, DeltaSeconds)) { RideTap(PC, EKeys::F); SetPhase(16); }
    }
    else if (Phase == 16 && PhaseTime > .2f)
    {
        if (!Player->GetClimb()->IsClimbing() || !NormalMovementRestored(Player)) { Error = TEXT("Actual tree climbing did not cancel riding and restore settings"); return false; }
        RideTap(PC, EKeys::F); SetPhase(17);
    }
    else if (Phase == 17 && Move->IsMovingOnGround())
    {
        if (!Board.IsValid()) { Error = TEXT("Climb cancellation lost the board"); return false; }
        if (Walk(Player, Board->GetActorLocation() + FVector(0, 0, 82), DeltaSeconds))
        {
            RideTap(PC, EKeys::Q);
            if (!Ride->IsRiding()) { Error = TEXT("Remount after climbing failed"); return false; }
            RecoveryBeforeFall = Player->RecoveryCount; DefeatBeforeFall = Player->GetVitality()->GetDefeatCount();
            Route = {FVector(-1800, -900, 82), FVector(-3000, -600, 82), FVector(-3800, -600, 82)}; SetPhase(18);
        }
    }
    else if (Phase == 18)
    {
        if (Player->RecoveryCount > RecoveryBeforeFall)
        {
            if (Player->RecoveryCount != RecoveryBeforeFall + 1 || Player->GetVitality()->GetDefeatCount() != DefeatBeforeFall + 1 || !NormalMovementRestored(Player) || Board.IsValid() || Player->GetVitality()->GetHealth() != 8)
            { Error = TEXT("Actual offstage glide fall did not cleanly defeat and restore normal play"); return false; }
            UE_LOG(LogTemp, Display, TEXT("RIDE TEST PASS: actual carrier drop, Q/D-pad mount/dismount/remount, speed and coasting, slope ascent/descent, mounted R, real hazard, ordinary jump/pound, tree climb and actual offstage fall cleanup passed."));
            return true;
        }
        FollowRoute(Player, DeltaSeconds);
    }
    return false;
}
