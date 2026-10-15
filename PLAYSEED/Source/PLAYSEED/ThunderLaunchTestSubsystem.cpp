#include "ThunderLaunchTestSubsystem.h"
#include "ThunderGuideTestSubsystem.h"
#include "ThunderMeadowStage.h"
#include "ThunderLauncher.h"
#include "ThunderExpeditionSubsystem.h"
#include "PlayseedLaunchComponent.h"
#include "PlayseedCarryComponent.h"
#include "PlayseedRideComponent.h"
#include "PlayseedClimbComponent.h"
#include "PlayseedCombatComponent.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace
{
void LaunchKey(APlayerController* PC, FKey Key, bool bPressed) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, bPressed ? IE_Pressed : IE_Released, bPressed ? 1 : 0)); }
void LaunchTap(APlayerController* PC, FKey Key) { LaunchKey(PC, Key, true); LaunchKey(PC, Key, false); }
}
void UThunderLaunchTestSubsystem::SetPhase(int32 Number)
{
    Phase = Number; PhaseTime = 0; Waypoint = 0;
    UE_LOG(LogTemp, Display, TEXT("LAUNCH TEST: phase %d"), Number);
}
void UThunderLaunchTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage)
{
    Stage = InStage; Error.Reset(); SetPhase(0);
    LaunchBefore = Player->GetLaunch()->GetLaunchCount();
    NormalArm = Player->GetCameraBoom()->TargetArmLength; NormalSocket = Player->GetCameraBoom()->SocketOffset;
    auto* PC = Cast<APlayerController>(Player->GetController());
    NormalPitchMin = PC->PlayerCameraManager->ViewPitchMin; NormalPitchMax = PC->PlayerCameraManager->ViewPitchMax;
    FlightDistance = HighestFeet = 0; bFlightScreenshot = false;
    GetGameInstance()->GetSubsystem<UThunderGuideTestSubsystem>()->BeginTest(Player, InStage, false);
}
bool UThunderLaunchTestSubsystem::CameraRestored(AWindseedCharacter* Player) const
{
    auto* PC = Cast<APlayerController>(Player->GetController());
    return FMath::IsNearlyEqual(Player->GetCameraBoom()->TargetArmLength, NormalArm) && Player->GetCameraBoom()->SocketOffset.Equals(NormalSocket, .1f)
        && FMath::IsNearlyEqual(PC->PlayerCameraManager->ViewPitchMin, NormalPitchMin) && FMath::IsNearlyEqual(PC->PlayerCameraManager->ViewPitchMax, NormalPitchMax);
}
bool UThunderLaunchTestSubsystem::WalkToPad(AWindseedCharacter* Player, float DeltaSeconds)
{
    const FVector PadApproach[] = {FVector(-2800, -600, 82), FVector(-3000, 1500, 82), Stage->GetLauncher()->GetActorLocation() + FVector(-140, 0, 82)};
    if (Waypoint >= 3) return true;
    const FVector Offset = PadApproach[Waypoint] - Player->GetActorLocation();
    if (Offset.Size2D() < 20 && Player->GetCharacterMovement()->IsMovingOnGround()) { Player->GetCharacterMovement()->StopMovementImmediately(); ++Waypoint; }
    else
    {
        Player->AddMovementInput(Offset.GetSafeNormal2D());
        auto* PC = Cast<APlayerController>(Player->GetController());
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, Offset.Rotation().Yaw, 0), DeltaSeconds, 3));
    }
    return Waypoint >= 3;
}
bool UThunderLaunchTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* PC = Cast<APlayerController>(Player->GetController());
    if (!Stage.IsValid() || !PC) { Error = TEXT("Launch stage/controller missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 60) { Error = FString::Printf(TEXT("Launch phase %d waypoint %d timed out at %s"), Phase, Waypoint, *Player->GetActorLocation().ToString()); return false; }
    if (Player->GetVitality()->GetHealth() != 8) { Error = TEXT("Launch route unexpectedly took damage"); return false; }
    auto* Launch = Player->GetLaunch();
    auto* Move = Player->GetCharacterMovement();
    if (Phase == 0)
    {
        auto* Guide = GetGameInstance()->GetSubsystem<UThunderGuideTestSubsystem>();
        const bool bDone = Guide->UpdateTest(Player, DeltaSeconds);
        if (!Guide->GetError().IsEmpty()) { Error = Guide->GetError(); return false; }
        if (bDone) { RecoveryBefore = Player->RecoveryCount; SetPhase(1); }
    }
    else if (Phase == 1 || Phase == 9)
    {
        if (WalkToPad(Player, DeltaSeconds))
        {
            Entry = Player->GetActorLocation(); LaunchTap(PC, EKeys::G); SetPhase(Phase == 1 ? 3 : 10);
        }
    }
    else if (Phase == 3 && PhaseTime > .3f)
    {
        if (!Launch->IsAiming() || Move->MovementMode != MOVE_None || Player->GetCameraBoom()->TargetArmLength != 0 || FMath::Abs(Launch->GetAimPitch() - 70) > .1f)
        { Error = TEXT("Nearby G did not enter default aim and freeze movement"); return false; }
        Seat = Player->GetActorLocation(); BeforeYaw = Launch->GetAimYaw();
        LaunchKey(PC, EKeys::W, true); LaunchTap(PC, EKeys::SpaceBar); LaunchTap(PC, EKeys::E); LaunchTap(PC, EKeys::F); LaunchTap(PC, EKeys::Q);
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseX, IE_Axis, 30));
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseY, IE_Axis, -1000));
        SetPhase(4);
    }
    else if (Phase == 4 && PhaseTime > .3f)
    {
        LaunchKey(PC, EKeys::W, false);
        if (!Launch->IsAiming() || Move->MovementMode != MOVE_None || FVector::Dist(Seat, Player->GetActorLocation()) > 2 || Player->GetRide()->IsRiding() || Player->GetClimb()->IsClimbing() || Player->GetCarry()->GetHeldObject() || Player->GetCombat()->IsAttacking() || FMath::Abs(FRotator::NormalizeAxis(Launch->GetAimYaw() - BeforeYaw)) < .1f || !FMath::IsNearlyEqual(Launch->GetAimPitch(), 80.f, .1f))
        { Error = FString::Printf(TEXT("Aim input/freeze/limits failed: yaw %.2f pitch %.2f position %s"), Launch->GetAimYaw(), Launch->GetAimPitch(), *Player->GetActorLocation().ToString()); return false; }
        LaunchTap(PC, EKeys::G); SetPhase(5);
    }
    else if (Phase == 5 && PhaseTime > .4f)
    {
        if (Launch->IsAiming() || !CameraRestored(Player) || FVector::Dist2D(Player->GetActorLocation(), Entry) > 5) { Error = TEXT("G exit did not restore entry and camera"); return false; }
        LaunchTap(PC, EKeys::G); SetPhase(6);
    }
    else if (Phase == 6 && PhaseTime > .3f)
    {
        if (!Launch->IsAiming()) { Error = TEXT("Re-entering aim failed"); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderLauncherAim.png"), false, false);
        SetPhase(7);
    }
    else if (Phase == 7 && PhaseTime > .5f) { LaunchTap(PC, EKeys::R); SetPhase(8); }
    else if (Phase == 8 && PhaseTime > .4f)
    {
        if (Launch->IsAiming() || !CameraRestored(Player) || Player->RecoveryCount != RecoveryBefore + 1 || Launch->GetLaunchCount() != LaunchBefore || !Move->IsMovingOnGround()) { Error = TEXT("R during aim retained camera/movement state or caused a launch"); return false; }
        RecoveryBefore = Player->RecoveryCount; SetPhase(9);
    }
    else if (Phase == 10 && PhaseTime > .4f)
    {
        if (!Launch->IsAiming() || FMath::Abs(Launch->GetAimPitch() - 70) > .1f || FMath::Abs(Launch->GetAimYaw() - Stage->GetLauncher()->GetDefaultAimYaw()) > .1f) { Error = TEXT("New aim did not restore its default angles"); return false; }
        LaunchTap(PC, EKeys::LeftMouseButton); SetPhase(11);
    }
    else if (Phase == 11 && PhaseTime > .2f)
    {
        if (Launch->IsAiming() || Launch->GetLaunchCount() != LaunchBefore + 1 || !CameraRestored(Player) || !Move->IsFalling() || Player->GetActorLocation().Z < Launch->GetLastLaunchOrigin().Z + 100 || FVector::Dist(Launch->GetLastLaunchOrigin(), Stage->GetLauncher()->GetSeatLocation()) > 5)
        { Error = TEXT("LMB did not make one actual airborne launch and restore the camera"); return false; }
        LastFlightPosition = Launch->GetLastLaunchOrigin(); SetPhase(12);
    }
    else if (Phase == 12)
    {
        FlightDistance += FVector::Dist(LastFlightPosition, Player->GetActorLocation()); LastFlightPosition = Player->GetActorLocation();
        HighestFeet = FMath::Max(HighestFeet, float(Player->GetActorLocation().Z - 82));
        if (!bFlightScreenshot && HighestFeet > 1600) { FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderSkybloomFlight.png"), false, false); bFlightScreenshot = true; }
        if (Move->IsMovingOnGround())
        {
            const FVector Top = Stage->GetSkyIslandTop();
            if (FMath::Abs(Player->GetActorLocation().Z - 82 - Top.Z) > 6 || FVector::Dist2D(Player->GetActorLocation(), Top) > 450 || HighestFeet < 1700 || FlightDistance < 2000 || Player->RecoveryCount != RecoveryBefore)
            { Error = FString::Printf(TEXT("Actual flight did not land on island: %s, peak %.1f, distance %.1f"), *Player->GetActorLocation().ToString(), HighestFeet, FlightDistance); return false; }
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderSkyIslandLanding.png"), false, false);
            UE_LOG(LogTemp, Display, TEXT("LAUNCH LANDING: %s; peak feet %.1f; continuous flight distance %.1f; velocity %s"), *Player->GetActorLocation().ToString(), HighestFeet, FlightDistance, *Launch->GetLastLaunchVelocity().ToString());
            SetPhase(13);
        }
    }
    else if (Phase == 13 && PhaseTime > .6f) { LaunchTap(PC, EKeys::R); SetPhase(14); }
    else if (Phase == 14 && PhaseTime > .4f)
    {
        if (Launch->IsAiming() || !CameraRestored(Player) || Player->RecoveryCount != RecoveryBefore + 1 || Launch->GetLaunchCount() != LaunchBefore + 1 || !GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->IsLauncherUnlocked()) { Error = TEXT("R after island landing did not preserve unlock and restore normal play"); return false; }
        UE_LOG(LogTemp, Display, TEXT("LAUNCH TEST PASS: actual fresh guide unlock and walking, G enter/exit, mouse angle/clamp and movement freeze, R during aim, one normal gravity launch, continuous flight and actual island landing, camera and R restoration passed."));
        return true;
    }
    return false;
}
