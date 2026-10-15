#include "ThunderFlightTestSubsystem.h"
#include "ThunderGuideTestSubsystem.h"
#include "ThunderMeadowStage.h"
#include "ThunderLauncher.h"
#include "ThunderFlightBloom.h"
#include "PlayseedFlightComponent.h"
#include "PlayseedLaunchComponent.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace
{
void FlightKey(APlayerController* PC, FKey Key, bool bPressed) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, bPressed ? IE_Pressed : IE_Released, bPressed ? 1 : 0)); }
void FlightTap(APlayerController* PC, FKey Key) { FlightKey(PC, Key, true); FlightKey(PC, Key, false); }
void FlightAxis(APlayerController* PC, FKey Key, float Value) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Axis, Value)); }
}
void UThunderFlightTestSubsystem::SetPhase(int32 Number)
{
    Phase = Number; PhaseTime = 0; Waypoint = 0;
    UE_LOG(LogTemp, Display, TEXT("FLIGHT TEST: phase %d"), Number);
}
void UThunderFlightTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage)
{
    Stage = InStage; Error.Reset(); Route.Reset(); SetPhase(0); OrbitPoint = 0;
    bSlowVerified = bFlightScreenshot = false;
    auto* Power = Player->GetFlight();
    GrantsBefore = Power->GetPowerGrants(); StartsBefore = Power->GetFlightStarts(); ExpirationsBefore = Power->GetExpirations(); LandingsBefore = Power->GetFlightLandings();
    LaunchesBefore = Player->GetLaunch()->GetLaunchCount();
    const auto* Move = Player->GetCharacterMovement();
    NormalFlySpeed = Move->MaxFlySpeed; NormalAcceleration = Move->MaxAcceleration; NormalBraking = Move->BrakingDecelerationFlying;
    NormalArm = Player->GetCameraBoom()->TargetArmLength; NormalSocket = Player->GetCameraBoom()->SocketOffset;
    GetGameInstance()->GetSubsystem<UThunderGuideTestSubsystem>()->BeginTest(Player, InStage, false);
}
bool UThunderFlightTestSubsystem::SettingsRestored(AWindseedCharacter* Player) const
{
    auto* Move = Player->GetCharacterMovement();
    return !Player->GetFlight()->IsFlying() && FMath::IsNearlyEqual(Move->MaxFlySpeed, NormalFlySpeed)
        && FMath::IsNearlyEqual(Move->MaxAcceleration, NormalAcceleration) && FMath::IsNearlyEqual(Move->BrakingDecelerationFlying, NormalBraking)
        && FMath::IsNearlyEqual(Player->GetCameraBoom()->TargetArmLength, NormalArm) && Player->GetCameraBoom()->SocketOffset.Equals(NormalSocket, .1f);
}
void UThunderFlightTestSubsystem::ClearAxes(AWindseedCharacter* Player)
{
    auto* PC = Cast<APlayerController>(Player->GetController());
    FlightAxis(PC, EKeys::MouseX, 0); FlightAxis(PC, EKeys::MouseY, 0);
    FlightKey(PC, EKeys::W, false); FlightKey(PC, EKeys::S, false);
}
void UThunderFlightTestSubsystem::SteerTo(AWindseedCharacter* Player, FVector Goal)
{
    const FRotator Desired = (Goal - Player->GetActorLocation() - Player->GetVelocity() * .2f).Rotation();
    auto* Power = Player->GetFlight();
    auto* PC = Cast<APlayerController>(Player->GetController());
    // 実際のマウス軸を送ります。位置・速度・飛行方向の結果をテストから設定しません。
    FlightAxis(PC, EKeys::MouseX, FMath::Clamp(FRotator::NormalizeAxis(Desired.Yaw - Power->GetFlightYaw()) * 6, -80.f, 80.f));
    FlightAxis(PC, EKeys::MouseY, -FMath::Clamp((FMath::Clamp(Desired.Pitch, -60.f, 70.f) - Power->GetFlightPitch()) * 6, -80.f, 80.f));
}
bool UThunderFlightTestSubsystem::WalkRoute(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (Waypoint >= Route.Num()) return true;
    const FVector Offset = Route[Waypoint] - Player->GetActorLocation();
    if (Offset.Size2D() < 20 && Player->GetCharacterMovement()->IsMovingOnGround()) { Player->GetCharacterMovement()->StopMovementImmediately(); ++Waypoint; }
    else
    {
        Player->AddMovementInput(Offset.GetSafeNormal2D());
        auto* PC = Cast<APlayerController>(Player->GetController());
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, Offset.Rotation().Yaw, 0), DeltaSeconds, 3));
    }
    return Waypoint >= Route.Num();
}
bool UThunderFlightTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* PC = Cast<APlayerController>(Player->GetController());
    if (!Stage.IsValid() || !PC) { Error = TEXT("Flight stage/controller missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 80) { Error = FString::Printf(TEXT("Flight phase %d waypoint %d timed out at %s"), Phase, Waypoint, *Player->GetActorLocation().ToString()); return false; }
    if (Player->GetVitality()->GetHealth() != 8) { Error = FString::Printf(TEXT("Flight phase %d took unexpected damage"), Phase); return false; }
    auto* Power = Player->GetFlight(); auto* Move = Player->GetCharacterMovement();
    if (Phase > 0 && Player->RecoveryCount != RecoveryBefore && Phase != 15) { Error = TEXT("Flight route unexpectedly recovered"); return false; }
    if (Phase == 0)
    {
        auto* Guide = GetGameInstance()->GetSubsystem<UThunderGuideTestSubsystem>(); const bool bDone = Guide->UpdateTest(Player, DeltaSeconds);
        if (!Guide->GetError().IsEmpty()) { Error = Guide->GetError(); return false; }
        if (bDone) { RecoveryBefore = Player->RecoveryCount; Route = {FVector(-3000, -600, 82), FVector(-3000, 2800, 82), Stage->GetFlightBloom()->GetActorLocation() + FVector(0, 0, 82)}; SetPhase(1); }
    }
    else if (Phase == 1 || Phase == 8)
    {
        WalkRoute(Player, DeltaSeconds);
        const int32 Expected = GrantsBefore + (Phase == 1 ? 1 : 2);
        if (Power->GetPowerGrants() >= Expected)
        {
            if (Power->GetPowerGrants() != Expected || Power->GetRemaining() < 40) { Error = TEXT("Actual bloom collection did not start a fresh timer exactly once"); return false; }
            Move->StopMovementImmediately();
            if (Phase == 1) { PowerStartTime = Player->GetWorld()->GetTimeSeconds(); FlightKey(PC, EKeys::SpaceBar, true); SetPhase(2); }
            else { Route = {FVector(-3000, 2200, 82), FVector(-2600, 2000, 82), Stage->GetLauncher()->GetActorLocation() + FVector(-140, 0, 82)}; SetPhase(9); }
        }
    }
    else if (Phase == 2 && PhaseTime > .18f) { FlightKey(PC, EKeys::SpaceBar, false); SetPhase(3); }
    else if (Phase == 3 && PhaseTime > .15f) { FlightTap(PC, EKeys::SpaceBar); SetPhase(4); }
    else if (Phase == 4 && PhaseTime > .2f)
    {
        if (!Power->IsFlying() || Move->MovementMode != MOVE_Flying || Power->GetFlightStarts() != StartsBefore + 1) { Error = TEXT("Second ordinary air Space did not start powered flight"); return false; }
        FlightKey(PC, EKeys::W, true); SetPhase(5);
    }
    else if (Phase == 5)
    {
        SteerTo(Player, FVector(-2600, 1500, 1150));
        if (PhaseTime > 2)
        {
            if (!Power->IsFlying() || Player->GetVelocity().Size() < 1000) { Error = TEXT("W input did not produce faster powered flight"); return false; }
            FlightKey(PC, EKeys::W, false); FlightKey(PC, EKeys::S, true); SetPhase(6);
        }
    }
    else if (Phase == 6)
    {
        const FVector FlightOrbit[] = {FVector(-2850, 650, 1150), FVector(-1550, 650, 1150), FVector(-1550, -650, 1150), FVector(-2850, -650, 1150)};
        if (!Power->HasPower())
        {
            ClearAxes(Player);
            if (Power->IsFlying() || Power->GetExpirations() != ExpirationsBefore + 1 || Player->GetWorld()->GetTimeSeconds() - PowerStartTime < 44 || !bSlowVerified || !SettingsRestored(Player)) { Error = TEXT("Real timed expiry did not end flight and restore settings"); return false; }
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderFlightExpiry.png"), false, false); SetPhase(7);
        }
        else
        {
            if (!Power->IsFlying()) { Error = TEXT("Power flight landed before actual timer expiry"); return false; }
            // 時間切れの前は場外へ慣性で出ないよう、広い草原の上へ通常の操舵で戻ります。
            const FVector Goal = Power->GetRemaining() < 5 ? FVector(-2200, 0, 1150) : FlightOrbit[OrbitPoint % 4];
            SteerTo(Player, Goal);
            if (Power->GetRemaining() >= 5 && FVector::Dist(Player->GetActorLocation(), Goal) < 220) ++OrbitPoint;
            if (PhaseTime > 3 && Player->GetVelocity().Size() < 900) bSlowVerified = true;
            if (!bFlightScreenshot && PhaseTime > 5) { FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderLeafwingFlight.png"), false, false); bFlightScreenshot = true; }
        }
    }
    else if (Phase == 7 && Move->IsMovingOnGround())
    {
        if (!SettingsRestored(Player) || Power->HasPower() || OrbitPoint < 4) { Error = TEXT("Expiry did not finish a steered loop and actual gravity landing"); return false; }
        Route = {FVector(-3000, -900, 82), FVector(-3000, 2800, 82), Stage->GetFlightBloom()->GetActorLocation() + FVector(0, 0, 82)}; SetPhase(8);
    }
    else if (Phase == 9)
    {
        if (WalkRoute(Player, DeltaSeconds)) { FlightTap(PC, EKeys::G); SetPhase(10); }
    }
    else if (Phase == 10 && PhaseTime > .3f)
    {
        if (!Player->GetLaunch()->IsAiming() || !Power->HasPower()) { Error = TEXT("Reacquired power was unavailable at the launcher"); return false; }
        FlightTap(PC, EKeys::LeftMouseButton); SetPhase(11);
    }
    else if (Phase == 11 && PhaseTime > .3f)
    {
        if (!Power->IsFlying() || Power->GetFlightStarts() != StartsBefore + 2 || Player->GetLaunch()->GetLaunchCount() != LaunchesBefore + 1) { Error = TEXT("Actual cannon launch did not enter powered flight after PendingLaunch"); return false; }
        FlightKey(PC, EKeys::S, true); FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderPoweredLaunch.png"), false, false); SetPhase(12);
    }
    else if (Phase == 12)
    {
        const FVector Approach(-3000, 1200, 1200); SteerTo(Player, Approach);
        if (!Power->IsFlying() || !Power->HasPower()) { Error = TEXT("Powered launch ended before controlled approach"); return false; }
        if (FVector::Dist(Player->GetActorLocation(), Approach) < 220) SetPhase(13);
    }
    else if (Phase == 13)
    {
        if (Power->IsFlying()) SteerTo(Player, FVector(-3000, 0, 82));
        else if (Move->IsMovingOnGround())
        {
            ClearAxes(Player);
            if (!Power->HasPower() || Power->GetFlightLandings() != LandingsBefore + 1 || !SettingsRestored(Player) || FMath::Abs(Player->GetActorLocation().Z - 82) > 6) { Error = TEXT("Real floor collision did not end powered flight and preserve remaining charge"); return false; }
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderFlightLanding.png"), false, false); SetPhase(14);
        }
    }
    else if (Phase == 14 && PhaseTime > .5f) { FlightTap(PC, EKeys::R); SetPhase(15); }
    else if (Phase == 15 && PhaseTime > .4f)
    {
        if (Power->HasPower() || !SettingsRestored(Player) || Player->RecoveryCount != RecoveryBefore + 1 || !Stage->GetFlightBloom()->IsAvailable()) { Error = TEXT("R did not clear charge and restore bloom and normal controls"); return false; }
        UE_LOG(LogTemp, Display, TEXT("FLIGHT TEST PASS: actual bloom pickup, ordinary jump and air Space, mouse steering loop, W/S speed, actual45s expiry and gravity landing, reacquisition, cannon-powered flight, real floor landing and R cleanup passed; orbit waypoints %d."), OrbitPoint);
        return true;
    }
    return false;
}
