#include "ThunderWarpTestSubsystem.h"
#include "ThunderWarpPair.h"
#include "WindseedCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UThunderWarpTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderWarpPair* InPair, const TArray<FVector>& FootPath)
{
    Pair = InPair;
    Trail = FootPath;
    Phase = 0;
    PhaseTime = 0;
    RecoveryBaseline = Player->RecoveryCount;
    Error.Reset();
    bRestartVerified = false;
    bSummitReached = false;
    bSnapshotSaved = false;
    SetLowerRoute();
    if (auto* PC = Cast<APlayerController>(Player->GetController()))
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseWheelAxis, IE_Axis, -6));
}

void UThunderWarpTestSubsystem::SetLowerRoute()
{
    Route = {FVector(0, 1100, 82), FVector(600, 1850, 82)};
    for (int32 Index = 0; Index <= 2; ++Index) Route.Add(Trail[Index] + FVector(0, 0, 82));
    Route.Add(Pair->GetCaveFloor(0) + FVector(0, 0, 82));
    Waypoint = 0;
}

void UThunderWarpTestSubsystem::SetUpperRoute()
{
    Route.Reset();
    // 上の洞窟から山頂まで歩き、同じ道を戻って上側の入口へ入ります。
    for (int32 Index = 22; Index < Trail.Num(); ++Index) Route.Add(Trail[Index] + FVector(0, 0, 82));
    for (int32 Index = Trail.Num() - 2; Index >= 22; --Index) Route.Add(Trail[Index] + FVector(0, 0, 82));
    Route.Add(Pair->GetCaveFloor(1) + FVector(0, 0, 82));
    Waypoint = 0;
}

bool UThunderWarpTestSubsystem::WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    if (Offset.Size2D() < 45 && FMath::Abs(Offset.Z) < 55 && Player->GetCharacterMovement()->IsMovingOnGround())
    { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (auto* PC = Cast<APlayerController>(Player->GetController()))
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-22, Offset.Rotation().Yaw, 0), DeltaSeconds, 2));
    return false;
}

bool UThunderWarpTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    if (!Pair.IsValid()) { Error = TEXT("Warp pair is missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 35) { Error = FString::Printf(TEXT("Warp phase %d waypoint %d timed out at %s"), Phase, Waypoint, *Player->GetActorLocation().ToString()); return false; }
    if (Pair->HasFailedWarp()) { Error = TEXT("Warp destination collision prevented arrival"); return false; }
    if (Phase != 4 && Player->RecoveryCount != RecoveryBaseline) { Error = TEXT("Unexpected fall recovery on warp route"); return false; }
    auto* PC = Cast<APlayerController>(Player->GetController());
    auto Advance = [this](int32 Next) { Phase = Next; PhaseTime = 0; };
    if (Phase == 0)
    {
        if (Pair->GetWarpCount() == 1)
        {
            Player->GetCharacterMovement()->StopMovementImmediately();
            Advance(1);
        }
        else if (Waypoint < Route.Num() && WalkToward(Player, Route[Waypoint], DeltaSeconds)) ++Waypoint;
    }
    else if (Phase == 1 || Phase == 3)
    {
        const int32 Expected = Phase == 1 ? 1 : 2;
        const int32 CaveIndex = Phase == 1 ? 1 : 0;
        if (Pair->GetWarpCount() != Expected || FVector::Dist2D(Player->GetActorLocation(), Pair->GetCaveFloor(CaveIndex)) > 130)
        { Error = TEXT("Warp bounced back without leaving the destination"); return false; }
        if (PhaseTime > 2)
        {
            UE_LOG(LogTemp, Display, TEXT("WARP TEST: arrival %d remained stable beyond cooldown."), Expected);
            if (Phase == 1 && bRestartVerified) Advance(5);
            else if (Phase == 1) { SetUpperRoute(); Advance(2); }
            else if (PC)
            {
                PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R, IE_Pressed, 1));
                PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R, IE_Released, 0));
                Advance(4);
            }
        }
    }
    else if (Phase == 2)
    {
        if (Pair->GetWarpCount() == 2)
        {
            if (!bSummitReached) { Error = TEXT("Upper route skipped the summit"); return false; }
            Player->GetCharacterMovement()->StopMovementImmediately();
            Advance(3);
        }
        else if (Waypoint < Route.Num() && WalkToward(Player, Route[Waypoint], DeltaSeconds))
        {
            if (Waypoint == 4) { bSummitReached = true; UE_LOG(LogTemp, Display, TEXT("WARP TEST: walked from upper cave to summit.")); }
            ++Waypoint;
        }
    }
    else if (Phase == 4 && PhaseTime > .3f)
    {
        if (Pair->GetWarpCount() != 0 || Player->RecoveryCount != RecoveryBaseline + 1 || FVector::Dist2D(Player->GetActorLocation(), FVector(-600, -600, 100)) > 5)
        { Error = TEXT("R did not reset warp state and recover"); return false; }
        RecoveryBaseline = Player->RecoveryCount;
        bRestartVerified = true;
        SetLowerRoute();
        Advance(0);
    }
    else if (Phase == 5)
    {
        if (WalkToward(Player, Trail[22] + FVector(0, 0, 82), DeltaSeconds)) Advance(6);
    }
    else if (Phase == 6)
    {
        if (PC)
        {
            const float Yaw = (Pair->GetCaveFloor(1) - Player->GetActorLocation()).Rotation().Yaw;
            PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-10, Yaw, 0), DeltaSeconds, 3));
        }
        if (PhaseTime > 1 && !bSnapshotSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderWarpCaves.png"), false, false);
            bSnapshotSaved = true;
            UE_LOG(LogTemp, Display, TEXT("WARP TEST PASS: ordinary trail approach, both directions, no arrival bounce, summit walk, R reset and reuse passed."));
        }
        return PhaseTime > 2;
    }
    return false;
}
