#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayseedLantern.generated.h"

class UStaticMeshComponent;

// World所有のオリジナル報酬です。各ミッションは固有の1〜7を設定し、保存はSubsystemへ任せます。
UCLASS()
class PLAYSEED_API APlayseedLantern : public AActor
{
    GENERATED_BODY()
public:
    APlayseedLantern();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void ConfigureReward(int32 Number);
    int32 GetRewardNumber() const { return RewardNumber; }
    bool IsCollected() const { return bCollected; }
    int32 GetSuccessfulCollections() const { return SuccessfulCollections; }
private:
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;
    TArray<FLinearColor> Colors;
    FVector RestLocation = FVector::ZeroVector;
    int32 RewardNumber = 0;
    int32 SuccessfulCollections = 0;
    float RetryRemaining = 0;
    bool bCollected = false;
};
