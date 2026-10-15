#include "ThunderMeadowGameMode.h"
#include "WindseedCharacter.h"
#include "ThunderExpeditionSubsystem.h"
#include "Engine/GameInstance.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/Canvas.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/SkyLight.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

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
    DrawText(TEXT("WASD: move   SPACE: jump   R: restart   M: stage selection"), FLinearColor::White, 30, Canvas->SizeY - 45, GEngine->GetMediumFont(), 1.2f);
    if (!Expedition->GetProgressProblem().IsEmpty())
        DrawText(Expedition->GetProgressProblem(), FLinearColor(1, .5f, .4f), 30, Canvas->SizeY - 80, GEngine->GetMediumFont(), 1);
}

AThunderMeadowGameMode::AThunderMeadowGameMode()
{
    DefaultPawnClass = AWindseedCharacter::StaticClass();
    HUDClass = AThunderMeadowHUD::StaticClass();
}

void AThunderMeadowGameMode::CreateEntryFloor()
{
    // StaticMeshActor の所有者は World、Component の所有者はその Actor です。
    // Engine の基本形状を使い、既存ステージの Actor や収集物は生成しません。
    auto* Floor = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(0, 0, -50), FRotator::ZeroRotator);
    auto* Mesh = Floor->GetStaticMeshComponent();
    Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
    Floor->SetActorScale3D(FVector(24, 24, 1));
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        auto* Material = UMaterialInstanceDynamic::Create(Palette, Floor);
        Material->SetVectorParameterValue(TEXT("Tint"), FLinearColor(.22f, .5f, .33f));
        Mesh->SetMaterial(0, Material);
    }
    auto* Sun = GetWorld()->SpawnActor<ADirectionalLight>(FVector(0, 0, 1500), FRotator(-50, -30, 0));
    Sun->GetLightComponent()->SetIntensity(3);
    auto* Sky = GetWorld()->SpawnActor<AStaticMeshActor>();
    auto* SkyMesh = Sky->GetStaticMeshComponent();
    SkyMesh->SetMobility(EComponentMobility::Movable);
    SkyMesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")));
    Sky->SetActorScale3D(FVector(400));
    SkyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SkyMesh->SetCastShadow(false);
    SkyMesh->SetMaterial(0, LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Sky.M_Sky")));
    auto* Fill = GetWorld()->SpawnActor<ASkyLight>();
    Fill->GetLightComponent()->SetMobility(EComponentMobility::Movable);
    Fill->GetLightComponent()->bLowerHemisphereIsBlack = false;
    Fill->GetLightComponent()->SetIntensity(1.2f);
    Fill->GetLightComponent()->SetRealTimeCaptureEnabled(true);
    auto* Exposure = GetWorld()->SpawnActor<APostProcessVolume>();
    Exposure->bUnbound = true;
    Exposure->Settings.bOverride_AutoExposureMethod = true;
    Exposure->Settings.AutoExposureMethod = AEM_Manual;
    Exposure->Settings.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
    Exposure->Settings.AutoExposureApplyPhysicalCameraExposure = false;
}

void AThunderMeadowGameMode::StartPlay()
{
    // 床を先に生成してから BeginPlay を開始し、プレイヤーを空中へ取り残さないようにします。
    CreateEntryFloor();
    Super::StartPlay();
    if (auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
    {
        Player->SetTrailStartLocation(FVector(-600, -600, 100));
        Player->ReturnToTrail();
        Player->RecoveryCount = 0;
    }
}
