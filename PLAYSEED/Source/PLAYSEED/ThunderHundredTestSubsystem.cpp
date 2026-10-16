#include "ThunderHundredTestSubsystem.h"
#include "ThunderHundredLantern.h"
#include "ThunderMeadowStage.h"
#include "ThunderMeadowGameMode.h"
#include "ThunderTraversalTestSubsystem.h"
#include "ThunderCoinLedger.h"
#include "ThunderExpeditionSubsystem.h"
#include "PlayseedCoin.h"
#include "PlayseedLantern.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
namespace { void HundredTap(APlayerController* PC, FKey Key) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Pressed, 1)); PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Released, 0)); } }
void UThunderHundredTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage, bool bReadProcess)
{
    Stage = InStage; Error.Reset(); PhaseTime = 0; Phase = bReadProcess ? 10 : 0; bSawNinetyNine = false; RecoveryBefore = Player->RecoveryCount;
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if (!Progress->GetSaveSlotName().StartsWith(TEXT("Validation_")) || !Progress->GetProgressProblem().IsEmpty() || Progress->GetEarnedRewardCount() != (bReadProcess ? 1 : 0) || Progress->HasEarnedReward(7) != bReadProcess) { Error = TEXT("Hundred test needs isolated actual expected reward7 state"); return; }
    if (!bReadProcess)
    {
        TArray<FVector> Mountain = Stage->GetMountainFootPath(); Mountain.SetNum(20);
        GetGameInstance()->GetSubsystem<UThunderTraversalTestSubsystem>()->BeginCoinTrailRoute(Player, Stage->GetMeadowCoinFootPath(), Mountain);
    }
}
bool UThunderHundredTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* Bonus = Stage.IsValid() ? Stage->GetHundredLantern() : nullptr;
    auto* Mode = Cast<AThunderMeadowGameMode>(Player->GetWorld()->GetAuthGameMode()); auto* PC = Cast<APlayerController>(Player->GetController());
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>(); auto* Move = Player->GetCharacterMovement();
    if (!Bonus || !Mode || !PC) { Error = TEXT("Hundred stage/controller missing"); return false; }
    PhaseTime += DeltaSeconds; if (PhaseTime > 240) { Error = FString::Printf(TEXT("Hundred phase%d timed out at%s"), Phase, *Player->GetActorLocation().ToString()); return false; }
    if (Player->GetVitality()->GetHealth() != 8 || !Progress->GetProgressProblem().IsEmpty()) { Error = TEXT("Hundred route took damage or save failed"); return false; }
    auto* Ledger = Mode->GetCoinLedger();
    auto Next = [this](int32 Number) { Phase = Number; PhaseTime = 0; };
    if (Phase == 0)
    {
        if (Progress->GetEarnedRewardCount() != 0 || (Ledger->GetTotal() < 100 && (Bonus->HasReachedHundred() || Bonus->GetReward()))) { Error = TEXT("Hundred reward appeared or saved before real threshold"); return false; }
        if (Ledger->GetTotal() == 99 && !bSawNinetyNine) { bSawNinetyNine = true; FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderHundredNinetyNine.png"), false, false); UE_LOG(LogTemp, Display, TEXT("HUNDRED TEST: actual total99, no bonus actor or saved reward.")); }
        auto* Route = GetGameInstance()->GetSubsystem<UThunderTraversalTestSubsystem>(); const bool bDone = Route->UpdateRoute(Player, DeltaSeconds);
        if (Route->HasFailed()) { Error = Route->GetError(); return false; }
        if (bDone)
        {
            if (!bSawNinetyNine || Ledger->GetTotal() != 100 || Ledger->GetRedCount() != 0 || !Bonus->GetReward() || Bonus->GetReward()->IsCollected() || Bonus->GetGenerationCount() != 1 || Bonus->GetTriggerTotal() != 100) { Error = TEXT("Actual99 to100 did not create exactly one uncollected reward7"); return false; }
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderHundredReady.png"), false, false); Next(1);
        }
    }
    else if (Phase == 1)
    {
        const FVector Goal = Bonus->GetRewardLocation() - FVector(0, 0, 18), Offset = Goal - Player->GetActorLocation();
        Player->AddMovementInput(Offset.GetSafeNormal2D(), FMath::Clamp(Offset.Size2D() / 100, .2f, 1.f)); PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-23, Offset.Rotation().Yaw, 0), DeltaSeconds, 3));
        if (Move->IsMovingOnGround() && Offset.Z > 60 && Offset.Size2D() < 350) Player->Jump(); else if (Player->GetVelocity().Z < 0) Player->StopJumping();
        if (Progress->HasEarnedReward(7)) { if (Move->IsMovingOnGround()) Move->StopMovementImmediately(); Next(2); }
    }
    else if (Phase == 2 && PhaseTime > .5f)
    {
        if (!Bonus->GetReward() || !Bonus->GetReward()->IsCollected() || Bonus->GetReward()->GetSuccessfulCollections() != 1 || Bonus->GetGenerationCount() != 1 || Progress->GetEarnedRewardCount() != 1 || !Progress->HasEarnedReward(7)) { Error = TEXT("Actual bonus contact did not save only reward7 once"); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderHundredReward.png"), false, false); Next(3);
    }
    else if (Phase == 3 && PhaseTime > .5f) { HundredTap(PC, EKeys::R); Next(4); }
    else if (Phase == 4 && PhaseTime > .4f)
    {
        if (Player->RecoveryCount != RecoveryBefore + 1 || Ledger->GetTotal() != 0 || Bonus->GetReward() || Bonus->HasReachedHundred() || Progress->GetEarnedRewardCount() != 1 || !Progress->HasEarnedReward(7)) { Error = TEXT("R failed to clear hundred visit and retain real reward7"); return false; }
        for (APlayseedCoin* Coin : Stage->GetTrailCoins()) if (!Coin || Coin->IsCollected()) { Error = TEXT("R did not restore100 actual trail coins"); return false; }
        UE_LOG(LogTemp, Display, TEXT("HUNDRED TEST PASS:100 real unique golds,99excluded,100one bonus,actual walk pickup7,duplicate guard,R saved7 retained.")); return true;
    }
    else if (Phase == 10 && PhaseTime > 1)
    {
        if (Bonus->GetReward() || Bonus->HasReachedHundred() || Ledger->GetTotal() != 0) { Error = TEXT("Read process did not start an empty hundred visit"); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderHundredRead.png"), false, false); HundredTap(PC, EKeys::R); Next(11);
    }
    else if (Phase == 11 && PhaseTime > .4f)
    {
        if (Progress->GetEarnedRewardCount() != 1 || !Progress->HasEarnedReward(7) || Ledger->GetTotal() != 0 || Bonus->GetReward()) { Error = TEXT("Read R lost actual reward7 or kept visit total"); return false; }
        UE_LOG(LogTemp, Display, TEXT("HUNDRED READ TEST PASS: separate process restored actually earned7 only, fresh ledger/bonus and R persistence; no fixtures.")); return true;
    }
    return false;
}
