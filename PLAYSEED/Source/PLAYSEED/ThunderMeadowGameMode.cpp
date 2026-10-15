#include "ThunderMeadowGameMode.h"
#include "ThunderCoinLedger.h"
#include "WindseedCharacter.h"
#include "PlayseedVitalityComponent.h"
#include "PlayseedInteractionComponent.h"
#include "PlayseedLaunchComponent.h"
#include "PlayseedFlightComponent.h"
#include "ThunderMeadowStage.h"
#include "ThunderStormkeeper.h"
#include "ThunderCloudflagRace.h"
#include "ThunderScarletTrail.h"
#include "Engine/World.h"
#include "ThunderExpeditionSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

void AThunderMeadowHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;
    DrawRect(FLinearColor(.02f, .06f, .1f, .94f), 20, 20, FMath::Min(920.f, Canvas->SizeX - 40.f), 130);
    DrawText(TEXT("THUNDER MEADOW"), FLinearColor(.8f, .9f, 1), 38, 32, GEngine->GetMediumFont(), 1.7f);
    // HUD は現在選択された目的だけを表示します。選択しただけでは達成扱いにしません。
    auto* Expedition = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    const int32 Mission = Expedition->GetSelectedMissionNumber();
    DrawText(FString::Printf(TEXT("LANTERNS %d / 7"), Expedition->GetEarnedRewardCount()), FLinearColor(.7f, 1, .8f), Canvas->SizeX - 235, 34, GEngine->GetMediumFont(), 1.1f);
    DrawText(FString::Printf(TEXT("%d: %s"), Mission, UThunderExpeditionSubsystem::GetMissionTitle(Mission)), FLinearColor(1, .86f, .52f), 38, 78, GEngine->GetMediumFont(), 1.1f);
    DrawText(UThunderExpeditionSubsystem::GetMissionGoal(Mission), FLinearColor::White, 38, 115, GEngine->GetMediumFont(), .85f);
    if (auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
    {
        const auto* Vitality = Player->GetVitality();
        const auto* Launch = Player->GetLaunch();
        if (Player->GetFlight()->HasPower())
        {
            DrawRect(FLinearColor(.02f, .06f, .1f, .9f), 570, 158, 260, 35);
            DrawText(FString::Printf(TEXT("LEAFWINGS %.1fs%s"), Player->GetFlight()->GetRemaining(), Player->GetFlight()->IsFlying() ? TEXT("  GLIDE") : TEXT("")), FLinearColor(.6f, 1, .75f), 580, 165, GEngine->GetMediumFont(), 1.1f);
            if (Expedition->GetProgressProblem().IsEmpty()) DrawText(TEXT("LEAFWINGS: air SPACE glide/fall | Mouse/stick: steer | W/S: speed"), FLinearColor(.6f, 1, .75f), 30, Canvas->SizeY - 84, GEngine->GetMediumFont(), .9f);
        }
        if (Launch->IsAiming())
        {
            DrawRect(FLinearColor(.02f, .06f, .1f, .94f), 20, 205, FMath::Min(920.f, Canvas->SizeX - 40.f), 38);
            DrawText(FString::Printf(TEXT("SKYBLOOM  %.0f deg   Mouse / right stick: aim   LMB / RT: launch   G / DOWN: leave"), Launch->GetAimPitch()), FLinearColor(1, .88f, .55f), 32, 214, GEngine->GetMediumFont(), .85f);
            const float X = Canvas->SizeX * .5f, Y = Canvas->SizeY * .5f;
            DrawLine(X - 12, Y, X + 12, Y, FLinearColor(1, .85f, .25f), 2);
            DrawLine(X, Y - 12, X, Y + 12, FLinearColor(1, .85f, .25f), 2);
        }
        const FLinearColor Color = Vitality->IsInvulnerable() ? FLinearColor(1, .4f, .35f) : FLinearColor(.85f, 1, .9f);
        DrawRect(FLinearColor(.02f, .06f, .1f, .9f), 20, 158, 215, 35);
        DrawText(FString::Printf(TEXT("VITALITY %d / %d%s"), Vitality->GetHealth(), Vitality->GetMaximumHealth(), Vitality->IsInvulnerable() ? TEXT("  !") : TEXT("")), Color, 30, 165, GEngine->GetMediumFont(), 1.1f);
        if (!Player->GetInteraction()->GetMessage().IsEmpty())
        {
            DrawRect(FLinearColor(.02f, .06f, .1f, .94f), 20, Canvas->SizeY - 145, FMath::Min(920.f, Canvas->SizeX - 40.f), 48);
            DrawText(Player->GetInteraction()->GetMessage(), FLinearColor(1, .88f, .55f), 34, Canvas->SizeY - 132, GEngine->GetMediumFont(), 1.05f);
        }
    }
    DrawText(TEXT("WASD: move  SPACE: jump  CTRL/RB: dive  LMB/RT: strike"), FLinearColor::White, 30, Canvas->SizeY - 62, GEngine->GetMediumFont(), 1.05f);
    DrawText(TEXT("E/X: lift  F/Y: climb  Q/UP: ride  G/DOWN: talk  R: restart  M: menu"), FLinearColor::White, 30, Canvas->SizeY - 38, GEngine->GetMediumFont(), 1.05f);
    if (auto* Mode = Cast<AThunderMeadowGameMode>(GetWorld()->GetAuthGameMode()))
    {
        const auto* Coins = Mode->GetCoinLedger();
        DrawRect(FLinearColor(.02f, .06f, .1f, .9f), 245, 158, 310, 35);
        DrawText(FString::Printf(TEXT("LIGHTCOINS %d   SCARLET %d"), Coins->GetTotal(), Coins->GetRedCount()), FLinearColor(1, .9f, .4f), 255, 165, GEngine->GetMediumFont(), 1.1f);
    }
    if (!Expedition->GetProgressProblem().IsEmpty())
        DrawText(Expedition->GetProgressProblem(), FLinearColor(1, .5f, .4f), 30, Canvas->SizeY - 80, GEngine->GetMediumFont(), 1);
    if (Mission == 1)
        if (const auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(this, AThunderMeadowStage::StaticClass())); Meadow && Meadow->GetStormkeeper())
            DrawText(FString::Printf(TEXT("STORMKEEPER  THROWS %d / 3"), Meadow->GetStormkeeper()->GetValidThrows()), FLinearColor(1, .85f, .6f), 30, 211, GEngine->GetMediumFont(), 1.1f);
    if (Mission == 2)
        if (const auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(this, AThunderMeadowStage::StaticClass())); Meadow && Meadow->GetCloudflagRace())
            DrawText(FString::Printf(TEXT("%.1fs  %s"), Meadow->GetCloudflagRace()->GetElapsed(), Meadow->GetCloudflagRace()->GetStateLabel()), FLinearColor(1, .85f, .6f), 30, 252, GEngine->GetMediumFont(), .9f);
    if (Mission == 4)
        if (const auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(this, AThunderMeadowStage::StaticClass())); Meadow && Meadow->GetScarletTrail())
            DrawText(FString::Printf(TEXT("SCARLET TRAIL  %d / 8"), Meadow->GetScarletTrail()->GetCollectedCount()), FLinearColor(1, .5f, .6f), 30, 211, GEngine->GetMediumFont(), 1.1f);
}

