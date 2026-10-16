#include "ThunderSentinelTestSubsystem.h"
#include "ThunderBoundSentinel.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
namespace
{
void SentinelTap(APlayerController* PC, FKey Key) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Pressed, 1)); PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Released, 0)); }
}
void UThunderSentinelTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderBoundSentinel* InSentinel)
{
    Sentinel = InSentinel; Phase = Waypoint = 0; PhaseTime = 0; Error.Reset();
    LungeBaseline = Sentinel->GetLungeCount(); HitBaseline = Sentinel->GetConfirmedHits(); RecoveryBaseline = Player->RecoveryCount;
}
bool UThunderSentinelTestSubsystem::Walk(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    if (Offset.Size2D() < 10 && Player->GetCharacterMovement()->IsMovingOnGround()) { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (auto* PC = Cast<APlayerController>(Player->GetController())) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, Offset.Rotation().Yaw, 0), DeltaSeconds, 3));
    return false;
}
bool UThunderSentinelTestSubsystem::Approach(AWindseedCharacter* Player, float DeltaSeconds)
{
    const FVector Points[] = {FVector(-600, -900, 82), FVector(1400, -900, 82), FVector(3150, -900, 82), FVector(3150, -2800, 82)};
    if (Waypoint < 4 && Walk(Player, Points[Waypoint], DeltaSeconds)) ++Waypoint;
    return Waypoint == 4;
}
bool UThunderSentinelTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* PC = Cast<APlayerController>(Player->GetController());
    if (!Sentinel.IsValid() || !PC) { Error = TEXT("Sentinel/controller missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 35) { Error = FString::Printf(TEXT("Sentinel phase %d timed out at %s, enemy %s"), Phase, *Player->GetActorLocation().ToString(), *Sentinel->GetActorLocation().ToString()); return false; }
    if (FVector::Dist2D(Sentinel->GetActorLocation(), Sentinel->GetAnchor()) > 300.5f || Sentinel->GetMaxObservedReach() > 300.5f) { Error = TEXT("Bound sentinel escaped its actual tether radius"); return false; }
    if (Phase != 3 && Phase != 4 && Player->GetVitality()->GetHealth() != 8) { Error = FString::Printf(TEXT("Sentinel phase %d unexpectedly took damage"), Phase); return false; }
    auto Next = [this](int32 To) { Phase = To; PhaseTime = 0; UE_LOG(LogTemp, Display, TEXT("SENTINEL TEST phase %d"), Phase); };
    if (Phase == 0) { if (Approach(Player, DeltaSeconds)) Next(1); }
    else if (Phase == 1)
    {
        if (Sentinel->GetLungeCount() >= LungeBaseline + 2 && !Sentinel->IsLunging() && !Sentinel->IsWindingUp())
        {
            if (Sentinel->GetConfirmedHits() != HitBaseline || Sentinel->GetMaxObservedReach() < 295) { Error = TEXT("Two actual lunges did not stop at the tether without hurting the outside player"); return false; }
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderSentinelBound.png"), false, false);
            UE_LOG(LogTemp, Display, TEXT("SENTINEL TEST: two real lunges, outside player unharmed, actual max reach %.2f."), Sentinel->GetMaxObservedReach());
            Next(2);
        }
    }
    else if (Phase == 2)
    {
        if (Sentinel->IsPatrolling() || Sentinel->IsWindingUp()) { Next(3); }
    }
    else if (Phase == 3)
    {
        Walk(Player, Sentinel->GetAnchor() + FVector(350, 0, 82), DeltaSeconds);
        if (Sentinel->GetConfirmedHits() == HitBaseline + 1)
        {
            if (Player->GetVitality()->GetHealth() != 6) { Error = TEXT("Actual sentinel contact did not take two health"); return false; }
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderSentinelImpact.png"), false, false); Next(4);
        }
    }
    else if (Phase == 4 && PhaseTime > .3f) { SentinelTap(PC, EKeys::R); Next(5); }
    else if (Phase == 5 && PhaseTime > .3f)
    {
        if (Player->RecoveryCount != RecoveryBaseline + 1 || !Sentinel->IsPatrolling() || FVector::Dist2D(Sentinel->GetActorLocation(), Sentinel->GetHomeLocation()) > 65) { Error = TEXT("R did not reset sentinel and restore health"); return false; }
        Waypoint = 0; Next(6);
    }
    else if (Phase == 6) { if (Approach(Player, DeltaSeconds)) Next(7); }
    else if (Phase == 7)
    {
        // 予告が終わって帰るまで安全な外側で待ってから、範囲の内側へ一歩入ります。
        if (Sentinel->IsPatrolling()) Next(8);
    }
    else if (Phase == 8)
    {
        Walk(Player, Sentinel->GetAnchor() + FVector(350, 0, 82), DeltaSeconds);
        if (Sentinel->IsWindingUp() && FVector::Dist2D(Player->GetActorLocation(), Sentinel->GetAnchor()) < 360) { LungeBaseline = Sentinel->GetLungeCount(); Next(9); }
    }
    else if (Phase == 9)
    {
        Walk(Player, Sentinel->GetAnchor() + FVector(350, 600, 82), DeltaSeconds);
        if (Sentinel->GetLungeCount() > LungeBaseline && !Sentinel->IsLunging() && !Sentinel->IsWindingUp())
        {
            if (Sentinel->GetConfirmedHits() != HitBaseline + 1) { Error = TEXT("Leaving the gold windup target failed to evade the real lunge"); return false; }
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderSentinelDodge.png"), false, false);
            UE_LOG(LogTemp, Display, TEXT("SENTINEL TEST: ordinary sideways input evaded a real locked-target lunge.")); Waypoint = 0; Next(10);
        }
    }
    else if (Phase == 10)
    {
        const FVector Goal = Sentinel->GetAnchor() + (Waypoint == 0 ? FVector(450, 600, 82) : FVector(450, 0, 82));
        if (Walk(Player, Goal, DeltaSeconds) && Waypoint == 0) ++Waypoint;
        if (Waypoint == 1 && FVector::Dist2D(Player->GetActorLocation(), Goal) < 12 && Sentinel->IsWindingUp())
        { LungeBaseline = Sentinel->GetLungeCount(); SentinelTap(PC, EKeys::R); Next(11); }
    }
    else if (Phase == 11 && PhaseTime > 1.7f)
    {
        if (Player->RecoveryCount != RecoveryBaseline + 2 || Sentinel->GetLungeCount() != LungeBaseline || Sentinel->GetConfirmedHits() != HitBaseline + 1 || !Sentinel->IsPatrolling()) { Error = TEXT("R during windup left a stale lunge or damage callback"); return false; }
        UE_LOG(LogTemp, Display, TEXT("SENTINEL TEST PASS: actual ground approach, bounded lunges, two damage, normal dodge, R during windup and no late contact.")); return true;
    }
    return false;
}
