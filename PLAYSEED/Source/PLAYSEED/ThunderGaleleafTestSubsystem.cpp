#include "ThunderGaleleafTestSubsystem.h"
#include "PlayseedGaleleafEnemy.h"
#include "PlayseedGlideBoard.h"
#include "PlayseedCoin.h"
#include "PlayseedVitalityComponent.h"
#include "ThunderCoinLedger.h"
#include "ThunderMeadowGameMode.h"
#include "WindseedCharacter.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace
{
void GaleleafKey(APlayerController* PC, FKey Key, bool bPressed)
{
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, bPressed ? IE_Pressed : IE_Released, bPressed ? 1 : 0));
}
void GaleleafTap(APlayerController* PC, FKey Key) { GaleleafKey(PC, Key, true); GaleleafKey(PC, Key, false); }
}

void UThunderGaleleafTestSubsystem::BeginTest(AWindseedCharacter* Player, APlayseedGaleleafEnemy* InEnemy)
{
    Enemy = InEnemy;
    Phase = Loop = Waypoint = 0;
    PhaseTime = 0;
    Error.Reset();
    LeafBaseline = Enemy->GetLeafDrops();
    AzureBaseline = Enemy->GetAzureDrops();
    StompBaseline = Enemy->GetSuccessfulStomps();
    RecoveryBaseline = Player->RecoveryCount;
    bAzureSaved = false;
}

bool UThunderGaleleafTestSubsystem::WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    if (Offset.Size2D() < 15 && Player->GetCharacterMovement()->IsMovingOnGround())
    { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (auto* PC = Cast<APlayerController>(Player->GetController())) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, Offset.Rotation().Yaw, 0), DeltaSeconds, 3));
    return false;
}

bool UThunderGaleleafTestSubsystem::Approach(AWindseedCharacter* Player, float DeltaSeconds)
{
    const FVector GaleleafApproachPoints[] = {FVector(-600, -900, 82), FVector(1900, -900, 82), FVector(1900, -600, 82), FVector(2100, -300, 82)};
    if (Waypoint < 4 && WalkToward(Player, GaleleafApproachPoints[Waypoint], DeltaSeconds)) ++Waypoint;
    return Waypoint >= 4;
}