AThunderMeadowGameMode::AThunderMeadowGameMode()
{
    CoinLedger = CreateDefaultSubobject<UThunderCoinLedger>(TEXT("VisitCoinLedger"));
    DefaultPawnClass = AWindseedCharacter::StaticClass();
    HUDClass = AThunderMeadowHUD::StaticClass();
}

void AThunderMeadowGameMode::StartPlay()
{
    // 地形 Actor を先に登録してから World の BeginPlay を開始します。
    // 各 Actor の準備が終わってから最初の Tick が進みます。
    Stage = GetWorld()->SpawnActor<AThunderMeadowStage>();
    Super::StartPlay();
    if (auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
    {
        Player->SetTrailStartLocation(FVector(-600, -600, 100));
        Player->ReturnToTrail();
        Player->RecoveryCount = 0;
    }
}

void AThunderMeadowGameMode::RestartStage(AWindseedCharacter* Player)
{
    // このステージが生成した仕掛けだけを初期化し、永続の獲得済み報酬には触れません。
    CoinLedger->ResetVisit();
    if (Stage) Stage->ResetMechanisms();
    Super::RestartStage(Player);
}

void AThunderMeadowGameMode::HandlePlayerFall(AWindseedCharacter* Player)
{
    // 雷鳴の丘では場外落下も死亡として扱います。保存済み報酬は再開で消しません。
    if (Player) Player->GetVitality()->Defeat();
}
