#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderSkyChimes.generated.h"
class APlayseedCoin;
class APlayseedLantern;
class UThunderCoinLedger;
class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
enum class EPlayseedCoinKind : uint8;

// 輪は衝突のない目印です。中央の本当のコイン5個だけが達成条件になります。
UCLASS()
class PLAYSEED_API AThunderSkyChimes : public AActor
{
    GENERATED_BODY()
public:
    AThunderSkyChimes();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    void Configure(const TArray<FVector>& Centres, FVector RewardLocation);
    void ResetChimes();
    int32 GetCollectedCount() const { return CollectedIds.Num(); }
    const TArray<TObjectPtr<APlayseedCoin>>& GetCoins() const { return Coins; }
    APlayseedLantern* GetReward() const { return Reward.Get(); }
    int32 GetRewardGenerationCount() const { return RewardGenerationCount; }
    FVector GetRewardRestLocation() const { return RewardRestLocation; }
private:
    void BindLedger();
    void HandleCoin(FName CoinId, EPlayseedCoinKind Kind);
    UPROPERTY() TObjectPtr<UStaticMesh> BeadMesh;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Beads;
    UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> Tints;
    UPROPERTY() TArray<TObjectPtr<APlayseedCoin>> Coins;
    TSet<FName> TargetIds;
    TSet<FName> CollectedIds;
    TWeakObjectPtr<UThunderCoinLedger> Ledger;
    TWeakObjectPtr<APlayseedLantern> Reward;
    FVector RewardRestLocation = FVector::ZeroVector;
    int32 RewardGenerationCount = 0;
};
