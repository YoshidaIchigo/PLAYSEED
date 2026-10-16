#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderPrisonGate.generated.h"
class UStaticMeshComponent;
class AThunderBoundSentinel;
class APlayseedLantern;
UCLASS()
class PLAYSEED_API AThunderPrisonGate : public AActor
{
    GENERATED_BODY()
public:
    AThunderPrisonGate();
    virtual void BeginPlay() override;
    void Configure(AThunderBoundSentinel* InSentinel);
    void ConfigureReferenceReward(FVector Position) { bReferenceReward=true;ReferenceRewardPosition=Position; }
    void HandleReleasedImpact(AThunderBoundSentinel* Caller, const FHitResult& Hit);
    void ResetGate();
    bool IsBroken() const { return bBroken; }
    int32 GetBreakCount() const { return BreakCount; }
    APlayseedLantern* GetReward() const { return Reward.Get(); }
    // 扉の原点は高さ200cmの中央です。報酬はその東側の床から100cmに置きます。
    FVector GetRewardLocation() const { return bReferenceReward?ReferenceRewardPosition:GetActorLocation() + FVector(100, 0, -100); }
private:
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Bars;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Rails;
    TWeakObjectPtr<AThunderBoundSentinel> Sentinel;
    TWeakObjectPtr<APlayseedLantern> Reward;
    bool bBroken = false;
    int32 BreakCount = 0;
    bool bReferenceReward=false;
    FVector ReferenceRewardPosition=FVector::ZeroVector;
};
