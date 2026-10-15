#include "ThunderStompTestSubsystem.h"
#include "PlayseedEnemyBase.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace
{
void StompKey(APlayerController* PC, FKey Key, bool bPressed) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, bPressed ? IE_Pressed : IE_Released, bPressed ? 1 : 0)); }
void StompTap(APlayerController* PC, FKey Key) { StompKey(PC, Key, true); StompKey(PC, Key, false); }
}

void UThunderStompTestSubsystem::BeginTest(AWindseedCharacter* Player, APlayseedEnemyBase* InEnemy)
{
    Enemy = InEnemy;
    Error.Reset();
    Phase = 0;
    PhaseTime = 0;
    ApproachLeg = 0;
    Loop = 0;
    RecoveryBaseline = Player->RecoveryCount;
    HitBaseline = Player->GetVitality()->GetSuccessfulImpacts();
    StompBaseline = Enemy->GetSuccessfulStomps();
    bLiveSaved = false;
    bImpactSaved = false;
    bLoggedPass = false;
}

bool UThunderStompTestSubsystem::WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    if (Offset.Size2D() < 25 && Player->GetCharacterMovement()->IsMovingOnGround())
    { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (auto* PC = Cast<APlayerController>(Player->GetController())) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, Offset.Rotation().Yaw, 0), DeltaSeconds, 2));
    return false;
}

bool UThunderStompTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    if (!Enemy.IsValid()) { Error = TEXT("Practice enemy is missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 20) { Error = FString::Printf(TEXT("Stomp phase %d timed out at %s"), Phase, *Player->GetActorLocation().ToString()); return false; }
    auto* PC = Cast<APlayerController>(Player->GetController());
    if (!PC) { Error = TEXT("Stomp controller is missing"); return false; }
    auto* Movement = Player->GetCharacterMovement();
    auto Advance = [this](int32 Next) { Phase = Next; PhaseTime = 0; };
    if (Phase == 0)
    {
        const FVector StompApproachPoints[] = {FVector(600, -600, 82), FVector(600, -1500, 82)};
        if (WalkToward(Player, StompApproachPoints[ApproachLeg], DeltaSeconds) && ++ApproachLeg == 2) Advance(1);
    }
    else if (Phase == 1)
    {
        if (Player->GetVitality()->GetSuccessfulImpacts() == HitBaseline + 1)
        {
            if (Enemy->IsDefeated() || Enemy->GetSuccessfulStomps() != StompBaseline || Player->GetVitality()->GetHealth() != 6)
            { Error = TEXT("Side contact was incorrectly counted as a stomp"); return false; }
            UE_LOG(LogTemp, Display, TEXT("STOMP TEST: side contact damaged the player without defeating the enemy."));
            StompTap(PC, EKeys::R);
            Advance(2);
        }
        else WalkToward(Player, Enemy->GetActorLocation() + FVector(0, 0, 82), DeltaSeconds);
    }
    else if ((Phase == 2 || Phase == 7) && PhaseTime > .3f)
    {
        const int32 ExpectedRecoveries = RecoveryBaseline + (Phase == 2 ? 1 : 2);
        if (Enemy->IsDefeated() || Player->GetVitality()->GetHealth() != 8 || Player->RecoveryCount != ExpectedRecoveries || FVector::Dist2D(Player->GetActorLocation(), FVector(-600, -600, 100)) > 5 || Player->GetVelocity().Size2D() > 5)
        { Error = TEXT("R did not revive the enemy and restore the player"); return false; }
        if (Phase == 7) Loop = 1;
        Advance(3);
    }
    else if (Phase == 3)
    {
        if (WalkToward(Player, Enemy->GetActorLocation() + FVector(-330, 0, 82), DeltaSeconds))
        {
            if (!bLiveSaved)
            {
                FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderTuftling.png"), false, false);
                bLiveSaved = true;
            }
            Advance(9);
        }
    }
    else if (Phase == 9 && PhaseTime > .5f)
    {
        StompKey(PC, EKeys::SpaceBar, true);
        Advance(4);
    }
    else if (Phase == 4)
    {
        if (PhaseTime > .15f) StompKey(PC, EKeys::SpaceBar, false);
        if (Enemy->IsDefeated()) Advance(5);
        else
        {
            const FVector Offset = Enemy->GetActorLocation() - Player->GetActorLocation();
            // 空中操作で減速しながら頭の上を狙います。垂直速度や座標を検証側から変更しません。
            const FVector Steering = Offset.GetSafeNormal2D() * Offset.Size2D() - FVector(Player->GetVelocity().X, Player->GetVelocity().Y, 0) * .22f;
            Player->AddMovementInput(Steering.GetSafeNormal2D());
        }
    }
    else if (Phase == 5 && PhaseTime > .1f)
    {
        if (Enemy->GetSuccessfulStomps() != StompBaseline + Loop + 1 || Player->GetVitality()->GetHealth() != 8 || Player->GetVelocity().Z <= 0)
        { Error = TEXT("Top contact did not defeat once and bounce without damage"); return false; }
        if (!bImpactSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderStomp.png"), false, false);
            bImpactSaved = true;
        }
        UE_LOG(LogTemp, Display, TEXT("STOMP TEST: stomp %d defeated once and bounced upward at %.1f cm/s."), Loop + 1, Player->GetVelocity().Z);
        Advance(6);
    }
    else if (Phase == 6 && PhaseTime > .3f && Movement->IsMovingOnGround())
    {
        if (Enemy->GetSuccessfulStomps() != StompBaseline + Loop + 1 || Player->GetVitality()->GetHealth() != 8)
        { Error = TEXT("Defeated enemy produced repeated contact or defeat"); return false; }
        if (Loop == 0) { StompTap(PC, EKeys::R); Advance(7); }
        else Advance(8);
    }
    else if (Phase == 8)
    {
        if (!bLoggedPass)
        {
            UE_LOG(LogTemp, Display, TEXT("STOMP TEST PASS: real side damage, ordinary jump approach, top crossing, one defeat/bounce, no repeated hit, R revival and second stomp passed."));
            bLoggedPass = true;
        }
        return PhaseTime > 2;
    }
    return false;
}
