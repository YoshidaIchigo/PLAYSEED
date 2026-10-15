#include "ThunderMeadowGameMode.h"
#include "WindseedCharacter.h"
#include "PlayseedVitalityComponent.h"
#include "ThunderMeadowStage.h"
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
        const FLinearColor Color = Vitality->IsInvulnerable() ? FLinearColor(1, .4f, .35f) : FLinearColor(.85f, 1, .9f);
        DrawRect(FLinearColor(.02f, .06f, .1f, .9f), 20, 158, 215, 35);
        DrawText(FString::Printf(TEXT("VITALITY %d / %d%s"), Vitality->GetHealth(), Vitality->GetMaximumHealth(), Vitality->IsInvulnerable() ? TEXT("  !") : TEXT("")), Color, 30, 165, GEngine->GetMediumFont(), 1.1f);
    }
    DrawText(TEXT("WASD: move   SPACE: jump   R: restart   M: stage selection"), FLinearColor::White, 30, Canvas->SizeY - 45, GEngine->GetMediumFont(), 1.2f);
    if (!Expedition->GetProgressProblem().IsEmpty())
        DrawText(Expedition->GetProgressProblem(), FLinearColor(1, .5f, .4f), 30, Canvas->SizeY - 80, GEngine->GetMediumFont(), 1);
}

AThunderMeadowGameMode::AThunderMeadowGameMode()
{
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
    if (Stage) Stage->ResetMechanisms();
    Super::RestartStage(Player);
}

void AThunderMeadowGameMode::HandlePlayerFall(AWindseedCharacter* Player)
{
    // 雷鳴の丘では場外落下も死亡として扱います。保存済み報酬は再開で消しません。
    if (Player) Player->GetVitality()->Defeat();
}