bool UThunderGaleleafTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    auto* PC = Cast<APlayerController>(Player->GetController());
    auto* Mode = Cast<AThunderMeadowGameMode>(Player->GetWorld()->GetAuthGameMode());
    if (!Enemy.IsValid() || !PC || !Mode) { Error = TEXT("Galeleaf enemy or controller missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 30) { Error = FString::Printf(TEXT("Galeleaf loop %d phase %d timed out at %s"), Loop, Phase, *Player->GetActorLocation().ToString()); return false; }
    if (Player->GetVitality()->GetHealth() != 8) { Error = FString::Printf(TEXT("Galeleaf loop %d phase %d took damage at %s, enemy %s, distance %.1f"), Loop, Phase, *Player->GetActorLocation().ToString(), *Enemy->GetActorLocation().ToString(), FVector::Dist2D(Player->GetActorLocation(), Enemy->GetActorLocation())); return false; }
    auto Next = [this](int32 Number) { Phase = Number; PhaseTime = 0; };
    if (Phase == 0)
    {
        if (Approach(Player, DeltaSeconds)) Next(Loop == 0 ? 1 : 8);
    }
    else if (Phase == 1 || Phase == 3)
    {
        const FVector Offset = Enemy->GetActorLocation() - Player->GetActorLocation();
        if (Offset.Size2D() < 165 && Player->GetCharacterMovement()->IsMovingOnGround())
        {
            Player->GetCharacterMovement()->StopMovementImmediately();
            GaleleafTap(PC, EKeys::LeftMouseButton);
            Next(Phase == 1 ? 2 : 4);
        }
        else
        {
            // 描画が長いフレームでも相手へ突っ込まないよう、近距離はアナログ入力を弱めます。
            Player->AddMovementInput(Offset.GetSafeNormal2D(), Offset.Size2D() < 350 ? .4f : 1.f);
            PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, Offset.Rotation().Yaw, 0), DeltaSeconds, 3));
        }
    }
    else if (Phase == 2 && PhaseTime > .3f)
    {
        if (Enemy->IsDefeated() || Enemy->HasGaleleaf() || !Enemy->GetDroppedGaleleaf() || Enemy->GetLeafDrops() != LeafBaseline + 1 || Enemy->GetAzureDrops() != AzureBaseline || Enemy->GetAzureCoin() || FVector::Dist2D(Enemy->GetActorLocation(), FVector(2700, 0, 0)) < 20)
        { Error = TEXT("First punch did not release one leaf, survive and flee without an azure drop"); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderGaleleafDrop.png"), false, false);
        Next(3);
    }
    else if (Phase == 4 && PhaseTime > .3f)
    {
        if (!Enemy->IsDefeated() || !Enemy->GetDroppedGaleleaf() || !Enemy->GetAzureCoin() || Enemy->GetLeafDrops() != LeafBaseline + Loop + 1 || Enemy->GetAzureDrops() != AzureBaseline + Loop + 1)
        { Error = TEXT("Second attack did not defeat and emit exactly one azure coin"); return false; }
        GaleleafTap(PC, EKeys::LeftMouseButton);
        Next(5);
    }
    else if (Phase == 5)
    {
        if (!Enemy->GetAzureCoin() || Enemy->GetLeafDrops() != LeafBaseline + Loop + 1 || Enemy->GetAzureDrops() != AzureBaseline + Loop + 1)
        { Error = TEXT("Galeleaf or azure contents were duplicated"); return false; }
        if (WalkToward(Player, Enemy->GetAzureCoin()->GetActorLocation(), DeltaSeconds))
        {
            if (!Enemy->GetAzureCoin()->IsCollected() || Mode->GetCoinLedger()->GetTotal() != 5) { Error = TEXT("Actual azure collection did not total five"); return false; }
            Next(6);
        }
    }
    else if (Phase == 6)
    {
        if (!bAzureSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderGaleleafAzure.png"), false, false);
            bAzureSaved = true;
        }
        if (PhaseTime > .6f)
        {
            if (Loop == 0) { GaleleafTap(PC, EKeys::R); Loop = 1; Next(7); }
            else
            {
                UE_LOG(LogTemp, Display, TEXT("GALELEAF TEST PASS: first punch and ordinary stomp each release one leaf while alive, flee and bounce, second attack drops one azure five, R cleanup and re-collection without duplicates passed."));
                return true;
            }
        }
    }
    else if (Phase == 7 && PhaseTime > .2f)
    {
        if (Enemy->IsDefeated() || !Enemy->HasGaleleaf() || Enemy->GetDroppedGaleleaf() || Enemy->GetAzureCoin() || Mode->GetCoinLedger()->GetTotal() != 0 || Player->RecoveryCount != RecoveryBaseline + 1)
        { Error = TEXT("R did not restore galeleaf armor and remove own drops and totals"); return false; }
        Waypoint = 0;
        Next(0);
    }
    else if (Phase == 8)
    {
        if (WalkToward(Player, FVector(2370, 0, 82), DeltaSeconds)) { GaleleafKey(PC, EKeys::SpaceBar, true); Next(9); }
    }
    else if (Phase == 9)
    {
        if (PhaseTime > .15f) GaleleafKey(PC, EKeys::SpaceBar, false);
        if (!Enemy->HasGaleleaf()) Next(10);
        else
        {
            const FVector Offset = Enemy->GetActorLocation() - Player->GetActorLocation();
            const FVector Steering = FVector(Offset.X, Offset.Y, 0) - FVector(Player->GetVelocity().X, Player->GetVelocity().Y, 0) * .22f;
            Player->AddMovementInput(Steering.GetSafeNormal2D());
        }
    }
    else if (Phase == 10)
    {
        Player->AddMovementInput((FVector(2200, -500, 82) - Player->GetActorLocation()).GetSafeNormal2D());
        if (PhaseTime > .08f)
        {
            if (Enemy->IsDefeated() || Enemy->HasGaleleaf() || !Enemy->GetDroppedGaleleaf() || Enemy->GetLeafDrops() != LeafBaseline + 2 || Enemy->GetAzureDrops() != AzureBaseline + 1 || Enemy->GetAzureCoin() || Enemy->GetSuccessfulStomps() != StompBaseline + 1 || Player->GetVelocity().Z <= 0)
            { Error = TEXT("First ordinary stomp did not release one leaf and bounce while enemy survived"); return false; }
            Next(11);
        }
    }
    else if (Phase == 11)
    {
        if (Player->GetCharacterMovement()->IsMovingOnGround()) { Player->GetCharacterMovement()->StopMovementImmediately(); Next(3); }
        else Player->AddMovementInput((FVector(2200, -500, 82) - Player->GetActorLocation()).GetSafeNormal2D());
    }
    return false;
}
