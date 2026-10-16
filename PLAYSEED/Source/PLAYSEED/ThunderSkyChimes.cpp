#include "ThunderSkyChimes.h"
#include "ThunderMeadowGameMode.h"
#include "ThunderCoinLedger.h"
#include "PlayseedCoin.h"
#include "PlayseedLantern.h"
#include "Components/SceneComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AThunderSkyChimes::AThunderSkyChimes()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SkyChimeRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere")); BeadMesh = Sphere.Object;
}
void AThunderSkyChimes::BindLedger()
{
    if (Ledger.IsValid()) return;
    if (auto* Mode = Cast<AThunderMeadowGameMode>(GetWorld()->GetAuthGameMode())) { Ledger = Mode->GetCoinLedger(); Ledger->OnCoinCollected.AddUObject(this, &AThunderSkyChimes::HandleCoin); }
}
void AThunderSkyChimes::BeginPlay() { Super::BeginPlay(); BindLedger(); }
void AThunderSkyChimes::Tick(float DeltaSeconds) { Super::Tick(DeltaSeconds); BindLedger(); }
void AThunderSkyChimes::EndPlay(const EEndPlayReason::Type Reason) { if (Ledger.IsValid()) Ledger->OnCoinCollected.RemoveAll(this); Super::EndPlay(Reason); }
void AThunderSkyChimes::Configure(const TArray<FVector>& Centres, FVector RewardLocation)
{
    if (!Coins.IsEmpty() || Centres.Num() != 5) return;
    RewardRestLocation = RewardLocation;
    auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette"));
    for (int32 Index = 0; Index < 5; ++Index)
    {
        const FName Id(*FString::Printf(TEXT("ThunderChimeCentre_%d"), Index + 1)); TargetIds.Add(Id);
        auto* Coin = GetWorld()->SpawnActor<APlayseedCoin>(Centres[Index], FRotator::ZeroRotator);
        if (Coin) { Coin->ConfigureCoin(Id, EPlayseedCoinKind::Gold); Coins.Add(Coin); }
        // 24個の小さな球を一つの部品で描き、オリジナルの鈴の輪を構成します。
        auto* Ring = NewObject<UInstancedStaticMeshComponent>(this); Ring->SetupAttachment(RootComponent); Ring->SetStaticMesh(BeadMesh);
        Ring->SetCollisionEnabled(ECollisionEnabled::NoCollision); Ring->RegisterComponent(); Rings.Add(Ring);
        auto* Tint = Palette ? UMaterialInstanceDynamic::Create(Palette, this) : nullptr; Tints.Add(Tint);
        if (Tint) { Tint->SetVectorParameterValue(TEXT("Tint"), FLinearColor(1, .65f, .12f)); Ring->SetMaterial(0, Tint); }
        for (int32 Bead = 0; Bead < 24; ++Bead)
        {
            const float Angle = Bead * 2 * PI / 24;
            const FVector Position = Centres[Index] + FVector(0, FMath::Cos(Angle) * 280, FMath::Sin(Angle) * 280);
            Ring->AddInstance(FTransform(FRotator::ZeroRotator, GetActorTransform().InverseTransformPosition(Position), FVector(.18f)));
        }
    }
}
void AThunderSkyChimes::HandleCoin(FName CoinId, EPlayseedCoinKind Kind)
{
    if (Kind != EPlayseedCoinKind::Gold || !TargetIds.Contains(CoinId) || CollectedIds.Contains(CoinId)) return;
    CollectedIds.Add(CoinId);
    for (int32 Index = 0; Index < Coins.Num(); ++Index) if (Coins[Index]->GetCoinId() == CoinId && Tints.IsValidIndex(Index) && Tints[Index]) Tints[Index]->SetVectorParameterValue(TEXT("Tint"), FLinearColor(.05f, .7f, .5f));
    UE_LOG(LogTemp, Display, TEXT("SKY CHIMES: actual centre %s, %d/5."), *CoinId.ToString(), CollectedIds.Num());
    if (CollectedIds.Num() != 5 || Reward.IsValid()) return;
    Reward = GetWorld()->SpawnActor<APlayseedLantern>(RewardRestLocation, FRotator::ZeroRotator);
    if (Reward.IsValid()) { Reward->ConfigureReward(5); ++RewardGenerationCount; }
}
void AThunderSkyChimes::ResetChimes()
{
    CollectedIds.Reset(); if (Reward.IsValid()) Reward->Destroy(); Reward.Reset();
    for (APlayseedCoin* Coin : Coins) if (Coin) Coin->ResetCoin();
    for (UMaterialInstanceDynamic* Tint : Tints) if (Tint) Tint->SetVectorParameterValue(TEXT("Tint"), FLinearColor(1, .65f, .12f));
}
