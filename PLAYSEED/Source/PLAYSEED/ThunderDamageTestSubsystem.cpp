#include "ThunderDamageTestSubsystem.h"
#include "ThunderDamageThistle.h"
#include "PlayseedVitalityComponent.h"
#include "ThunderExpeditionSubsystem.h"
#include "WindseedCharacter.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UThunderDamageTestSubsystem::BeginTest(AWindseedCharacter* Player, AThunderDamageThistle* InThistle)
{
    Thistle = InThistle;
    Error.Reset();
    Phase = 0;
    PhaseTime = 0;
    RecoveryBaseline = Player->RecoveryCount;
    const auto* Vitality = Player->GetVitality();
    HitBaseline = Vitality->GetSuccessfulImpacts();
    BlockBaseline = Vitality->GetBlockedImpacts();
    DefeatBaseline = Vitality->GetDefeatCount();
    RewardBaseline = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->GetEarnedRewardCount();
    bSnapshotSaved = false;
}

void UThunderDamageTestSubsystem::Approach(AWindseedCharacter* Player, float DeltaSeconds)
{
    const FVector Offset = Thistle->GetActorLocation() - Player->GetActorLocation();
    // 検証側からダメージを注入せず、歩いてイバラへ接触します。
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (auto* PC = Cast<APlayerController>(Player->GetController()))
        PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-18, Offset.Rotation().Yaw, 0), DeltaSeconds, 2));
}

bool UThunderDamageTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    if (!Thistle.IsValid()) { Error = TEXT("Damage thistle is missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 20) { Error = FString::Printf(TEXT("Damage phase %d timed out at %s"), Phase, *Player->GetActorLocation().ToString()); return false; }
    auto* Vitality = Player->GetVitality();
    auto* PC = Cast<APlayerController>(Player->GetController());
    auto Advance = [this](int32 Next) { Phase = Next; PhaseTime = 0; };
    if (GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->GetEarnedRewardCount() != RewardBaseline)
    { Error = TEXT("Damage or defeat changed saved rewards"); return false; }
    if (Phase == 0)
    {
        if (Vitality->GetSuccessfulImpacts() == HitBaseline + 1) Advance(1);
        else Approach(Player, DeltaSeconds);
    }
    else if (Phase == 1 && PhaseTime > .08f)
    {
        const float PushSpeed = FVector::DotProduct(Player->GetVelocity().GetSafeNormal2D(), Vitality->GetLastImpulse().GetSafeNormal2D()) * Player->GetVelocity().Size2D();
        if (Vitality->GetHealth() != 6 || !Vitality->IsInvulnerable() || PushSpeed < 150 || Player->GetVelocity().Z <= 0)
        { Error = TEXT("Contact did not reduce health and apply upward knockback"); return false; }
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderDamageImpact.png"), false, false);
        UE_LOG(LogTemp, Display, TEXT("DAMAGE TEST: health 6/8, knockback %.1f cm/s and upward velocity %.1f."), PushSpeed, Player->GetVelocity().Z);
        Advance(2);
    }
    else if (Phase == 2)
    {
        if (Vitality->GetSuccessfulImpacts() != HitBaseline + 1 || Vitality->GetHealth() != 6)
        { Error = TEXT("Invulnerability allowed another impact too soon"); return false; }
        if (Vitality->GetBlockedImpacts() > BlockBaseline && PC)
        {
            UE_LOG(LogTemp, Display, TEXT("DAMAGE TEST: repeated real contact was blocked during invulnerability; pressing R."));
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R, IE_Pressed, 1));
            PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R, IE_Released, 0));
            Advance(3);
        }
        else Approach(Player, DeltaSeconds);
    }
    else if (Phase == 3 && PhaseTime > .3f)
    {
        if (Vitality->GetHealth() != 8 || Vitality->IsInvulnerable() || Player->RecoveryCount != RecoveryBaseline + 1)
        { Error = TEXT("R did not restore health and clear invulnerability"); return false; }
        RecoveryBaseline = Player->RecoveryCount;
        HitBaseline = Vitality->GetSuccessfulImpacts();
        Advance(4);
    }
    else if (Phase == 4)
    {
        if (Vitality->GetDefeatCount() == DefeatBaseline + 1)
        {
            Player->GetCharacterMovement()->StopMovementImmediately();
            if (Vitality->GetSuccessfulImpacts() != HitBaseline + 4 || Vitality->GetHealth() != 8 || Player->RecoveryCount != RecoveryBaseline + 1)
            { Error = TEXT("Four actual impacts did not cause exactly one defeat and full recovery"); return false; }
            UE_LOG(LogTemp, Display, TEXT("DAMAGE TEST: four real impacts caused one defeat; health and stage restarted."));
            Advance(5);
        }
        else Approach(Player, DeltaSeconds);
    }
    else if (Phase == 5 && PhaseTime > .3f)
    {
        if (FVector::Dist2D(Player->GetActorLocation(), FVector(-600, -600, 100)) > 5)
        { Error = TEXT("Defeat recovery retained movement momentum"); return false; }
        // この座標置換は場外落下の復帰確認だけです。被弾の接触ルートには使いません。
        Player->SetActorLocation(FVector(0, -1200, -900), false, nullptr, ETeleportType::TeleportPhysics);
        Advance(6);
    }
    else if (Phase == 6 && PhaseTime > .3f)
    {
        if (Vitality->GetDefeatCount() != DefeatBaseline + 2 || Vitality->GetHealth() != 8 || Player->RecoveryCount != RecoveryBaseline + 2 || FVector::Dist2D(Player->GetActorLocation(), FVector(-600, -600, 100)) > 5)
        { Error = TEXT("Thunder fall did not cause one defeat and safe recovery"); return false; }
        Advance(7);
    }
    else if (Phase == 7)
    {
        if (PC) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, -45, 0), DeltaSeconds, 3));
        if (PhaseTime > 1 && !bSnapshotSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderVitality.png"), false, false);
            bSnapshotSaved = true;
            UE_LOG(LogTemp, Display, TEXT("DAMAGE TEST PASS: real contact, health, invulnerability, knockback, R restore, defeat, fall recovery and saved rewards passed."));
        }
        return PhaseTime > 2;
    }
    return false;
}
