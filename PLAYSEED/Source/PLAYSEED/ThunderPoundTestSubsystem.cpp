#include "ThunderPoundTestSubsystem.h"
#include "PlayseedActionComponent.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace
{
void PoundPress(APlayerController* PC, FKey Key, EInputEvent Event)
{
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, Event, Event == IE_Pressed ? 1 : 0));
}
void PoundTap(APlayerController* PC, FKey Key) { PoundPress(PC, Key, IE_Pressed); PoundPress(PC, Key, IE_Released); }
}

void UThunderPoundTestSubsystem::BeginTest(AWindseedCharacter* Player)
{
    Error.Reset();
    Phase = 0;
    PhaseTime = 0;
    PoundBaseline = Player->GetActions()->GetCompletedPounds();
    RecoveryBaseline = Player->RecoveryCount;
    HitBaseline = Player->GetVitality()->GetSuccessfulImpacts();
    ObservedImpacts = 0;
    NormalGravity = Player->GetCharacterMovement()->GravityScale;
    NormalAirControl = Player->GetCharacterMovement()->AirControl;
    bSnapshotSaved = false;
    Player->GetActions()->OnPoundImpact.AddUObject(this, &UThunderPoundTestSubsystem::RecordImpact);
    if (auto* PC = Cast<APlayerController>(Player->GetController())) PoundTap(PC, EKeys::LeftControl);
}

void UThunderPoundTestSubsystem::RecordImpact(FVector Position)
{
    ++ObservedImpacts;
    UE_LOG(LogTemp, Display, TEXT("POUND TEST: received landing notification %d at %s."), ObservedImpacts, *Position.ToString());
}

bool UThunderPoundTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 15) { Error = FString::Printf(TEXT("Pound phase %d timed out at %s"), Phase, *Player->GetActorLocation().ToString()); return false; }
    auto* Actions = Player->GetActions();
    auto* Movement = Player->GetCharacterMovement();
    auto* PC = Cast<APlayerController>(Player->GetController());
    if (!PC) { Error = TEXT("Pound test controller is missing"); return false; }
    auto Advance = [this](int32 Next) { Phase = Next; PhaseTime = 0; };
    auto SettingsRestored = [&]() { return FMath::IsNearlyEqual(Movement->GravityScale, NormalGravity) && FMath::IsNearlyEqual(Movement->AirControl, NormalAirControl); };
    if (Phase == 0 && PhaseTime > .3f)
    {
        if (Actions->IsGroundPounding() || Actions->GetCompletedPounds() != PoundBaseline || ObservedImpacts != 0)
        { Error = TEXT("Grounded Ctrl incorrectly started a pound"); return false; }
        PoundPress(PC, EKeys::SpaceBar, IE_Pressed);
        Advance(1);
    }
    else if (Phase == 1 || Phase == 5)
    {
        if (PhaseTime > .15f) PoundPress(PC, EKeys::SpaceBar, IE_Released);
        if (Movement->IsFalling() && Player->GetActorLocation().Z > 250)
        {
            PoundTap(PC, Phase == 1 ? EKeys::LeftControl : EKeys::Gamepad_RightShoulder);
            Advance(Phase == 1 ? 2 : 6);
        }
    }
    else if (Phase == 2 && PhaseTime > .05f)
    {
        if (!Actions->IsGroundPounding() || Player->GetVelocity().Z > -900 || Player->GetVelocity().Size2D() > 30)
        { Error = TEXT("Airborne Ctrl did not start a vertical dive"); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderGroundPound.png"), false, false);
        Advance(3);
    }
    else if (Phase == 3 && Movement->IsMovingOnGround())
    {
        if (Actions->IsGroundPounding() || Actions->GetCompletedPounds() != PoundBaseline + 1 || ObservedImpacts != 1 || !SettingsRestored())
        { Error = TEXT("Pound landing did not notify exactly once and restore movement"); return false; }
        Advance(4);
    }
    else if (Phase == 4 && PhaseTime > .4f)
    {
        if (ObservedImpacts != 1) { Error = TEXT("Landing notification repeated on later ticks"); return false; }
        PoundPress(PC, EKeys::SpaceBar, IE_Pressed);
        Advance(5);
    }
    else if (Phase == 6 && PhaseTime > .03f)
    {
        if (!Actions->IsGroundPounding()) { Error = TEXT("Gamepad RB binding did not begin the dive"); return false; }
        PoundTap(PC, EKeys::R);
        Advance(7);
    }
    else if (Phase == 7 && PhaseTime > .3f)
    {
        if (Actions->IsGroundPounding() || !SettingsRestored() || ObservedImpacts != 1 || Player->RecoveryCount != RecoveryBaseline + 1)
        { Error = TEXT("R did not cancel the dive without a landing event"); return false; }
        Advance(8);
    }
    else if (Phase == 8)
    {
        const FVector Offset = FVector(-250, -1200, 82) - Player->GetActorLocation();
        if (Offset.Size2D() < 30 && Movement->IsMovingOnGround())
        {
            Movement->StopMovementImmediately();
            PoundPress(PC, EKeys::SpaceBar, IE_Pressed);
            Advance(9);
        }
        else Player->AddMovementInput(Offset.GetSafeNormal2D());
    }
    else if (Phase == 9)
    {
        const FVector Offset = FVector(0, -1200, 82) - Player->GetActorLocation();
        Player->AddMovementInput(Offset.GetSafeNormal2D());
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, 0, 0), DeltaSeconds, 3));
        if (PhaseTime > .15f) PoundPress(PC, EKeys::SpaceBar, IE_Released);
        const float FeetZ = Player->GetActorLocation().Z - Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        if (Offset.Size2D() < 70 && FeetZ > 150) { PoundTap(PC, EKeys::LeftControl); Advance(10); }
    }
    else if (Phase == 10)
    {
        // イバラへ空中から実際に接触し、被弾時に急降下が解除されることを確かめます。
        if (Player->GetVitality()->GetSuccessfulImpacts() == HitBaseline + 1)
        {
            if (Actions->IsGroundPounding() || !SettingsRestored() || ObservedImpacts != 1)
            { Error = TEXT("Real damage did not cancel the dive without a landing event"); return false; }
            Advance(11);
        }
    }
    else if (Phase == 11 && PhaseTime > 1.3f && Movement->IsMovingOnGround())
    {
        if (Actions->GetCompletedPounds() != PoundBaseline + 1 || ObservedImpacts != 1)
        { Error = TEXT("Cancelled dive later produced an impact"); return false; }
        PoundTap(PC, EKeys::R);
        Advance(12);
    }
    else if (Phase == 12 && PhaseTime > 1)
    {
        if (!SettingsRestored() || Actions->IsGroundPounding() || Player->GetVitality()->GetHealth() != 8 || Player->RecoveryCount != RecoveryBaseline + 2)
        { Error = TEXT("Final R did not restore the normal character"); return false; }
        if (!bSnapshotSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderPoundRecovery.png"), false, false);
            bSnapshotSaved = true;
            UE_LOG(LogTemp, Display, TEXT("POUND TEST PASS: grounded rejection, Ctrl/RB dives, single landing event, R and real-contact damage cancellation, normal settings restored."));
        }
        return PhaseTime > 2;
    }
    return false;
}
