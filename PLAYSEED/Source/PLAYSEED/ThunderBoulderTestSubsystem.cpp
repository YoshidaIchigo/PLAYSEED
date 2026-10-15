#include "ThunderBoulderTestSubsystem.h"
#include "ThunderRollingBoulder.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UThunderBoulderTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderRollingBoulder* InBoulder, const TArray<FVector>& FootPath, FVector ThistlePosition)
{
    Boulder = InBoulder;
    MountainPath = FootPath;
    Thistle = ThistlePosition;
    Route = {FVector(0, 1100, 82), FVector(600, 1850, 82)};
    for (int32 Index = 0; Index <= 3; ++Index) Route.Add(FootPath[Index] + FVector(0, 0, 82));
    Phase = -2;
    Waypoint = 0;
    PhaseTime = 0;
    Error.Reset();
    DamageBaseline = Player->GetVitality()->GetSuccessfulImpacts();
    BoulderHitBaseline = Boulder->GetConfirmedHits();
    RecoveryBaseline = Player->RecoveryCount;
    DefeatBaseline = Player->GetVitality()->GetDefeatCount();
    bImpactSaved = bReturnSaved = false;
}

bool UThunderBoulderTestSubsystem::WalkRoute(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (Waypoint >= Route.Num()) { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    const FVector Offset = Route[Waypoint] - Player->GetActorLocation();
    if (Offset.Size2D() < 25 && FMath::Abs(Offset.Z) < 65 && Player->GetCharacterMovement()->IsMovingOnGround())
    { ++Waypoint; Player->GetCharacterMovement()->StopMovementImmediately(); return Waypoint >= Route.Num(); }
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (auto* PC = Cast<APlayerController>(Player->GetController())) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-18, Offset.Rotation().Yaw, 0), DeltaSeconds, 3));
    return false;
}

