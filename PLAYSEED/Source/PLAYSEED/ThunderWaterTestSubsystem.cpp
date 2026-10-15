#include "ThunderWaterTestSubsystem.h"
#include "ThunderWaterBomb.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UThunderWaterTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderWaterBomb* InBomb)
{
    Bomb = InBomb;
    Phase = Waypoint = 0;
    PhaseTime = 0;
    Error.Reset();
    HitBaseline = Bomb->GetConfirmedHits();
    RecoveryBaseline = Player->RecoveryCount;
    bWarningSaved = bEscapeSaved = false;
}

bool UThunderWaterTestSubsystem::WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    if (Offset.Size2D() < 15 && Player->GetCharacterMovement()->IsMovingOnGround())
    { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (auto* PC = Cast<APlayerController>(Player->GetController())) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, Offset.Rotation().Yaw, 0), DeltaSeconds, 3));
    return false;
}

bool UThunderWaterTestSubsystem::WalkApproach(AWindseedCharacter* Player, float DeltaSeconds)
{
    // 木の橋や傾く足場を横切らず、地上の西側から南の警告地点へ向かいます。
    const FVector WaterApproachPoints[] = {FVector(-3000, -600, 82), FVector(-3000, -2900, 82), Bomb->GetActorLocation() + FVector(0, -300, 82)};
    if (Waypoint < 3 && WalkToward(Player, WaterApproachPoints[Waypoint], DeltaSeconds)) ++Waypoint;
    return Waypoint >= 3;
}

bool UThunderWaterTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* PC = Cast<APlayerController>(Player->GetController());
    if (!Bomb.IsValid() || !PC) { Error = TEXT("Water bomb test actor or controller missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 30) { Error = FString::Printf(TEXT("Water phase %d timed out at %s"), Phase, *Player->GetActorLocation().ToString()); return false; }
    auto Next = [this](int32 Number) { Phase = Number; PhaseTime = 0; };
    if (Phase == 0)
    {
        if (WalkApproach(Player, DeltaSeconds)) Next(1);
    }
    else if (Phase == 1)
    {
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-22, 90, 0), DeltaSeconds, 3));
        if (PhaseTime > .5f && Bomb->IsWarning() && Bomb->GetPhaseAge() < .2f)
        {
            if (!bWarningSaved)
            {
                FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderWaterWarning.png"), false, false);
                bWarningSaved = true;
                SeenWarningSerial = Bomb->GetWarningSerial();
            }
            else if (Bomb->GetWarningSerial() > SeenWarningSerial)
            {
                // 撮影の停止時間を回避行動へ混ぜず、次の本当の警告から進みます。
                FirstImpactBaseline = Bomb->GetImpactCount();
                Next(2);
            }
        }
    }
    else if (Phase == 2)
    {
        if (WalkToward(Player, Bomb->GetActorLocation() + FVector(0, 0, 82), DeltaSeconds)) Next(3);
    }
    else if (Phase == 3)
    {
        if (Bomb->GetConfirmedHits() == HitBaseline + 1)
        {
            if (Bomb->GetImpactCount() != FirstImpactBaseline + 1 || Player->GetVitality()->GetHealth() != 6 || Player->GetVitality()->GetLastImpulse().Size2D() < 400)
            { Error = TEXT("Water landing did not damage and knock back once"); return false; }
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderWaterImpact.png"), false, false);
            Next(4);
        }
    }
    else if (Phase == 4)
    {
        if (Bomb->GetImpactCount() != FirstImpactBaseline + 1 || Bomb->GetConfirmedHits() != HitBaseline + 1)
        { Error = TEXT("One water landing was counted repeatedly"); return false; }
        if (Bomb->IsFalling() && Bomb->GetBombHeight() < 500 && Bomb->GetBombHeight() > 100)
        {
            CancelImpactCount = Bomb->GetImpactCount();
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R, IE_Pressed, 1));
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R, IE_Released, 0));
            Next(5);
        }
    }
    else if (Phase == 5 && PhaseTime > .15f)
    {
        if (Player->GetVitality()->GetHealth() != 8 || Player->RecoveryCount != RecoveryBaseline + 1 || !Bomb->IsWarning() || Bomb->GetBombHeight() != 700 || Bomb->GetPhaseAge() > .4f || Bomb->GetImpactCount() != CancelImpactCount)
        { Error = TEXT("R during falling did not cancel the old impact and restore warning/health"); return false; }
        Waypoint = 0;
        UE_LOG(LogTemp, Display, TEXT("WATER TEST: one real landing dealt two damage and knockback; R during next falling cancelled impact and restored warning and health."));
        Next(6);
    }
    else if (Phase == 6)
    {
        if (WalkApproach(Player, DeltaSeconds)) Next(7);
    }
    else if (Phase == 7)
    {
        if (Bomb->IsWarning() && Bomb->GetPhaseAge() < .2f) { EscapeImpactBaseline = Bomb->GetImpactCount(); Next(8); }
    }
    else if (Phase == 8)
    {
        if (WalkToward(Player, Bomb->GetActorLocation() + FVector(0, 0, 82), DeltaSeconds)) Next(9);
    }
    else if (Phase == 9)
    {
        if (WalkToward(Player, Bomb->GetActorLocation() + FVector(-500, 0, 82), DeltaSeconds)) Next(10);
    }
    else if (Phase == 10)
    {
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-18, (Bomb->GetActorLocation() - Player->GetActorLocation()).Rotation().Yaw, 0), DeltaSeconds, 3));
        if (Bomb->GetImpactCount() > EscapeImpactBaseline)
        {
            if (Bomb->GetImpactCount() != EscapeImpactBaseline + 1 || Bomb->GetConfirmedHits() != HitBaseline + 1 || Player->GetVitality()->GetHealth() != 8)
            { Error = TEXT("Walking away during warning did not avoid the blast"); return false; }
            Next(11);
        }
    }
    else if (Phase == 11)
    {
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-18, (Bomb->GetActorLocation() - Player->GetActorLocation()).Rotation().Yaw, 0), DeltaSeconds, 3));
        if (!bEscapeSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderWaterEscape.png"), false, false);
            bEscapeSaved = true;
            UE_LOG(LogTemp, Display, TEXT("WATER TEST PASS: visible warning, falling, one landing, real two-damage knockback, R while falling, enter then retreat during warning without damage."));
        }
        return PhaseTime > 1;
    }
    return false;
}
