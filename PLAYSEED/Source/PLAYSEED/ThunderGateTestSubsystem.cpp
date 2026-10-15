#include "ThunderGateTestSubsystem.h"
#include "ThunderShortcutGate.h"
#include "WindseedCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UThunderGateTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderShortcutGate* InGate)
{
    Gate = InGate;
    Phase = 0;
    PhaseTime = 0;
    RecoveryBaseline = Player->RecoveryCount;
    Error.Reset();
    bRestartVerified = false;
    bSnapshotSaved = false;
    if (auto* PC = Cast<APlayerController>(Player->GetController()))
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseWheelAxis, IE_Axis, -6));
}

bool UThunderGateTestSubsystem::WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    if (Offset.Size2D() < 45 && Player->GetCharacterMovement()->IsMovingOnGround())
    { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    // 柵の検証ではジャンプやテレポートを使わず、開閉による歩行の違いを確かめます。
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (auto* PC = Cast<APlayerController>(Player->GetController()))
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, Offset.Rotation().Yaw, 0), DeltaSeconds, 2));
    return false;
}

bool UThunderGateTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    if (!Gate.IsValid()) { Error = TEXT("Shortcut gate is missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 20) { Error = FString::Printf(TEXT("Gate phase %d timed out at %s"), Phase, *Player->GetActorLocation().ToString()); return false; }
    if (Phase != 5 && Player->RecoveryCount != RecoveryBaseline) { Error = TEXT("Unexpected recovery during shortcut test"); return false; }
    auto Advance = [this](int32 Next) { Phase = Next; PhaseTime = 0; };
    auto* PC = Cast<APlayerController>(Player->GetController());
    if (Phase == 0)
    {
        if (WalkToward(Player, Gate->GetActorLocation() + FVector(-200, 0, 82), DeltaSeconds)) Advance(1);
    }
    else if (Phase == 1)
    {
        WalkToward(Player, Gate->GetActorLocation() + FVector(300, 0, 82), DeltaSeconds);
        if (PhaseTime > 1.2f)
        {
            const float X = Player->GetActorLocation().X - Gate->GetActorLocation().X;
            if (Gate->IsOpen() || X > -50 || X < -100) { Error = TEXT("Closed gate did not block normal walking"); return false; }
            UE_LOG(LogTemp, Display, TEXT("GATE TEST: closed gate blocked the player %.1f cm before its centre."), -X);
            Advance(2);
        }
    }
    else if (Phase == 2)
    {
        if (WalkToward(Player, Gate->GetSwitchTop() + FVector(0, 0, 82), DeltaSeconds) && Gate->IsOpen()) Advance(3);
    }
    else if (Phase == 3 && PhaseTime > .8f)
    {
        if (!Gate->IsOpen() || Gate->GetActivationCount() != 1) { Error = TEXT("Pressure switch activated more or less than once"); return false; }
        Advance(4);
    }
    else if (Phase == 4)
    {
        if (WalkToward(Player, Gate->GetActorLocation() + FVector(300, 0, 82), DeltaSeconds))
        {
            if (!bRestartVerified && PC)
            {
                PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R, IE_Pressed, 1));
                PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R, IE_Released, 0));
                Advance(5);
            }
            else Advance(6);
        }
    }
    else if (Phase == 5 && PhaseTime > .3f)
    {
        if (Gate->IsOpen() || Gate->GetActivationCount() != 0 || Player->RecoveryCount != RecoveryBaseline + 1 || FVector::Dist2D(Player->GetActorLocation(), FVector(-600, -600, 100)) > 5)
        { Error = TEXT("R did not close the gate and reset switch progress"); return false; }
        RecoveryBaseline = Player->RecoveryCount;
        bRestartVerified = true;
        UE_LOG(LogTemp, Display, TEXT("GATE TEST: R closed the gate; checking collision and switch again."));
        Advance(0);
    }
    else if (Phase == 6)
    {
        if (PC) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-18, 210, 0), DeltaSeconds, 3));
        if (PhaseTime > 1 && !bSnapshotSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderShortcutGate.png"), false, false);
            bSnapshotSaved = true;
            UE_LOG(LogTemp, Display, TEXT("GATE TEST PASS: closed collision, pressure activation, passage, R reset and second activation passed."));
        }
        return PhaseTime > 2;
    }
    return false;
}
