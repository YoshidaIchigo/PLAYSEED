#include "ThunderPostTestSubsystem.h"
#include "ThunderMeadowStage.h"
#include "ThunderBindingPost.h"
#include "ThunderPrisonGate.h"
#include "ThunderBoundSentinel.h"
#include "ThunderExpeditionSubsystem.h"
#include "PlayseedLantern.h"
#include "PlayseedVitalityComponent.h"
#include "PlayseedActionComponent.h"
#include "WindseedCharacter.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
namespace { void PostTap(APlayerController* PC, FKey Key) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Pressed, 1)); PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Released, 0)); } }
void UThunderPostTestSubsystem::Next(int32 Number) { Phase = Number; PhaseTime = 0; Waypoint = 0; bJumpStarted = bPoundPressed = false; JumpTime = 0; UE_LOG(LogTemp, Display, TEXT("POST TEST phase %d round %d"), Number, Round); }
void UThunderPostTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage, bool bReadProcess)
{
    Stage = InStage; Error.Reset(); Round = 0; RecoveryBaseline = Player->RecoveryCount; BreakBaseline = Stage->GetPrisonGate()->GetBreakCount(); ContactBaseline = Stage->GetBoundSentinel()->GetConfirmedHits();
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if (!Progress->GetSaveSlotName().StartsWith(TEXT("Validation_")) || !Progress->GetProgressProblem().IsEmpty() || !RewardBaseline.Begin(Progress, 6, bReadProcess) || Progress->HasEarnedReward(6) != bReadProcess) { Error = TEXT("Post test needs isolated expected actual reward6 state"); return; }
    Next(bReadProcess ? 20 : 0);
}
bool UThunderPostTestSubsystem::Walk(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    if (Offset.Size2D() < 12 && Player->GetCharacterMovement()->IsMovingOnGround()) { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D(), FMath::Clamp(Offset.Size2D() / 90, .2f, 1.f));
    auto* PC = Cast<APlayerController>(Player->GetController()); PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-22, Offset.Rotation().Yaw, 0), DeltaSeconds, 3)); return false;
}
bool UThunderPostTestSubsystem::Approach(AWindseedCharacter* Player, float DeltaSeconds)
{
    const FVector Points[] = {FVector(-600, -900, 82), FVector(-600, -1700, 82), FVector(1700, -1700, 82), FVector(1700, -2900, 82), FVector(1700, -3100, 82), FVector(2250, -3100, 82), FVector(2250, -2800, 82)};
    if (Waypoint < UE_ARRAY_COUNT(Points) && Walk(Player, Points[Waypoint], DeltaSeconds)) ++Waypoint;
    return Waypoint == UE_ARRAY_COUNT(Points);
}
bool UThunderPostTestSubsystem::JumpToPost(AWindseedCharacter* Player, float DeltaSeconds, bool bPound)
{
    auto* Post = Stage->GetBindingPost(); auto* Move = Player->GetCharacterMovement(); auto* PC = Cast<APlayerController>(Player->GetController());
    const FVector Offset = Post->GetActorLocation() - Player->GetActorLocation(); const float Feet = Player->GetActorLocation().Z - 82;
    // 空中の慣性は逆方向の通常入力で減速します。速度や位置を直接変更しません。
    if (bJumpStarted && !Move->IsMovingOnGround())
    {
        const FVector Desired = (Offset.GetSafeNormal2D() * Offset.Size2D() * 4.5f).GetClampedToMaxSize2D(470);
        const FVector Correction = Desired - FVector(Player->GetVelocity().X, Player->GetVelocity().Y, 0);
        if (Correction.Size2D() > 2) Player->AddMovementInput(Correction.GetSafeNormal2D(), FMath::Clamp(Correction.Size2D() / 150, 0.f, 1.f));
    }
    else if (Offset.Size2D() > 8) Player->AddMovementInput(Offset.GetSafeNormal2D(), FMath::Clamp(Offset.Size2D() / 100, .15f, 1.f));
    PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-28, 0, 0), DeltaSeconds, 4));
    if (!bJumpStarted && Move->IsMovingOnGround() && Offset.Size2D() < 330) { Player->Jump(); bJumpStarted = true; JumpTime = 0; BeforePounds = Post->GetPoundCount(); UE_LOG(LogTemp, Display, TEXT("POST TEST: normal jump from %s toward actual top %.0f"), *Player->GetActorLocation().ToString(), Post->GetTopHeight()); }
    if (!bJumpStarted) return false;
    JumpTime += DeltaSeconds; if (JumpTime > .18f) Player->StopJumping();
    if (bPound && !bPoundPressed && !Move->IsMovingOnGround() && Offset.Size2D() < 45 && Feet > Post->GetTopHeight() + 45 && JumpTime > .2f) { PostTap(PC, EKeys::LeftControl); bPoundPressed = true; }
    return bPound ? Post->GetPoundCount() == BeforePounds + 1 : JumpTime > .3f && Move->IsMovingOnGround() && FMath::Abs(Feet - Post->GetTopHeight()) < 6 && Offset.Size2D() < 50;
}
bool UThunderPostTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* Post = Stage.IsValid() ? Stage->GetBindingPost() : nullptr; auto* Gate = Stage.IsValid() ? Stage->GetPrisonGate() : nullptr; auto* Enemy = Stage.IsValid() ? Stage->GetBoundSentinel() : nullptr;
    auto* PC = Cast<APlayerController>(Player->GetController()); auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    if (!Post || !Gate || !Enemy || !PC) { Error = TEXT("Post/gate/sentinel/controller missing"); return false; }
    if (!RewardBaseline.HasPrior(Progress)) { Error = TEXT("A previously earned campaign reward was lost"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 45) { Error = FString::Printf(TEXT("Post phase %d round %d timed out hero %s enemy %s pounds%d"), Phase, Round, *Player->GetActorLocation().ToString(), *Enemy->GetActorLocation().ToString(), Post->GetPoundCount()); return false; }
    // 通常着地の負例では、無防備な静止に番獣が接触する場合も実観測してからRへ進みます。
    // 実際の杭の床情報・位置・接触回数が一致する場合だけを許し、その後の本攻略は体力8を守ります。
    const auto* Movement = Player->GetCharacterMovement();
    const bool bOrdinaryTop = Phase == 2 && Round == 0 && Movement->CurrentFloor.IsWalkableFloor() && Movement->CurrentFloor.HitResult.GetActor() == Post && FVector::Dist2D(Player->GetActorLocation(), Post->GetActorLocation()) < 50 && FMath::Abs(Player->GetActorLocation().Z - 82 - Post->GetTopHeight()) < 6;
    const bool bObservedTopContact = bOrdinaryTop && Player->GetVitality()->GetHealth() == 6 && Enemy->GetConfirmedHits() == ContactBaseline + 1;
    if ((Player->GetVitality()->GetHealth() != 8 && !bObservedTopContact) || !Progress->GetProgressProblem().IsEmpty()) { Error = FString::Printf(TEXT("Post route phase %d waypoint %d at %s enemy %s unexpectedly took damage or save failed"), Phase, Waypoint, *Player->GetActorLocation().ToString(), *Enemy->GetActorLocation().ToString()); return false; }
    if (bOrdinaryTop)
    {
        if (Post->GetPoundCount() != 0 || Enemy->IsReleased() || Gate->IsBroken() || Gate->GetReward()) { Error = TEXT("Ordinary physical floor landing incorrectly advanced post"); return false; }
        UE_LOG(LogTemp, Display, TEXT("POST TEST: actual post floor landing excluded, observed health%d actual enemy contacts%d; R before positive pound route."), Player->GetVitality()->GetHealth(), Enemy->GetConfirmedHits() - ContactBaseline);
        Player->StopJumping(); PostTap(PC, EKeys::R); Next(4); return false;
    }
    if (Phase < 8 && Progress->GetEarnedRewardCount() != RewardBaseline.BeforeCount()) { Error = TEXT("Reward6 was saved before walking to actual lantern"); return false; }
    if (Phase == 0) { if (Approach(Player, DeltaSeconds)) Next(1); }
    else if (Phase == 1) { if (Enemy->IsRecovering()) Next(2); }
    else if (Phase == 2)
    {
        if (JumpToPost(Player, DeltaSeconds, Round != 0))
        {
            if (Round == 0)
            {
                if (Post->GetPoundCount() != 0 || Enemy->IsReleased() || Gate->IsBroken() || Gate->GetReward()) { Error = TEXT("Ordinary top landing incorrectly advanced post"); return false; }
                UE_LOG(LogTemp, Display, TEXT("POST TEST: actual ordinary landing on post top did not advance it."));
                Player->GetCharacterMovement()->StopMovementImmediately(); PostTap(PC, EKeys::R); Next(4);
            }
            else if (Round == 1) { PostTap(PC, EKeys::R); Next(4); }
            else if (Post->GetPoundCount() < 3) Next(5);
            else Next(6);
        }
    }
    else if (Phase == 4)
    {
        if (PhaseTime > .4f)
        {
            if (Post->GetPoundCount() != 0 || Gate->IsBroken() || Gate->GetReward() || Enemy->IsReleased() || Player->RecoveryCount != RecoveryBaseline + 1) { Error = TEXT("R did not clear actual partial post progress and keep prison closed"); return false; }
            RecoveryBaseline = Player->RecoveryCount; ++Round; Next(0);
        }
    }
    else if (Phase == 5 && PhaseTime > .2f) Next(2);
    else if (Phase == 6)
    {
        Walk(Player, Post->GetActorLocation() + FVector(0, 360, 82), DeltaSeconds);
        const float GateYaw = (Gate->GetActorLocation() - Player->GetActorLocation()).Rotation().Yaw;
        PC->SetControlRotation(FRotator(-18, GateYaw, 0));
        if (Gate->IsBroken() && Enemy->IsDeparted())
        {
            if (Post->GetPoundCount() != 3 || Gate->GetBreakCount() != BreakBaseline + 1 || !Gate->GetReward() || Gate->GetReward()->IsCollected()) { Error = TEXT("Three real pounds did not cause exactly one actual gate collision and an uncollected reward"); return false; }
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderPostGateOpen.png"), false, false); Next(7);
        }
    }
    else if (Phase == 7)
    {
        const FVector Points[] = {FVector(3150, -2440, 82), FVector(3150, -2800, 82)};
        if (Waypoint < 2 && Walk(Player, Points[Waypoint], DeltaSeconds)) ++Waypoint;
        if (Waypoint == 2) Next(8);
    }
    else if (Phase == 8) { Walk(Player, Gate->GetRewardLocation() - FVector(0, 0, 18), DeltaSeconds); if (Progress->HasEarnedReward(6)) Next(9); }
    else if (Phase == 9 && PhaseTime > .5f)
    {
        if (Progress->GetEarnedRewardCount() != RewardBaseline.AfterCount() || !Gate->GetReward() || !Gate->GetReward()->IsCollected() || Gate->GetBreakCount() != BreakBaseline + 1) { Error = TEXT("Walking through actually broken gate did not save only reward6 once"); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderPostReward.png"), false, false); Next(10);
    }
    else if (Phase == 10 && PhaseTime > .5f) { PostTap(PC, EKeys::R); Next(11); }
    else if (Phase == 11 && PhaseTime > .4f)
    {
        if (Post->GetPoundCount() != 0 || Gate->IsBroken() || Gate->GetReward() || Enemy->IsReleased() || !Progress->HasEarnedReward(6) || Progress->GetEarnedRewardCount() != RewardBaseline.AfterCount()) { Error = TEXT("R failed to reset prison visit while retaining actually saved reward6"); return false; }
        UE_LOG(LogTemp, Display, TEXT("POST TEST PASS: real ordinary landing excluded, actual partial pound R, three real top pounds, released enemy actual gate sweep, actual walk into reward6, R persistence.")); return true;
    }
    else if (Phase == 20 && PhaseTime > 1)
    {
        if (Post->GetPoundCount() != 0 || Gate->IsBroken() || Gate->GetReward() || Enemy->IsReleased()) { Error = TEXT("Separate read process did not start fresh post/prison visit"); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderPostRead.png"), false, false); PostTap(PC, EKeys::R); Next(21);
    }
    else if (Phase == 21 && PhaseTime > .4f)
    {
        if (!Progress->HasEarnedReward(6) || Progress->GetEarnedRewardCount() != RewardBaseline.AfterCount() || Post->GetPoundCount() != 0 || Gate->IsBroken()) { Error = TEXT("Separate read R lost actual reward6"); return false; }
        UE_LOG(LogTemp, Display, TEXT("POST READ TEST PASS: separate process loaded actually earned reward6, fresh post/prison, R retained save; no fixtures.")); return true;
    }
    return false;
}
