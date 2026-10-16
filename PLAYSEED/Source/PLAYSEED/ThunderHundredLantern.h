#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderHundredLantern.generated.h"
class UThunderCoinLedger;
class APlayseedLantern;
enum class EPlayseedCoinKind : uint8;
UCLASS()
class PLAYSEED_API AThunderHundredLantern : public AActor
{
    GENERATED_BODY()
public:
    AThunderHundredLantern();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    void ResetBonus();
    APlayseedLantern* GetReward() const { return Reward.Get(); }
    FVector GetRewardLocation() const { return RewardLocation; }
    int32 GetGenerationCount() const { return GenerationCount; }
    int32 GetTriggerTotal() const { return TriggerTotal; }
    FName GetTriggerCoinId() const { return TriggerCoinId; }
    bool HasReachedHundred() const { return bReachedHundred; }
private:
    void BindLedger();
    void HandleCoin(FName CoinId, EPlayseedCoinKind Kind);
    FVector FindRewardFloor(FVector CoinLocation) const;
    TWeakObjectPtr<UThunderCoinLedger> Ledger;
    TWeakObjectPtr<APlayseedLantern> Reward;
    FVector RewardLocation = FVector::ZeroVector;
    FName TriggerCoinId;
    int32 GenerationCount = 0, TriggerTotal = 0;
    bool bReachedHundred = false;
};
