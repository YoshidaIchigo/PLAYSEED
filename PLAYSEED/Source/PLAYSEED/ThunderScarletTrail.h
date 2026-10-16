#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderScarletTrail.generated.h"
class APlayseedCoin;
class APlayseedLantern;
class UThunderCoinLedger;
enum class EPlayseedCoinKind : uint8;

// 地形上の8個の固有IDだけを数えます。練習用の赤コインはこの目的に含めません。
UCLASS()
class PLAYSEED_API AThunderScarletTrail : public AActor
{
    GENERATED_BODY()
public:
    AThunderScarletTrail();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    void Configure(const TArray<FVector>& CoinLocations);
    void ResetTrail();
    void SetRewardRestLocation(FVector Location) { FixedRewardLocation=Location;bFixedReward=true; }
    int32 GetCollectedCount() const { return CollectedIds.Num(); }
    const TArray<TObjectPtr<APlayseedCoin>>& GetCoins() const { return Coins; }
    APlayseedLantern* GetReward() const { return Reward.Get(); }
    int32 GetRewardGenerationCount() const { return RewardGenerationCount; }
    FVector GetRewardRestLocation() const { return bFixedReward?FixedRewardLocation:GetActorLocation() + FVector(0, 200, 100); }
private:
    FVector FixedRewardLocation=FVector::ZeroVector;
    bool bFixedReward=false;
    void BindLedger();
    void HandleCoin(FName CoinId, EPlayseedCoinKind Kind);
    UPROPERTY() TArray<TObjectPtr<APlayseedCoin>> Coins;
    TSet<FName> TargetIds;
    TSet<FName> CollectedIds;
    TWeakObjectPtr<UThunderCoinLedger> Ledger;
    TWeakObjectPtr<APlayseedLantern> Reward;
    int32 RewardGenerationCount = 0;
};
