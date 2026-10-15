#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ThunderCoinLedger.generated.h"

enum class EPlayseedCoinKind : uint8;
DECLARE_MULTICAST_DELEGATE_TwoParams(FThunderCoinCollected, FName, EPlayseedCoinKind);

// GameMode所有なので入場ごとに新しくなります。セーブ用Subsystemとは寿命が異なります。
UCLASS()
class PLAYSEED_API UThunderCoinLedger : public UActorComponent
{
    GENERATED_BODY()
public:
    bool TryCollect(FName CoinId, EPlayseedCoinKind Kind);
    void ResetVisit();
    bool HasCollected(FName CoinId) const { return CollectedIds.Contains(CoinId); }
    int32 GetTotal() const { return Total; }
    int32 GetRedCount() const { return RedIds.Num(); }
    FThunderCoinCollected OnCoinCollected;
private:
    TSet<FName> CollectedIds;
    TSet<FName> RedIds;
    int32 Total = 0;
};
