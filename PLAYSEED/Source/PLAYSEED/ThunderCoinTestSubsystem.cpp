#include "ThunderCoinTestSubsystem.h"
#include "ThunderMeadowStage.h"
#include "ThunderMeadowGameMode.h"
#include "ThunderCoinLedger.h"
#include "ThunderExpeditionSubsystem.h"
#include "PlayseedCoin.h"
#include "WindseedCharacter.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UThunderCoinTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage)
{
    Stage = InStage;
    Phase = 0;
    CoinIndex = 0;
    PhaseTime = 0;
    Error.Reset();
    bCoinsSaved = bTotalSaved = false;
    RecoveryBaseline = Player->RecoveryCount;
    RewardsBaseline = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->GetEarnedRewardCount();
}

bool UThunderCoinTestSubsystem::WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    if (Offset.Size2D() < 12 && Player->GetCharacterMovement()->IsMovingOnGround())
    { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (auto* PC = Cast<APlayerController>(Player->GetController()))
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-22, Offset.Rotation().Yaw, 0), DeltaSeconds, 3));
    return false;
}

bool UThunderCoinTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* Mode = Cast<AThunderMeadowGameMode>(Player->GetWorld()->GetAuthGameMode());
    auto* PC = Cast<APlayerController>(Player->GetController());
    if (!Stage.IsValid() || !Mode || !PC || Stage->GetTrainingCoins().Num() != 3)
    { Error = TEXT("Coin test stage or controller is missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 20) { Error = FString::Printf(TEXT("Coin phase %d timed out"), Phase); return false; }
    auto* Ledger = Mode->GetCoinLedger();
    const auto& Coins = Stage->GetTrainingCoins();
    auto Next = [this](int32 Number) { Phase = Number; PhaseTime = 0; };
    if (Phase == 0)
    {
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-18, 45, 0), DeltaSeconds, 3));
        if (Ledger->GetTotal() != 0 || Ledger->GetRedCount() != 0) { Error = TEXT("Coin visit did not begin empty"); return false; }
        if (PhaseTime > .5f && !bCoinsSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderCoinKinds.png"), false, false);
            bCoinsSaved = true;
        }
        if (PhaseTime > 1) Next(1);
    }
    else if (Phase == 1)
    {
        if (WalkToward(Player, Coins[CoinIndex]->GetActorLocation(), DeltaSeconds))
        {
            const int32 ExpectedTotals[] = {1, 3, 8};
            if (!Coins[CoinIndex]->IsCollected() || Ledger->GetTotal() != ExpectedTotals[CoinIndex] || Ledger->GetRedCount() != (CoinIndex ? 1 : 0))
            { Error = TEXT("Walking into gold/scarlet/azure did not add 1/2/5 exactly once"); return false; }
            if (++CoinIndex == 3) { CoinIndex = 2; Next(2); }
        }
    }
    else if (Phase == 2)
    {
        // 同じ三つの位置を歩き直します。消えたコインの衝突へ触れても増えてはいけません。
        if (Ledger->GetTotal() != 8 || Ledger->GetRedCount() != 1) { Error = TEXT("A collected coin counted twice"); return false; }
        if (WalkToward(Player, Coins[CoinIndex]->GetActorLocation(), DeltaSeconds) && --CoinIndex < 0)
        {
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R, IE_Pressed, 1));
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R, IE_Released, 0));
            Next(3);
        }
    }
    else if (Phase == 3 && PhaseTime > .2f)
    {
        if (Ledger->GetTotal() != 0 || Ledger->GetRedCount() != 0 || Player->RecoveryCount != RecoveryBaseline + 1 ||
            Coins[0]->IsCollected() || Coins[1]->IsCollected() || Coins[2]->IsCollected())
        { Error = TEXT("R while overlapping did not reset the visit and coin visibility"); return false; }
        Next(4);
    }
    else if (Phase == 4)
    {
        if (WalkToward(Player, Coins[2]->GetActorLocation(), DeltaSeconds))
        {
            if (Ledger->GetTotal() != 5 || Ledger->GetRedCount() != 0 || Coins[0]->IsCollected() || Coins[1]->IsCollected() || !Coins[2]->IsCollected())
            { Error = TEXT("Azure recollection after R did not start a fresh visit"); return false; }
            Next(5);
        }
    }
    else if (Phase == 5)
    {
        if (GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->GetEarnedRewardCount() != RewardsBaseline)
        { Error = TEXT("Coin totals unexpectedly changed saved rewards"); return false; }
        if (PhaseTime > .5f && !bTotalSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderCoinTotal.png"), false, false);
            bTotalSaved = true;
            UE_LOG(LogTemp, Display, TEXT("COIN TEST PASS: walked into values 1/2/5, total 8, one scarlet, duplicate rejection, R while overlapping, azure recollection total 5; saved rewards unchanged."));
        }
        return PhaseTime > 1.5f;
    }
    return false;
}
