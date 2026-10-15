#include "ThunderScarletTrail.h"
#include "ThunderMeadowGameMode.h"
#include "ThunderCoinLedger.h"
#include "PlayseedCoin.h"
#include "PlayseedLantern.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"

AThunderScarletTrail::AThunderScarletTrail()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ScarletTrailRoot"));
}
void AThunderScarletTrail::BindLedger()
{
    if (Ledger.IsValid()) return;
    if (auto* Mode = Cast<AThunderMeadowGameMode>(GetWorld()->GetAuthGameMode()))
    {
        Ledger = Mode->GetCoinLedger(); Ledger->OnCoinCollected.AddUObject(this, &AThunderScarletTrail::HandleCoin);
    }
}
void AThunderScarletTrail::BeginPlay() { Super::BeginPlay(); BindLedger(); }
void AThunderScarletTrail::Tick(float DeltaSeconds) { Super::Tick(DeltaSeconds); BindLedger(); }
void AThunderScarletTrail::EndPlay(const EEndPlayReason::Type Reason)
{
    if (Ledger.IsValid()) Ledger->OnCoinCollected.RemoveAll(this); Super::EndPlay(Reason);
}
void AThunderScarletTrail::Configure(const TArray<FVector>& CoinLocations)
{
    if (!Coins.IsEmpty() || CoinLocations.Num() != 8) return;
    for (int32 Index = 0; Index < 8; ++Index)
    {
        const FName Id(*FString::Printf(TEXT("ThunderScarlet_%d"), Index + 1)); TargetIds.Add(Id);
        auto* Coin = GetWorld()->SpawnActor<APlayseedCoin>(CoinLocations[Index], FRotator::ZeroRotator);
        if (Coin) { Coin->ConfigureCoin(Id, EPlayseedCoinKind::Scarlet); Coins.Add(Coin); }
    }
}
void AThunderScarletTrail::HandleCoin(FName CoinId, EPlayseedCoinKind Kind)
{
    if (Kind != EPlayseedCoinKind::Scarlet || !TargetIds.Contains(CoinId) || CollectedIds.Contains(CoinId)) return;
    CollectedIds.Add(CoinId);
    UE_LOG(LogTemp, Display, TEXT("SCARLET TRAIL: actual target %s, %d/8."), *CoinId.ToString(), CollectedIds.Num());
    if (CollectedIds.Num() != 8 || Reward.IsValid()) return;
    // 集計通知中でもガード済みです。他の目的（100点など）の通知も同時に受けられます。
    Reward = GetWorld()->SpawnActor<APlayseedLantern>(GetRewardRestLocation(), FRotator::ZeroRotator);
    if (Reward.IsValid()) { Reward->ConfigureReward(4); ++RewardGenerationCount; }
}
void AThunderScarletTrail::ResetTrail()
{
    CollectedIds.Reset(); if (Reward.IsValid()) Reward->Destroy(); Reward.Reset();
    for (APlayseedCoin* Coin : Coins) if (Coin) Coin->ResetCoin();
}