bool UThunderBoulderTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* PC = Cast<APlayerController>(Player->GetController());
    if (!Boulder.IsValid() || !PC) { Error = TEXT("Boulder test actor or controller missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 45) { Error = FString::Printf(TEXT("Boulder phase %d timed out at waypoint %d, %s"), Phase, Waypoint, *Player->GetActorLocation().ToString()); return false; }
    auto Next = [this](int32 Number) { Phase = Number; PhaseTime = 0; };
    if (Phase == -2)
    {
        // 体力を直接書かず、既存のトゲへ三度触れて2点にしてから岩の致命傷を試します。
        if (Player->GetVitality()->GetSuccessfulImpacts() == DamageBaseline + 3)
        {
            if (Player->GetVitality()->GetHealth() != 2) { Error = TEXT("Three real thistle hits did not leave two health"); return false; }
            Player->GetCharacterMovement()->StopMovementImmediately();
            Next(-1);
        }
        else
        {
            const FVector PreludeGoal = Player->GetActorLocation().X < -450 && Player->GetActorLocation().Y > -1150 ? FVector(-600, -1200, 82) : Thistle;
            Player->AddMovementInput((PreludeGoal - Player->GetActorLocation()).GetSafeNormal2D());
        }
    }
    else if (Phase == -1)
    {
        if (Player->GetVitality()->GetDefeatCount() == DefeatBaseline + 1)
        {
            if (Player->RecoveryCount != RecoveryBaseline + 1 || Player->GetVitality()->GetHealth() != 8 || Boulder->GetConfirmedHits() != BoulderHitBaseline + 1)
            { Error = TEXT("Real fatal boulder contact did not recover once"); return false; }
            Next(-3);
        }
        else WalkRoute(Player, DeltaSeconds);
    }
    else if (Phase == -3 && PhaseTime > .05f)
    {
        if (FVector::Dist2D(Player->GetActorLocation(), FVector(-600, -600, 100)) > 5 || Boulder->GetTravelledDistance() > 150 || Boulder->IsWaiting())
        { Error = TEXT("Fatal-contact reset was overwritten by the old boulder tick"); return false; }
        DamageBaseline = Player->GetVitality()->GetSuccessfulImpacts();
        BoulderHitBaseline = Boulder->GetConfirmedHits();
        RecoveryBaseline = Player->RecoveryCount;
        Waypoint = 0;
        UE_LOG(LogTemp, Display, TEXT("BOULDER FATAL RESET TEST PASS: three real thistle hits, fatal rolling contact, one defeat and player/boulder positions remained reset."));
        Next(0);
    }
    else if (Phase == 0)
    {
        if (Player->GetVitality()->GetSuccessfulImpacts() == DamageBaseline + 1)
        {
            if (Player->GetVitality()->GetHealth() != 6 || Boulder->GetConfirmedHits() != BoulderHitBaseline + 1 || Player->GetVitality()->GetLastImpulse().Size2D() < 400)
            { Error = TEXT("Real rolling contact did not damage and knock back once"); return false; }
            Next(1);
        }
        else WalkRoute(Player, DeltaSeconds);
    }
    else if (Phase == 1)
    {
        PC->SetControlRotation(FRotator(-15, (Boulder->GetActorLocation() - Player->GetActorLocation()).Rotation().Yaw, 0));
        if (PhaseTime > .04f && !bImpactSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderBoulderImpact.png"), false, false);
            bImpactSaved = true;
        }
        if (PhaseTime > .25f)
        {
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R, IE_Pressed, 1));
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R, IE_Released, 0));
            Next(2);
        }
    }
    else if (Phase == 2 && PhaseTime > .05f)
    {
        if (Player->RecoveryCount != RecoveryBaseline + 1 || Player->GetVitality()->GetHealth() != 8 || Boulder->IsWaiting() || Boulder->GetTravelledDistance() > 150 || FVector::Dist2D(Player->GetActorLocation(), FVector(-600, -600, 100)) > 5)
        { Error = TEXT("R did not restore player and restart the boulder path"); return false; }
        Route = {FVector(0, 1100, 82), FVector(600, 1850, 82)};
        const FVector MountainCenter(2200, 2100, 0);
        for (int32 Index = 0; Index <= 19; ++Index)
        {
            // 道幅420cmの端を普通に歩きます。中央の岩球から150cm離して上へ進みます。
            const FVector Outward = (MountainPath[Index] - MountainCenter).GetSafeNormal2D();
            Route.Add(MountainPath[Index] + Outward * 150 + FVector(0, 0, 82));
        }
        Waypoint = 0;
        UE_LOG(LogTemp, Display, TEXT("BOULDER TEST: real contact dealt two points and knockback; R restored health, start and path."));
        Next(3);
    }
    else if (Phase == 3)
    {
        if (Player->RecoveryCount != RecoveryBaseline + 1 || Player->GetVitality()->GetHealth() != 8)
        { Error = TEXT("Walking the edge of the boulder path was not safe"); return false; }
        if (WalkRoute(Player, DeltaSeconds)) { RespawnBaseline = Boulder->GetRespawnCount(); Next(4); }
    }
    else if (Phase == 4)
    {
        if (Boulder->IsWaiting() && Boulder->GetWaitRemaining() == 0)
        {
            RespawnBaseline = Boulder->GetRespawnCount();
            Next(5);
        }
    }
    else if (Phase == 5)
    {
        if (!Boulder->IsWaiting() || Boulder->GetRespawnCount() != RespawnBaseline || Player->GetVitality()->GetHealth() != 8)
        { Error = TEXT("Boulder respawned while the player occupied its source"); return false; }
        if (PhaseTime > 2)
        {
            const FVector MountainCenter(2200, 2100, 0);
            Route = {MountainPath[20] + (MountainPath[20] - MountainCenter).GetSafeNormal2D() * 150 + FVector(0, 0, 82)};
            Waypoint = 0;
            Next(6);
        }
    }
    else if (Phase == 6)
    {
        if (WalkRoute(Player, DeltaSeconds)) Next(7);
    }
    else if (Phase == 7)
    {
        if (Boulder->GetRespawnCount() != RespawnBaseline + 1 || Player->GetVitality()->GetHealth() != 8 || Player->RecoveryCount != RecoveryBaseline + 1 || Boulder->IsWaiting())
        { Error = TEXT("Leaving the source did not permit one safe respawn"); return false; }
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, (Boulder->GetActorLocation() - Player->GetActorLocation()).Rotation().Yaw, 0), DeltaSeconds, 3));
        if (PhaseTime > .4f && !bReturnSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderBoulderReturn.png"), false, false);
            bReturnSaved = true;
            UE_LOG(LogTemp, Display, TEXT("BOULDER TEST PASS: real rolling contact, two damage and knockback, R reset, normal walk up the edge, source occupied for two seconds without respawn, leave and one safe respawn."));
        }
        return PhaseTime > 1.2f;
    }
    return false;
}
