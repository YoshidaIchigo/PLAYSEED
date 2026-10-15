#include "ThunderBoxTestSubsystem.h"
#include "PlayseedBreakableBox.h"
#include "PlayseedCoin.h"
#include "PlayseedActionComponent.h"
#include "ThunderCoinLedger.h"
#include "ThunderMeadowGameMode.h"
#include "WindseedCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace
{
void BoxKey(APlayerController* PC, FKey Key, bool bPressed)
{
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, bPressed ? IE_Pressed : IE_Released, bPressed ? 1 : 0));
}
void BoxTap(APlayerController* PC, FKey Key) { BoxKey(PC, Key, true); BoxKey(PC, Key, false); }
}

void UThunderBoxTestSubsystem::BeginTest(AWindseedCharacter* Player, APlayseedBreakableBox* InBox)
{
    Box = InBox;
    Phase = DropIndex = 0;
    PhaseTime = 0;
    Error.Reset();
    BreakBaseline = Box->GetBreakCount();
    BatchBaseline = Box->GetDropGenerationCount();
    RecoveryBaseline = Player->RecoveryCount;
    PoundBaseline = Player->GetActions()->GetCompletedPounds();
    bCacheSaved = bDropsSaved = false;
}

bool UThunderBoxTestSubsystem::WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    if (Offset.Size2D() < 15 && Player->GetCharacterMovement()->IsMovingOnGround())
    { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (auto* PC = Cast<APlayerController>(Player->GetController())) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-23, Offset.Rotation().Yaw, 0), DeltaSeconds, 3));
    return false;
}

bool UThunderBoxTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* PC = Cast<APlayerController>(Player->GetController());
    auto* Mode = Cast<AThunderMeadowGameMode>(Player->GetWorld()->GetAuthGameMode());
    if (!Box.IsValid() || !PC || !Mode) { Error = TEXT("Box test actor or controller missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 20) { Error = FString::Printf(TEXT("Box phase %d timed out at %s"), Phase, *Player->GetActorLocation().ToString()); return false; }
    auto* Ledger = Mode->GetCoinLedger();
    auto Next = [this](int32 Number) { Phase = Number; PhaseTime = 0; };
    if (Phase == 0)
    {
        if (WalkToward(Player, Box->GetActorLocation() + FVector(-140, 0, 82), DeltaSeconds)) Next(1);
    }
    else if (Phase == 1)
    {
        if (!bCacheSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderCopperCache.png"), false, false);
            bCacheSaved = true;
        }
        if (PhaseTime > .6f) { BoxTap(PC, EKeys::LeftMouseButton); Next(2); }
    }
    else if (Phase == 2 && PhaseTime > .3f)
    {
        if (!Box->IsBroken() || Box->GetBreakCount() != BreakBaseline + 1 || Box->GetDropGenerationCount() != BatchBaseline + 1 || Box->GetDropCoins().Num() != 3)
        { Error = TEXT("Punch did not break one box and generate exactly three coins"); return false; }
        BoxTap(PC, EKeys::LeftMouseButton);
        Next(3);
    }
    else if (Phase == 3)
    {
        if (Box->GetBreakCount() != BreakBaseline + 1 || Box->GetDropGenerationCount() != BatchBaseline + 1 || Box->GetDropCoins().Num() != 3)
        { Error = TEXT("Broken box generated contents again"); return false; }
        const auto& Drops = Box->GetDropCoins();
        if (!Drops[DropIndex].IsValid()) { Error = TEXT("A box drop is missing"); return false; }
        if (WalkToward(Player, Drops[DropIndex]->GetActorLocation(), DeltaSeconds) && ++DropIndex == 3) Next(4);
    }
    else if (Phase == 4)
    {
        if (Ledger->GetTotal() != 3) { Error = TEXT("Three real box drops did not total three"); return false; }
        if (!bDropsSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderCacheDrops.png"), false, false);
            bDropsSaved = true;
        }
        if (PhaseTime > .6f) { BoxTap(PC, EKeys::R); Next(5); }
    }
    else if (Phase == 5 && PhaseTime > .2f)
    {
        if (Box->IsBroken() || Box->GetDropCoins().Num() || Ledger->GetTotal() != 0 || Player->RecoveryCount != RecoveryBaseline + 1)
        { Error = TEXT("R did not restore the box and remove its own drops and total"); return false; }
        Next(6);
    }
    else if (Phase == 6)
    {
        if (WalkToward(Player, Box->GetActorLocation() + FVector(-280, 0, 82), DeltaSeconds))
        { BoxKey(PC, EKeys::SpaceBar, true); Next(7); }
    }
    else if (Phase == 7)
    {
        if (PhaseTime > .15f) BoxKey(PC, EKeys::SpaceBar, false);
        const FVector Offset = Box->GetActorLocation() - Player->GetActorLocation();
        const FVector Steering = FVector(Offset.X, Offset.Y, 0) - FVector(Player->GetVelocity().X, Player->GetVelocity().Y, 0) * .22f;
        Player->AddMovementInput(Steering.GetSafeNormal2D());
        const float Feet = Player->GetActorLocation().Z - Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        if (Player->GetCharacterMovement()->IsFalling() && Feet > Box->GetActorLocation().Z + 145 && Offset.Size2D() < 30)
        { BoxTap(PC, EKeys::LeftControl); Next(8); }
    }
    else if (Phase == 8)
    {
        if (Box->IsBroken())
        {
            if (Box->GetBreakCount() != BreakBaseline + 2 || Box->GetDropGenerationCount() != BatchBaseline + 2 || Box->GetDropCoins().Num() != 3 || Player->GetActions()->GetCompletedPounds() != PoundBaseline + 1)
            { Error = TEXT("Pound landing did not break once and generate a fresh batch"); return false; }
            DropIndex = 0;
            Next(9);
        }
    }
    else if (Phase == 9)
    {
        const auto& Drops = Box->GetDropCoins();
        if (Drops.Num() != 3 || !Drops[DropIndex].IsValid()) { Error = TEXT("Fresh pound drops are missing"); return false; }
        if (WalkToward(Player, Drops[DropIndex]->GetActorLocation(), DeltaSeconds) && ++DropIndex == 3) Next(10);
    }
    else if (Phase == 10 && PhaseTime > .3f)
    {
        if (Ledger->GetTotal() != 3 || Box->GetDropGenerationCount() != BatchBaseline + 2)
        { Error = TEXT("Reopened box IDs did not collect once after R"); return false; }
        UE_LOG(LogTemp, Display, TEXT("BOX TEST PASS: normal walk and punch, exactly three collectible drops, no duplicate batch, R cleanup, normal jump and pound on top, fresh three points after R."));
        return true;
    }
    return false;
}
